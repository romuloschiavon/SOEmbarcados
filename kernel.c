#include "kernel.h"
#include "scheduler.h"
#include "user_app.h"
#include "timer.h"
#include "config.h"
#include "mem.h"

// Fila principal de tarefas do sistema
ready_queue_t r_queue;

void create_task(uint8_t id, uint8_t priority, f_ptr task)
{
    tcb_t new_task;
    
    // Inicializa parâmetros básicos da tarefa
    new_task.task_id            = id;
    new_task.task_priority      = priority;
    new_task.task_f             = task;
    
    // Estado inicial da tarefa
    new_task.task_sp            = 0;        // Stack pointer inicial
    new_task.time_sleeping      = 0;        // Não está em delay
    new_task.task_state         = READY;    // Pronta para execução
    
    // Registradores iniciais zerados
    new_task.BSR_REG            = 0x000;
    new_task.W_REG              = 0x000;
    new_task.STATUS_REG         = 0x000;
    
    // Adiciona tarefa na fila de prontas
    r_queue.ready_queue[r_queue.ready_queue_size] = new_task;
    r_queue.ready_queue_size += 1;
}

void remove_task(f_ptr task)
{
    // Proteção: não remove tarefa inválida ou Idle
    if(task == NULL || task == idle) 
    {
        return;
    }
    
    di();  // Seção crítica
    
    // Procura tarefa na fila e remove
    for(uint8_t i = 0; i < r_queue.ready_queue_size; i++) 
    {
        if(r_queue.ready_queue[i].task_f == task) 
        {
            // Compacta a fila movendo tarefas posteriores
            for(uint8_t j = i; j < r_queue.ready_queue_size - 1; j++) 
            {
                r_queue.ready_queue[j] = r_queue.ready_queue[j+1];
            }
            r_queue.ready_queue_size--;
            break;
        }
    }
    
    RESTORE_CONTEXT();  // Força troca de contexto imediatamente
    
    ei();
}

void delay(uint16_t time)
{
    di();
    
    // Bloqueia tarefa atual por 'time' ticks
    SAVE_CONTEXT(WAITING);
    r_queue.ready_queue[r_queue.task_running].time_sleeping = time;
    scheduler();      // Chama próxima tarefa
    RESTORE_CONTEXT();
    
    ei();
}

void yield()
{
    di();
    
    // Cede processador voluntariamente
    SAVE_CONTEXT(READY);
    scheduler();
    RESTORE_CONTEXT();
    
    ei();
}

void os_init()
{
    // Inicializa estruturas do kernel
    r_queue.ready_queue_size    = 0;
    r_queue.task_running        = 0;
    
    // Configuração de hardware específica do usuário
    user_config();
    
    // Cria tarefa Idle (sempre presente)
    create_task(0, 0, idle);
    asm("global _idle");  // Torna função visível para o linker
    
    // Configura timer para preempção
    config_timer0();
    
    #if DYNAMIC_MEM == ON
    SRAMInitHeap();  // Inicializa heap para alocação dinâmica
    #endif
}

void os_start()
{
    #if DEFAULT_SCHEDULER == PRIORITY_SCHEDULER
    // TODO: Implementar ordenação da fila por prioridade se necessário
    #endif

    // Habilita interrupções globais
    ei();

    // Garante configuração do hardware
    user_config();   
    
    // Inicia timer - sistema operacional em funcionamento
    start_timer0();
}

void change_state(state_t new_state)
{
    di();
    
    // Força mudança de estado da tarefa atual
    SAVE_CONTEXT(new_state);
    scheduler();
    RESTORE_CONTEXT();
    
    ei();    
}

// Tarefa Idle - executa quando nenhuma outra está pronta
TASK idle()
{
    #if IDLE_DEBUG == ON
    TRISDbits.RD7 = 0;  // LED de debug da tarefa Idle
    #endif    
    
    while (1) {
        #if IDLE_DEBUG == ON
        LATDbits.LATD7 = ~PORTDbits.RD7;  // Pisca LED para debug
        #endif
        Nop();  // Operação vazia para economizar energia
    }
}

void decrease_time(void)
{
    // Decrementa contador de delay de todas as tarefas (exceto Idle)
    for (uint8_t i = 1; i < r_queue.ready_queue_size; i++) {
        if (r_queue.ready_queue[i].time_sleeping > 0) {
            r_queue.ready_queue[i].time_sleeping--;
            // Se acabou o delay, tarefa volta para READY
            if (r_queue.ready_queue[i].time_sleeping == 0) {
                r_queue.ready_queue[i].task_state = READY;
            }
        }
    }
}


