#include "scheduler.h"
#include "config.h"
#include "types.h"

// Referência para a fila de aptos
extern ready_queue_t r_queue;


void __reentrant scheduler()
{
    #if DEFAULT_SCHEDULER == RR_SCHEDULER
    // chama rr scheduler
    rr_scheduler();
    #elif DEFAULT_SCHEDULER == PRIORITY_SCHEDULER
    // chama priority_scheduler
    priority_scheduler();
    #endif
}

void __reentrant rr_scheduler()
{
    uint8_t idle_task = 0;
    do {    
        r_queue.task_running = (r_queue.task_running+1) % r_queue.ready_queue_size;
        if (r_queue.task_running == 0) {
            idle_task++;
            if (idle_task == 2) break;
        }
    } while (r_queue.ready_queue[r_queue.task_running].task_state != READY ||
             r_queue.task_running == 0);
}

void __reentrant priority_scheduler()
{
    // Ordenar a fila de aptos por prioridade
    // 
}
