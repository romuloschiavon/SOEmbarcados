#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"
#include "io.h"

// Implementação das tarefas do sistema
TASK tarefa_acelerador()
{
    unsigned int adc_value = 0;
    uint8_t adc_scaled = 0;

    LED_ACELERADOR = OFF;

    while (1) {
        // Lê o valor do potenciômetro (acelerador)
        adc_value = adc_read();

        // Converte de 10 bits para 8 bits para facilitar o processamento
        adc_scaled = (uint8_t)(adc_value >> 2);
        
        // LED indica quando o acelerador está pressionado (acima de 50%)
        LED_ACELERADOR = (adc_value > 512) ? ON : OFF;

        // Envia dados para o controle central via pipe
        write_pipe(&pipe_acelerador_central, adc_scaled);

        delay(25);
    }
}

TASK tarefa_controle_central()
{
    uint8_t acelerador_value;
    LED_CONTROLE_CENTRAL = OFF;

    while (1) {
        // Recebe dados do acelerador através do pipe
        read_pipe(&pipe_acelerador_central, &acelerador_value);
        
        LED_CONTROLE_CENTRAL = ON;
        
        // Protege o acesso à variável compartilhada usando mutex
        mutex_lock(&mutex_injecao);
        g_tempo_injecao = acelerador_value * 4; // Calcula tempo de injeção
        mutex_unlock(&mutex_injecao);
        
        // Sinaliza que há novos dados para a injeção eletrônica
        sem_post(&sem_injecao);

        delay(10);

        LED_CONTROLE_CENTRAL = OFF;
    }
}

TASK tarefa_injecao_eletronica()
{
    uint16_t tempo_local = 0;

    while (1) {
        // Aguarda sinal de que há novos dados de injeção
        sem_wait(&sem_injecao);
        
        // Lê o tempo de injeção de forma segura
        mutex_lock(&mutex_injecao);
        if(freio){
            g_tempo_injecao = 0; // Se freio ativado, corta combustível
        }
        tempo_local = g_tempo_injecao;
        mutex_unlock(&mutex_injecao);

        // Controla o bico injetor via PWM
        if (tempo_local > 255){
            pwm_set_duty(tempo_local);
            LED_INJECAO_ELETRONICA = ON;
        } else {
            pwm_set_duty(0); // Motor parado
        }
        
        delay(5);
        LED_INJECAO_ELETRONICA = OFF;
    }
}

TASK tarefa_controle_estabilidade(void)
{
    // Pequeno delay para evitar bouncing do botão
    delay(10);
    
    // Aguarda sinal do freio de emergência
    sem_wait(&sem_freio);

    freio = 1;
    
    // Loop principal do controle de estabilidade
    while(1)
    {
        // Para o motor imediatamente
        mutex_lock(&mutex_injecao);
        g_tempo_injecao = 0;
        mutex_unlock(&mutex_injecao);
        
        // Garante que o PWM está em 0
        pwm_set_duty(0);
        
        // Verifica se o botão de freio ainda está pressionado
        if(PORTBbits.RB0 == 0) {
            // LED piscando indica sistema de emergência ativo
            LED_CONTROLE_ESTABILIDADE = ON;
            delay(250);
            
            LED_CONTROLE_ESTABILIDADE = OFF;
            delay(250);
            
        } else {
            // Botão foi solto, sistema pode voltar ao normal
            freio = 0;
            LED_CONTROLE_ESTABILIDADE = OFF;
            
            estabilidade = 0;
            
            // Remove esta tarefa da execução
            remove_task(tarefa_controle_estabilidade);
            
            // Yield infinito (nunca deveria executar após remove_task)
            while(1) {
                yield();
            }
        }
    }
}

void ISR_FREIO(void)
{
    if (INTCONbits.INT0IF){
        // Limpa a flag de interrupção
        INTCONbits.INT0IF = 0;
        
        __delay_ms(5); // Debounce básico
        
        // Verifica se é uma pressão válida e se não há outro controle ativo
        if(PORTBbits.RB0 == 0 && !estabilidade) 
        {
            freio = 1;
            estabilidade = 1;
            sem_post(&sem_freio);
            // Cria tarefa de emergência com prioridade máxima
            create_task(4, 4, tarefa_controle_estabilidade);
        }
    }
}

void user_config(void)
{
    // Configura pinos dos LEDs como saída
    TRISDbits.TRISD0 = 0;  // LED do acelerador
    TRISDbits.TRISD1 = 0;  // LED do controle central
    TRISDbits.TRISD2 = 0;  // LED da injeção eletrônica
    TRISDbits.TRISD4 = 0;  // LED do controle de estabilidade
    
    // Configura botão do freio como entrada
    TRISBbits.TRISB0 = 1;

    // Inicializa todos os LEDs apagados
    LED_ACELERADOR = OFF;
    LED_CONTROLE_CENTRAL = OFF;
    LED_INJECAO_ELETRONICA = OFF;
    LED_CONTROLE_ESTABILIDADE = OFF;

    // Inicializa variáveis de controle
    estabilidade = 0;
    freio = 0;

    // Inicializa recursos de sincronização
    create_pipe(&pipe_acelerador_central);
    mutex_init(&mutex_injecao);
    sem_init(&sem_freio, 0);        // Inicialmente bloqueado
    sem_init(&sem_injecao, 0);      // Inicialmente bloqueado

    // Inicializa periféricos
    adc_init();    // Para leitura do potenciômetro
    pwm_init();    // Para controle do bico injetor

    // Configura interrupção externa para o freio (borda de descida)
    ext_interrupt_init(0, 0);

    // Garante que as funções sejam visíveis globalmente para o linker
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}