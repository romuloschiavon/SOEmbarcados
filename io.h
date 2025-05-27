#ifndef IO_H
#define IO_H

// PWM
void pwm_init(void);
void pwm_set_duty(uint8_t duty);

// Interrupção externa (freio)
void ext_interrupt_handler(void);

// ADC
void adc_init(void);
uint16_t adc_read(void);

#endif /* IO_H */