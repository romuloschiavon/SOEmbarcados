#ifndef IO_H
#define IO_H

#include <stdint.h>

// PWM
void pwm_init(void);
void pwm_set_duty(uint8_t duty);

// Interrupção externa (freio)
void ext_interrupt_init(uint8_t int_num, uint8_t edge_type, void (*callback)(void));
void ext_interrupt_handler(void);

// ADC
void adc_init(void);
uint16_t adc_read(void);

// ISR Principal - versão normal
void ISR_PRINCIPAL(void);

#endif /* IO_H */