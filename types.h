#ifndef TYPES_H
#define	TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <xc.h>

#include "config.h"

#define RR_SCHEDULER            1
#define PRIORITY_SCHEDULER      2


// Tipos de dados

typedef void TASK;

typedef void (*f_ptr)(void);

typedef unsigned char byte;

typedef enum {
    READY         = 0, 
    RUNNING       = 1,
    WAITING       = 2,
    SEM_WAITING   = 3
} state_t;

typedef struct tcb {
    uint8_t task_id;
    uint8_t task_priority;
    state_t task_state;
    f_ptr task_f;
    uint16_t time_sleeping;
    // Hardware
    byte W_REG;
    byte STATUS_REG;
    byte BSR_REG;
    // Pilha da tarefa
    uint24_t task_stack[MAX_STACK_SIZE];
    uint8_t task_sp;    
} tcb_t;

// Filas

typedef struct ready_queue {
    tcb_t ready_queue[MAX_USER_TASKS+1];
    uint8_t task_running;
    uint8_t ready_queue_size;
    tcb_t *task;
} ready_queue_t;


#endif	/* TYPES_H */

