#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"
#include "io.h"

pipe_t pipe_acelerador_central;
mutex_t mutex_injecao;
unsigned int g_tempo_injecao = 0;
sem_t sem_freio;

TASK tarefa_acelerador()
{
    unsigned int adc_value = 0;
    uint8_t adc_scaled = 0;

    adc_init();

    while (1) {
        adc_value = adc_read();

        if (adc_value > 512) {
            LED_ACELERADOR = ON;
        } else {
            LED_ACELERADOR = OFF;
        }

        adc_scaled = (uint8_t)(adc_value >> 2);
        write_pipe(&pipe_acelerador_central, adc_scaled);

        delay(10);
    }
}

TASK tarefa_controle_central()
{
    uint8_t acelerador_value;
    uint16_t tempo_injecao_calculado;

    while (1) {
        read_pipe(&pipe_acelerador_central, &acelerador_value);

        if (acelerador_value >= 25) {
            tempo_injecao_calculado = acelerador_value * 4;

            LED_CONTROLE_CENTRAL = ON;

            mutex_lock(&mutex_injecao);
            g_tempo_injecao = tempo_injecao_calculado;
            mutex_unlock(&mutex_injecao);
        }

        LED_CONTROLE_CENTRAL = OFF;
        delay(10);
    }
}

TASK tarefa_injecao_eletronica()
{
    uint16_t duty;

    pwm_init();

    while (1) {
        mutex_lock(&mutex_injecao);
        duty = g_tempo_injecao;
        mutex_unlock(&mutex_injecao);

        if (duty > 0) {
            LED_INJECAO_ELETRONICA = ON;
            pwm_set_duty((uint8_t)(duty >> 2)); // Convert 10-bit to 8-bit
        } else {
            LED_INJECAO_ELETRONICA = OFF;
            pwm_set_duty(0);
        }

        delay(5);
    }
}

TASK tarefa_controle_estabilidade()
{
    sem_wait(&sem_freio);

    while (PORTBbits.RB0 == 1) {
        LED_CONTROLE_ESTABILIDADE = ON;

        __delay_ms(300);

        mutex_lock(&mutex_injecao);
        g_tempo_injecao = 0;
        mutex_unlock(&mutex_injecao);

        pwm_set_duty(0);

        delay(50);
    }

    LED_ACELERADOR = OFF;
    LED_CONTROLE_ESTABILIDADE = OFF;

    remove_task(tarefa_controle_estabilidade);

    while (1) {
        yield();
    }
}

void freio_interrupt_callback(void)
{
    create_task(4, 4, tarefa_controle_estabilidade);
    sem_post(&sem_freio);
}

void user_config(void)
{
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 0;

    INTCON2bits.RBPU = 0;

    LED_ACELERADOR = OFF;
    LED_CONTROLE_CENTRAL = OFF;
    LED_INJECAO_ELETRONICA = OFF;
    LED_CONTROLE_ESTABILIDADE = OFF;

    create_pipe(&pipe_acelerador_central);
    mutex_init(&mutex_injecao);
    sem_init(&sem_freio, 0);

    ext_interrupt_init(0, 1, freio_interrupt_callback);

    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica");
}
