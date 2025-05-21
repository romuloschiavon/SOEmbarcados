#ifndef KERNEL_H
#define	KERNEL_H

#include "types.h"


extern ready_queue_t r_queue;

// Chamadas de sistema

void create_task(uint8_t id, uint8_t priority, f_ptr task);
void delay(uint16_t time);
void yield(void);
void os_init(void);
void os_start(void);
void change_state(state_t new_state);
void decrease_time(void);

// Tarefa idle
TASK idle();


// Troca de contexto
#define SAVE_CONTEXT(state) \
{ \
    do { \
        r_queue.task = &r_queue.ready_queue[r_queue.task_running]; \
        if (r_queue.task->task_state == RUNNING) { \
            r_queue.task->BSR_REG       = BSR; \
            r_queue.task->STATUS_REG    = STATUS; \
            r_queue.task->W_REG         = WREG; \
            r_queue.task->task_sp       = 0; \
            r_queue.task->task_state    = state; \
            while (STKPTR) { \
                r_queue.task->task_stack[r_queue.task->task_sp] = TOS; \
                r_queue.task->task_sp++; \
                asm("POP"); \
            } \
        } \
    } while(0); \
} \

#define RESTORE_CONTEXT() \
{\
    do { \
        r_queue.task = &r_queue.ready_queue[r_queue.task_running]; \
        if (r_queue.task->task_state == READY) { \
            STKPTR  = 0; \
            if (r_queue.task->task_sp > 0) { \
                BSR     = r_queue.task->BSR_REG; \
                STATUS  = r_queue.task->STATUS_REG; \
                WREG    = r_queue.task->W_REG; \
                do { \
                    asm("PUSH"); \
                    r_queue.task->task_sp--; \
                    TOS = r_queue.task->task_stack[r_queue.task->task_sp]; \
                } while (r_queue.task->task_sp); \
             } \
             else { \
                asm("PUSH"); \
                TOS = (uint24_t)r_queue.task->task_f; \
             }\
            r_queue.task->task_state = RUNNING; \
        } \
    } while(0); \
}\

#endif	/* KERNEL_H */

