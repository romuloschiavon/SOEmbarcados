#ifndef CONFIG_H
#define	CONFIG_H

#define ON                  1
#define OFF                 2

#define MAX_USER_TASKS      4
#define MAX_STACK_SIZE      32

#define DEFAULT_SCHEDULER   PRIORITY_SCHEDULER

#define IDLE_DEBUG          ON

#define DYNAMIC_MEM         ON

#define PIPE_SIZE           4

#define _XTAL_FREQ 8000000  // cristal interno

#define LED_ACELERADOR  LATDbits.LATD0
#define LED_CONTROLE_CENTRAL  LATDbits.LATD1
#define LED_INJECAO_ELETRONICA  LATDbits.LATD2
#define LED_CONTROLE_ESTABILIDADE  LATDbits.LATD3

// Definições para os bicos injetores PWM - SOLUÇÃO ROBUSTA!
#define BICO_INJETOR_1  PWM_MODULE_CCP1    // RC2/CCP1 - PWM Hardware
#define BICO_INJETOR_2  PWM_MODULE_CCP2    // RC1/CCP2 - PWM Hardware  
#define BICO_INJETOR_3  PWM_MODULE_TIMER1  // RC0 - PWM via Timer1 (híbrido)

#endif	/* CONFIG_H */

