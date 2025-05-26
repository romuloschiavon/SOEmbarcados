#ifndef IO_H
#define	IO_H

#include "config.h"
#include <xc.h>

typedef enum {
    PWM_MODULE_CCP1 = 1,    // CCP1 padrão (RC2) - Bico 1
    PWM_MODULE_CCP2 = 2,    // CCP2 padrão (RC1) - Bico 2
    PWM_MODULE_TIMER1 = 3   // Timer1 PWM (RC0) - Bico 3 (solução híbrida)
} pwm_module_t;

// Inicializa um módulo CCP para operar em modo PWM.
void pwm_init(pwm_module_t module, unsigned int frequency);

// Define o duty cycle do sinal PWM.
void pwm_set_duty_cycle(pwm_module_t module, unsigned int duty_cycle_value);

// Inicia a geração do sinal PWM.
void pwm_start(void);

// Para a geração do sinal PWM.
void pwm_stop(void);

// Atualiza PWM via Timer1 (terceiro bico)
void timer1_pwm_update(void);

// --- API para ADC (a ser implementada) ---
void adc_init(void);
unsigned int adc_read(void);

// --- API para Interrupção Externa ---
typedef void (*interrupt_callback_t)(void); // Callback para a ISR
void ext_interrupt_init(unsigned char int_num, unsigned char edge_type, interrupt_callback_t callback_function);
void ext_interrupt_handler(void); // Declaração para a ISR

#endif	/* IO_H */

