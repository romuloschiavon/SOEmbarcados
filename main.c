#include "kernel.h"
#include "user_app.h"

int main()
{
    os_init();
    
    // Criação das tarefas em ordem de prioridade
    create_task(1, 1, tarefa_acelerador);           // Prioridade 1 (baixa)
    create_task(2, 2, tarefa_controle_central);     // Prioridade 2 (média)
    create_task(3, 3, tarefa_injecao_eletronica);   // Prioridade 3 (alta)
    // A tarefa de estabilidade é criada dinamicamente com prioridade 4 (máxima)

    os_start();
    
    while (1);
    
    return 0;
}
