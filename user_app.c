#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"
#include "io.h"

// ===== IMPLEMENTAÃÃO DAS TAREFAS =====
TASK tarefa_acelerador()
{
    unsigned int adc_value = 0;
    uint8_t adc_scaled = 0;

    LED_ACELERADOR = OFF;

    while (1) {
        // LÃª acelerador (potenciÃ´metro)
        adc_value = adc_read();

        // Processa leitura do ADC
        adc_scaled = (uint8_t)(adc_value >> 2);  // Converte de 10 para 8 bits
        
        // Feedback visual
        LED_ACELERADOR = (adc_value > 512) ? ON : OFF;

        // PIPE: ComunicaÃ§Ã£o com controle central
        write_pipe(&pipe_acelerador_central, adc_scaled);

        delay(25);
    }
}

TASK tarefa_controle_central()
{
    uint8_t acelerador_value;
    LED_CONTROLE_CENTRAL = OFF;

    while (1) {
        // PIPE: Recebe dados do acelerador
        read_pipe(&pipe_acelerador_central, &acelerador_value);
        
        LED_CONTROLE_CENTRAL = ON;
        
        // MUTEX: Protege o acesso Ã  variÃ¡vel compartilhada
        mutex_lock(&mutex_injecao);
        g_tempo_injecao = acelerador_value * 4;
        mutex_unlock(&mutex_injecao);
        
        sem_post(&sem_injecao);

        delay(10);

        LED_CONTROLE_CENTRAL = OFF;
    }
}

TASK tarefa_injecao_eletronica()
{
    uint16_t tempo_local = 0;

    while (1) {
        // SEMÃFORO: Aguarda novos dados de injeÃ§Ã£o
        sem_wait(&sem_injecao);
        
        // MUTEX: Acesso seguro Ã  variÃ¡vel compartilhada
        mutex_lock(&mutex_injecao);
        if(freio){
            g_tempo_injecao = 0;
        }
        tempo_local = g_tempo_injecao;
        mutex_unlock(&mutex_injecao);

        if (tempo_local > 255){
            pwm_set_duty(tempo_local);
            LED_INJECAO_ELETRONICA = ON;
        } else {
            pwm_set_duty(0);
        }
        
        delay(5);
        LED_INJECAO_ELETRONICA = OFF;
    }
}

TASK tarefa_controle_estabilidade(void)
{
    // Debounce
    delay(10);
    
    // Aguarda sinal do semáforo
    sem_wait(&sem_freio);

    freio = 1;
    
    // ✅ MODIFICADO: Loop infinito para manter motor parado e LED piscando
    while(1)
    {
        // ONE-SHOT: Executa uma única vez
        mutex_lock(&mutex_injecao);
        g_tempo_injecao = 0;
        mutex_unlock(&mutex_injecao);
        
        // Para o motor (RC2)
        pwm_set_duty(0);
        
        // ✅ CORRIGIDO: Agora acende usando LAT em vez de PORT
        // ✅ CORRIGIDO: Verifica se o botão ainda está pressionado
        if(PORTBbits.RB0 == 0) {
            // Liga o LED
            LED_CONTROLE_ESTABILIDADE = ON;
            delay(250);  // Meio segundo ligado
            
            LED_CONTROLE_ESTABILIDADE = OFF;
            delay(250);  // Meio segundo desligado
            
            // Continua em loop enquanto o botão estiver pressionado
        } else {
            // ✅ CORRIGIDO: Quando soltar o botão, motor pode voltar a funcionar
            freio = 0;
            LED_CONTROLE_ESTABILIDADE = OFF;
            
            // Desativa a tarefa
            estabilidade = 0;
            
            // Remove a própria tarefa
            remove_task(tarefa_controle_estabilidade);
            
            // Infinity yield (nunca deve chegar aqui após remove_task)
            while(1) {
                yield();
            }
        }
    }
}

void ISR_FREIO(void)
{
    if (INTCONbits.INT0IF){
    // Limpa flag de interrupÃ§Ã£o
        INTCONbits.INT0IF = 0;
        
        __delay_ms(5); // Debounce
        if(PORTBbits.RB0 == 0 && !estabilidade) 
        {
            freio = 1;
            estabilidade = 1;
            sem_post(&sem_freio);
            create_task(4, 4, tarefa_controle_estabilidade); // Prioridade mï¿½xima
        }
    }
    
}

void user_config(void)
{
    // Configura LEDs
    TRISDbits.TRISD0 = 0;  // LED_ACELERADOR
    TRISDbits.TRISD1 = 0;  // LED_CONTROLE_CENTRAL
    TRISDbits.TRISD2 = 0;  // LED_INJECAO_ELETRONICA
    TRISDbits.TRISD4 = 0;  // LED_CONTROLE_ESTABILIDADE (✅ VERIFICADO: Está OK)
    
    // Configura RB0 como entrada para o freio
    TRISBbits.TRISB0 = 1;

    // ✅ VERIFICADO: Inicializa LEDs apagados (usa LAT, não PORT)
    LED_ACELERADOR = OFF;
    LED_CONTROLE_CENTRAL = OFF;
    LED_INJECAO_ELETRONICA = OFF;
    LED_CONTROLE_ESTABILIDADE = OFF;

    // ✅ CORRIGIDO: Garantir que as variáveis de estado estão inicializadas
    estabilidade = 0;
    freio = 0;

    // Inicializa recursos de sincronização
    create_pipe(&pipe_acelerador_central);
    mutex_init(&mutex_injecao);
    sem_init(&sem_freio, 0);
    sem_init(&sem_injecao, 0);

    // Inicializa hardware
    adc_init();    // Para leitura do acelerador
    pwm_init();    // Para controle do bico injetor via RC2

    // Configura interrupção do freio
    ext_interrupt_init(0, 0);

    // Garantir que as funções sejam globais para o linker
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}