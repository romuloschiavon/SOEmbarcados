#include "scheduler.h"
#include "config.h"
#include "types.h"

// ReferÃªncia para a fila de aptos
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
    do
    {
        r_queue.task_running = (r_queue.task_running + 1) % r_queue.ready_queue_size;
        if (r_queue.task_running == 0)
        {
            idle_task++;
            if (idle_task == 2)
                break;
        }
    } while (r_queue.ready_queue[r_queue.task_running].task_state != READY ||
             r_queue.task_running == 0);
}

void __reentrant priority_scheduler()
{
    uint8_t next_task_idx = 0; // Por padrÃ£o, aponta para a tarefa Idle
    uint8_t highest_priority = 0;
    uint8_t found_user_task = 0;    // Procura pela tarefa de maior prioridade pronta (excluindo a Idle inicialmente)
    // Iterar de 1 porque a tarefa 0 Ã© a Idle
    for (uint8_t i = 1; i < r_queue.ready_queue_size; i++)
    {
        if (r_queue.ready_queue[i].task_state == READY)
        {
            // Se Ã© a primeira tarefa encontrada ou tem prioridade maior
            if (!found_user_task || r_queue.ready_queue[i].task_priority > highest_priority)
            {
                highest_priority = r_queue.ready_queue[i].task_priority;
                next_task_idx = i;
                found_user_task = 1;
            }
            // Se tem mesma prioridade, mantÃ©m a primeira encontrada (menor Ã­ndice)
        }
    }

    // Se nenhuma tarefa de usuÃ¡rio estiver pronta, ou se a tarefa Idle for a Ãºnica opÃ§Ã£o
    // e a tarefa Idle estiver pronta, seleciona a Idle.
    // A tarefa Idle (Ã­ndice 0) sempre estÃ¡ "pronta" em termos de estado lÃ³gico,
    // mas sÃ³ deve rodar se nenhuma outra de maior prioridade estiver.
    if (!found_user_task && r_queue.ready_queue[0].task_state == READY)
    {
        r_queue.task_running = 0; // Tarefa Idle
    }
    else if (found_user_task)
    {
        r_queue.task_running = next_task_idx;
    }
    else
    {
        // Fallback para Idle se algo inesperado ocorrer e nenhuma tarefa for encontrada.
        r_queue.task_running = 0;
    }
}
