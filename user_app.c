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
    adc_init(0); // Canal AN0

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
        
        delay(50); // Pequeno delay para não sobrecarregar
    }    
}

TASK tarefa_controle_central() // Prioridade 2
{
    uint8_t acelerador_value;
    uint16_t tempo_injecao_calculado;
    
    while (1) {
        // Liga LED do controle central quando processa dados
        LED_CONTROLE_CENTRAL = 1;
        
        // Recebe valor do acelerador via pipe
        read_pipe(&pipe_acelerador_central, &acelerador_value);
        
        // Calcula tempo de injeção baseado no valor do acelerador
        // Converte de 0-255 para 0-1023 (duty cycle PWM)
        tempo_injecao_calculado = (uint16_t)acelerador_value * 4;
        
        // Protege acesso à variável compartilhada com mutex
        mutex_lock(&mutex_injecao);
        g_tempo_injecao = tempo_injecao_calculado;
        mutex_unlock(&mutex_injecao);
        
        delay(10); // Delay curto para permitir outras tarefas
        
        // Desliga LED após processamento
        LED_CONTROLE_CENTRAL = 0;
    }    
}

TASK tarefa_injecao_eletronica() // Prioridade 3 (mais alta)
{
    uint16_t duty_cycle_atual;    // Inicializa os três módulos PWM para os bicos injetores
    pwm_init(BICO_INJETOR_1, 1000); // Bico 1 - RC2/CCP1 - 1kHz
    pwm_init(BICO_INJETOR_2, 1000); // Bico 2 - RC1/CCP2 - 1kHz  
    pwm_init(BICO_INJETOR_3, 1000); // Bico 3 - RC0/Timer1 - 1kHz
    
    // liga o PWM para os bicos injetores 1 e 2
    T2CONbits.TMR2ON = 1;
    
    while (1) {
        // Protege acesso à variável compartilhada com mutex
        mutex_lock(&mutex_injecao);
        duty_cycle_atual = g_tempo_injecao;
        mutex_unlock(&mutex_injecao);
        
        // Calcula duty cycle para os três bicos baseado na posição do acelerador
        duty_cycle_bico1 = duty_cycle_atual;                    // Bico 1: duty cycle direto
        duty_cycle_bico2 = (duty_cycle_atual * 90) / 100;       // Bico 2: 90% do valor
        duty_cycle_bico3 = (duty_cycle_atual * 80) / 100;       // Bico 3: 80% do valor        // Aplica PWM aos três bicos injetores
        pwm_set_duty_cycle(BICO_INJETOR_1, duty_cycle_bico1);  // Bico 1 (RC2/CCP1)
        pwm_set_duty_cycle(BICO_INJETOR_2, duty_cycle_bico2);  // Bico 2 (RC1/CCP2)
        pwm_set_duty_cycle(BICO_INJETOR_3, duty_cycle_bico3);  // Bico 3 (RC0/Timer1)
        
        // Controla LED baseado na atividade dos bicos
        if (duty_cycle_atual > 100) { // Threshold baixo para detectar atividade
            LED_INJECAO_ELETRONICA = 1; // Liga LED quando injeção ativa
        } else {
            LED_INJECAO_ELETRONICA = 0; // Desliga LED quando injeção inativa
        }
        
        delay(5); // Delay curto para atualização rápida do PWM
   }
}

// Callback para interrupção de freio
void freio_interrupt_callback(void) {
    // Cria tarefa de controle de estabilidade dinamicamente (one shot)
    create_task(4, 4, tarefa_controle_estabilidade);
}

TASK tarefa_controle_estabilidade() // Prioridade 4 (mais alta de usuário) - ONE SHOT
{
    // Acionamento imediato dos "freios" (one shot)
    LED_CONTROLE_ESTABILIDADE = 1; // Liga LED de freio
    
    // Para todos os bicos PWM imediatamente
    pwm_set_duty_cycle(BICO_INJETOR_1, 0); // Para Bico 1 (RC2/CCP1)
    pwm_set_duty_cycle(BICO_INJETOR_2, 0); // Para Bico 2 (RC1/CCP2)
    pwm_set_duty_cycle(BICO_INJETOR_3, 0); // Para Bico 3 (RC0/Timer1)
    
    // Para todas as outras tarefas (emergência)
    LED_ACELERADOR = 0;
    LED_CONTROLE_CENTRAL = 0;
    LED_INJECAO_ELETRONICA = 0;

    // Simula tempo de frenagem (one shot)
    delay(100);
    
    // Desliga LED de freio
    LED_CONTROLE_ESTABILIDADE = 0;
    
    // Tarefa one shot termina aqui (não entra em loop)
}
}

void user_config()
{
    TRISDbits.TRISD0 = 0; // RD0 como saída (LED tarefa_acelerador)
    TRISDbits.TRISD1 = 0; // RD1 como saída (LED tarefa_controle_central)
    TRISDbits.TRISD2 = 0; // RD2 como saída (LED tarefa_injecao_eletronica)
    TRISDbits.TRISD3 = 0; // RD3 como saída (LED tarefa_controle_estabilidade)
    TRISDbits.TRISD7 = 0; // RD7 como saída (LED idle debug)

    // Inicializa todos os LEDs desligados
    LED_ACELERADOR = 0;
    LED_CONTROLE_CENTRAL = 0;
    LED_INJECAO_ELETRONICA = 0;
    LED_CONTROLE_ESTABILIDADE = 0;
    LATDbits.LATD7 = 0; // LED idle debug inicialmente desligado

    // Configuração dos pinos conforme especificação do usuário
    
    // Acelerador (Potenciômetro)
    TRISAbits.TRISA0 = 1; // RA0/AN0 como entrada    // Três Bicos Injetores PWM
    TRISCbits.TRISC2 = 0; // RC2 como saída para Bico 1 (CCP1)
    TRISCbits.TRISC1 = 0; // RC1 como saída para Bico 2 (CCP2)
    TRISCbits.TRISC0 = 0; // RC0 como saída para Bico 3 (Timer1)
    
    // Inicializa os três bicos desligados
    LATCbits.LATC2 = 0; // Bico 1 inicialmente desligado
    LATCbits.LATC1 = 0; // Bico 2 inicialmente desligado  
    LATCbits.LATC0 = 0; // Bico 3 inicialmente desligado

    // Freio (Interrupção Externa)
    TRISBbits.TRISB0 = 1; // RB0/INT0 como entrada
    
    // Inicializar pipes, mutex e semáforos
    create_pipe(&pipe_acelerador_central);
    mutex_init(&mutex_injecao);
    sem_init(&sem_freio, 0); // Inicialmente bloqueado
    
    // Configurar interrupção externa para o freio
    ext_interrupt_init(0, 0, freio_interrupt_callback); // INT0, falling edge

    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}