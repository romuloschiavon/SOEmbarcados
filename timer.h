#ifndef TIMER_H
#define	TIMER_H

void config_timer0(void);
void start_timer0(void);

// Tratador de interrup��o do timer
void __interrupt() ISR_TMR0(void);

#endif	/* TIMER_H */

