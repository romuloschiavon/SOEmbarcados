#ifndef IO_H
#define	IO_H

#include <xc.h>

typedef enum {
    PWM_MODULE_CCP1 = 1,
    PWM_MODULE_CCP2 = 2
} pwm_module_t;

// Inicializa um módulo CCP para operar em modo PWM.
void pwm_init(pwm_module_t module, unsigned int frequency);

// Define o duty cycle do sinal PWM.
void pwm_set_duty_cycle(pwm_module_t module, unsigned int duty_cycle_value);

// Inicia a geração do sinal PWM.
void pwm_start(pwm_module_t module);

// Para a geração do sinal PWM.
void pwm_stop(pwm_module_t module);

// --- API para ADC (a ser implementada) ---
void adc_init(unsigned char channel);
unsigned int adc_read(void);

// --- API para Interrupção Externa (a ser implementada) ---
// typedef void (*interrupt_callback_t)(void); // Callback para a ISR
// void ext_interrupt_init(unsigned char int_num, unsigned char edge_type, interrupt_callback_t callback_function);

#endif	/* IO_H */

