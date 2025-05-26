#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"
#include "io.h"

// Variáveis globais para comunicação
pipe_t pipe_acelerador_central;
mutex_t mutex_injecao;
unsigned int g_tempo_injecao = 0;
sem_t sem_freio;

// Variáveis para controle dos três bicos injetores PWM
unsigned int duty_cycle_bico1 = 0;
unsigned int duty_cycle_bico2 = 0;
unsigned int duty_cycle_bico3 = 0;

TASK tarefa_acelerador() // Prioridade 1 (configurada em main.c)
{
    unsigned int adc_value;
    uint8_t adc_scaled;
    
    // Inicializa o ADC para o canal AN0 (onde o potenciômetro está conectado)
    adc_init();

    while (1) {
        adc_value = adc_read(); // Lê o valor do potenciômetro (0-1023)
        
        // Controla LED do acelerador baseado no valor do potenciômetro
        if (adc_value > 512) {
            LED_ACELERADOR = 1; // Liga LED quando potenciômetro > 50%            
        } else {
            LED_ACELERADOR = 0; // Desliga LED quando potenciômetro <= 50%
        }

        // Escala o valor ADC (0-1023) para 8 bits (0-255) e envia via pipe
        adc_scaled = (uint8_t)(adc_value >> 2); // Divide por 4 para converter 10 bits para 8 bits
        write_pipe(&pipe_acelerador_central, adc_scaled);
        
        delay(10); // Pequeno delay para não sobrecarregar
    }    
}

TASK tarefa_controle_central() // Prioridade 2
{
    uint8_t acelerador_value;
    uint16_t tempo_injecao_calculado;

    while (1)
    {
        acelerador_value = 0;
        // Recebe valor do acelerador via pipe
        read_pipe(&pipe_acelerador_central, &acelerador_value);

        if (acelerador_value >= 25)
        {
            tempo_injecao_calculado = 0;

            // Liga LED do controle central quando processa dados
            LED_CONTROLE_CENTRAL = 1;

            // Calcula tempo de injeção baseado no valor do acelerador
            // Converte de 0-255 para 0-1023 (duty cycle PWM)
            tempo_injecao_calculado = (uint16_t)acelerador_value * 4;

            // Protege acesso à variável compartilhada com mutex
            mutex_lock(&mutex_injecao);
            g_tempo_injecao = tempo_injecao_calculado;
            mutex_unlock(&mutex_injecao);
        }
        
        // Desliga LED após processamento
        LED_CONTROLE_CENTRAL = 0;

        delay(10); // Delay curto para permitir outras tarefas
    }
}

TASK tarefa_injecao_eletronica() // Prioridade 3 (mais alta)
{
    uint16_t duty_cycle_atual;

    // Inicia o PWM para os bicos injetores
    pwm_init(BICO_INJETOR_1, 1000); // Bico 1 - RC2/CCP1 - 1kHz
    pwm_init(BICO_INJETOR_2, 1000); // Bico 2 - RC1/CCP2 - 1kHz  
    pwm_init(BICO_INJETOR_3, 1000); // Bico 3 - RC0/Timer1 - 1kHz
    
    while (1) {
        // Protege acesso à variável compartilhada com mutex
        mutex_lock(&mutex_injecao);
        duty_cycle_atual = g_tempo_injecao;
        mutex_unlock(&mutex_injecao);
        
        // Controla LED e PWM baseado na atividade dos bicos
        if (duty_cycle_atual > 0) {
            pwm_start();
            LED_INJECAO_ELETRONICA = ON; // Liga LED quando injeção ativa
            
            // Calcula duty cycle para os três bicos baseado na posição do acelerador
            duty_cycle_bico1 = duty_cycle_atual;                    // Bico 1: duty cycle direto
            duty_cycle_bico2 = (duty_cycle_atual * 90) / 100;       // Bico 2: 90% do valor
            duty_cycle_bico3 = (duty_cycle_atual * 80) / 100;       // Bico 3: 80% do valor
            
            // Aplica PWM aos três bicos injetores
            pwm_set_duty_cycle(BICO_INJETOR_1, duty_cycle_bico1);  // Bico 1 (RC2/CCP1)
            pwm_set_duty_cycle(BICO_INJETOR_2, duty_cycle_bico2);  // Bico 2 (RC1/CCP2)
            pwm_set_duty_cycle(BICO_INJETOR_3, duty_cycle_bico3);  // Bico 3 (RC0/Timer1)
        } else {
            pwm_stop();
            LED_INJECAO_ELETRONICA = OFF; // Desliga LED quando injeção inativa
            
            // Para imediatamente o PWM quando duty_cycle é 0
            pwm_set_duty_cycle(BICO_INJETOR_1, 0); // Desliga Bico 1
            pwm_set_duty_cycle(BICO_INJETOR_2, 0); // Desliga Bico 2
            pwm_set_duty_cycle(BICO_INJETOR_3, 0); // Desliga Bico 3
            
            // Zera as variáveis de duty cycle
            duty_cycle_bico1 = 0;
            duty_cycle_bico2 = 0;
            duty_cycle_bico3 = 0;
        }

        delay(5); // Pequeno delay para permitir outras tarefas
    }
}

// Callback para interrupção de freio
void freio_interrupt_callback(void) { 
    // Cria tarefa de controle de estabilidade
    create_task(4, 4, tarefa_controle_estabilidade); // Prioridade 4 (mais alta de usuário)
    
    // Sinaliza que o freio foi acionado (se necessário para outras lógicas)
    sem_post(&sem_freio); 
}

TASK tarefa_controle_estabilidade() // Prioridade 4 (mais alta de usuário) - ONE SHOT
{
    // Espera pelo evento de freio (semáforo)
    sem_wait(&sem_freio);

    while (PORTBbits.RB0 == 1){
        // Simula acionamento dos freios (liga LED)
        LED_CONTROLE_ESTABILIDADE = ON;

        // Mantém freios acionados por por 300ms
        __delay_ms(300);

        mutex_lock(&mutex_injecao);
        g_tempo_injecao = 0; // Zera tempo de injeção
        mutex_unlock(&mutex_injecao);

        delay(50);
    }

    LED_ACELERADOR = OFF;
    LED_CONTROLE_ESTABILIDADE = OFF;

    remove_task(tarefa_controle_estabilidade);
    
    while (1){
        yield();
    }
}

void user_config()
{
    // Configura os pinos dos LEDs como saída
    TRISDbits.TRISD0 = 0; // LED_ACELERADOR
    TRISDbits.TRISD1 = 0; // LED_CONTROLE_CENTRAL
    TRISDbits.TRISD2 = 0; // LED_INJECAO_ELETRONICA
    TRISDbits.TRISD3 = 0; // LED_CONTROLE_ESTABILIDADE

    INTCON2bits.RBPU = 0; // Habilita pull-ups no PORTB (se necessário)
    
    // Inicializa LEDs desligados
    LED_ACELERADOR = OFF;
    LED_CONTROLE_CENTRAL = OFF;
    LED_INJECAO_ELETRONICA = OFF;
    LED_CONTROLE_ESTABILIDADE = OFF;
    
    // Inicializa pipe e mutex
    create_pipe(&pipe_acelerador_central);
    mutex_init(&mutex_injecao);
    sem_init(&sem_freio, 0); // Semáforo para o evento de freio
    
    // Configura interrupção externa para o pino RB0 (INT0) para simular o freio
    // Borda de subida, e registra o callback
    ext_interrupt_init(0, 1, freio_interrupt_callback); // INT0, edge_type = 1 (subida)

    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica");
}