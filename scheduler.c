#include "scheduler.h"
#include "config.h"
#include "types.h"

// Referência para a fila de tarefas prontas
extern ready_queue_t r_queue;

void __reentrant scheduler()
{
#if DEFAULT_SCHEDULER == RR_SCHEDULER
    // Usa escalonamento Round Robin
    rr_scheduler();
#elif DEFAULT_SCHEDULER == PRIORITY_SCHEDULER
    // Usa escalonamento por prioridade
    priority_scheduler();
#endif
}

void __reentrant rr_scheduler()
{
    uint8_t idle_task = 0;
    
    // Busca próxima tarefa pronta em ordem circular
    do
    {
        r_queue.task_running = (r_queue.task_running + 1) % r_queue.ready_queue_size;
        
        // Controla quantas vezes passou pela tarefa idle
        if (r_queue.task_running == 0)
        {
            idle_task++;
            if (idle_task == 2)  // Se passou 2 vezes, não há tarefas prontas
                break;
        }
    } while (r_queue.ready_queue[r_queue.task_running].task_state != READY ||
             r_queue.task_running == 0);
}

void __reentrant priority_scheduler()
{
    uint8_t next_task_idx = 0;      // Por padrão executa tarefa Idle
    uint8_t highest_priority = 0;
    uint8_t found_user_task = 0;
    
    // Procura tarefa de usuário com maior prioridade (exclui Idle inicialmente)
    for (uint8_t i = 1; i < r_queue.ready_queue_size; i++)
    {
        if (r_queue.ready_queue[i].task_state == READY)
        {
            // Se é a primeira encontrada ou tem prioridade maior que a atual
            if (!found_user_task || r_queue.ready_queue[i].task_priority > highest_priority)
            {
                highest_priority = r_queue.ready_queue[i].task_priority;
                next_task_idx = i;
                found_user_task = 1;
            }
            // Em caso de empate de prioridade, mantém a primeira (menor índice)
        }
    }

    // Define qual tarefa executar baseado no que foi encontrado
    if (!found_user_task && r_queue.ready_queue[0].task_state == READY)
    {
        r_queue.task_running = 0;  // Executa Idle se nenhuma tarefa pronta
    }
    else if (found_user_task)
    {
        r_queue.task_running = next_task_idx;  // Executa tarefa de maior prioridade
    }
    else
    {
        r_queue.task_running = 0;  // Fallback para Idle em caso de erro
    }
}
