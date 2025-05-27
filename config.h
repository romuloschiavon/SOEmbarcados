#ifndef CONFIG_H
#define CONFIG_H

#define ON                  1
#define OFF                 0

#define MAX_USER_TASKS      4
#define MAX_STACK_SIZE      32

#define DEFAULT_SCHEDULER   PRIORITY_SCHEDULER

#define IDLE_DEBUG          ON

#define DYNAMIC_MEM         ON

#define PIPE_SIZE           3

#define _XTAL_FREQ          4000000UL

// LEDs de status
#define LED_ACELERADOR              LATDbits.LATD0
#define LED_CONTROLE_CENTRAL        LATDbits.LATD1
#define LED_INJECAO_ELETRONICA      LATDbits.LATD2
#define LED_CONTROLE_ESTABILIDADE   LATDbits.LATD3

// Bicos injetores controlados por PWM nas saídas ECCP1
#define BICO_INJETOR_1  0  // RD5 - P1B
#define BICO_INJETOR_2  1  // RD6 - P1C
#define BICO_INJETOR_3  2  // RD7 - P1D

#endif /* CONFIG_H */
