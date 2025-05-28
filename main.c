#include "kernel.h"
#include "user_app.h"

int main()
{
    os_init();
    
    // ✅ TODAS as tarefas com prioridades corretas
    create_task(1, 2, tarefa_acelerador);           // Prioridade 2
    create_task(2, 3, tarefa_controle_central);     // Prioridade 3  
    create_task(3, 4, tarefa_injecao_eletronica);   // Prioridade 4
    create_task(4, 1, tarefa_controle_estabilidade); // Prioridade 1 (MAIOR)

    os_start();
    
    while (1);
    
    return 0;
}
