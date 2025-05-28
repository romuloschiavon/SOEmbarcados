#ifndef IO_H
#define IO_H

#include <stdint.h>

// PWM (apenas RC2)
void pwm_init(void);
void pwm_set_duty(uint8_t duty);

// InterrupÃ§Ã£o externa (freio)
void ext_interrupt_init(uint8_t int_num, uint8_t edge_type);


// ADC
void adc_init(void);
uint16_t adc_read(void);

#endif /* IO_H */