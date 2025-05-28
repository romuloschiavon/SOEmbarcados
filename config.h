#ifndef CONFIG_H
#define CONFIG_H

#define ON                  1
#define OFF                 0

#define MAX_USER_TASKS      5
#define MAX_STACK_SIZE      32

#define DEFAULT_SCHEDULER   PRIORITY_SCHEDULER

#define IDLE_DEBUG          OFF

#define DYNAMIC_MEM         ON

#define PIPE_SIZE           3

#define _XTAL_FREQ          4000000UL

// LEDs de status
#define LED_ACELERADOR              LATDbits.LATD0
#define LED_CONTROLE_CENTRAL        LATDbits.LATD1
#define LED_INJECAO_ELETRONICA      LATDbits.LATD2
#define LED_CONTROLE_ESTABILIDADE   LATDbits.LATD4

// Bico injetor controlado por PWM no CCP1 (RC2)
#define PWM_BICO_INJETOR     LATCbits.LATC2  // RC2 (CCP1)

#endif /* CONFIG_H */
