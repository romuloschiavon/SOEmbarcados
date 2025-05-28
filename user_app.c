#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"
#include "io.h"

// ===== RECURSOS DE COMUNICAÇÃO E SINCRONIZAÇÃO =====
pipe_t pipe_acelerador_central;   // Para comunicação unidirecional por dados
mutex_t mutex_injecao;            // Para proteção de recursos compartilhados
sem_t sem_freio;                  // Para sinalização de eventos
sem_t sem_injecao;                // Para sinalização de novos dados de injeção

// Variável protegida pelo mutex
unsigned int g_tempo_injecao = 0;

// ===== IMPLEMENTAÇÃO DAS TAREFAS =====

TASK tarefa_acelerador()
{
    unsigned int adc_value = 0;
    uint8_t adc_scaled = 0;

    adc_init();

    while (1) {
        // Lê acelerador (potenciômetro)
        adc_value = adc_read();

        // Processa leitura do ADC
        adc_scaled = (uint8_t)(adc_value >> 2);  // Converte de 10 para 8 bits
        
        // Feedback visual
        LED_ACELERADOR = (adc_value > 512) ? ON : OFF;

        // PIPE: Comunicação unidirecional com controle central
        write_pipe(&pipe_acelerador_central, adc_scaled);

        delay(10);
    }
}

TASK tarefa_controle_central()
{
    uint8_t acelerador_value;
    uint16_t tempo_injecao_calculado;

    while (1) {
        // PIPE: Recebe dados do acelerador
        read_pipe(&pipe_acelerador_central, &acelerador_value);

        // Calcula tempo de injeção baseado no acelerador
        if (acelerador_value >= 25) {
            tempo_injecao_calculado = acelerador_value * 2;
            
            LED_CONTROLE_CENTRAL = ON;
            
            // MUTEX: Protege o acesso à variável compartilhada
            mutex_lock(&mutex_injecao);
            g_tempo_injecao = tempo_injecao_calculado;
            mutex_unlock(&mutex_injecao);
            
            // SEMÁFORO: Sinaliza que há novos dados disponíveis
            sem_post(&sem_injecao);
        } else {
            LED_CONTROLE_CENTRAL = OFF;
            
            // ✅ ZERA INJEÇÃO quando acelerador baixo
            mutex_lock(&mutex_injecao);
            g_tempo_injecao = 0;
            mutex_unlock(&mutex_injecao);
            
            sem_post(&sem_injecao);  // ✅ Sempre sinaliza
        }

        delay(10);
    }
}

TASK tarefa_injecao_eletronica()
{
    uint16_t tempo_local = 0;
    pwm_init();

    while (1) {
        // SEMÁFORO: Aguarda novos dados de injeção
        sem_wait(&sem_injecao);
        
        // MUTEX: Acesso seguro à variável compartilhada
        mutex_lock(&mutex_injecao);
        tempo_local = g_tempo_injecao;
        mutex_unlock(&mutex_injecao);

        if (tempo_local > 0) {
            // ✅ SEM DIVISÃO - usa valor direto (limitado a 255)
            uint8_t duty = (tempo_local > 255) ? 255 : (uint8_t)tempo_local;
            pwm_set_duty(duty); 
            LED_INJECAO_ELETRONICA = ON;
        } else {
            pwm_set_duty(0);
            LED_INJECAO_ELETRONICA = OFF;
        }
        
        delay(50);  // ✅ Delay visível para LED
    }
}

TASK tarefa_controle_estabilidade()
{
    while (1) {
        // SEMÁFORO: Bloqueia aguardando sinal do botão de freio
        // Esse é o uso correto do semáforo para este caso de uso
        sem_wait(&sem_freio);
        
        // Quando semáforo é liberado = Freio acionado!
        LED_CONTROLE_ESTABILIDADE = ON;
        
        // MUTEX: Acesso seguro à variável compartilhada
        // Zera tempo de injeção em emergência
        mutex_lock(&mutex_injecao);
        g_tempo_injecao = 0;
        mutex_unlock(&mutex_injecao);
        
        // Aguarda soltar o freio (polling necessário para este hardware)
        while (PORTBbits.RB0 == 0) {
            delay(50);
        }
        
        // Freio liberado
        LED_CONTROLE_ESTABILIDADE = OFF;
    }
}

// ISR para freio - Simples e eficiente
void freio_interrupt_callback(void)
{
    // SEMÁFORO: Sinaliza a tarefa de controle de estabilidade
    sem_post(&sem_freio);    
}

void user_config(void)
{
    // Configura pinos de saída para LEDs
    TRISDbits.TRISD0 = 0;  // LED_ACELERADOR
    TRISDbits.TRISD1 = 0;  // LED_CONTROLE_CENTRAL
    TRISDbits.TRISD2 = 0;  // LED_INJECAO_ELETRONICA
    TRISDbits.TRISD3 = 0;  // LED_CONTROLE_ESTABILIDADE
    TRISDbits.TRISD4 = 0;  // LED extra se necessário
    
    // Configura RB0 como entrada para o freio
    TRISBbits.TRISB0 = 1;

    // Inicializa LEDs apagados
    LED_ACELERADOR = OFF;
    LED_CONTROLE_CENTRAL = OFF;
    LED_INJECAO_ELETRONICA = OFF;
    LED_CONTROLE_ESTABILIDADE = OFF;

    // Inicializa recursos de sincronização
    create_pipe(&pipe_acelerador_central);  // PIPE: Comunicação Acelerador → Controle
    mutex_init(&mutex_injecao);             // MUTEX: Proteção do tempo de injeção
    sem_init(&sem_freio, 0);                // SEMÁFORO: Notificação do freio
    sem_init(&sem_injecao, 0);              // SEMÁFORO: Notificação de novos dados de injeção

    // Configura interrupção do freio
    // edge_type = 0 para borda de descida (quando chave fecha)
    ext_interrupt_init(0, 0, freio_interrupt_callback);

    // Garantir que as funções sejam globais para o linker
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}