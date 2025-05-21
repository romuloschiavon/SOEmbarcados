#include "kernel.h"
#include "user_app.h"


int main()
{
    os_init();
    
    // Cria tarefas de usuario com prioridades para teste
    // Prioridade 0 (idle)
    // Prioridade 1 para tarefa_acelerador (RD0)
    // Prioridade 2 para tarefa_controle_central (RD1)
    // Prioridade 3 para tarefa_controle_estabilidade (RD2)
    // Prioridade 4 para tarefa_injecao_eletronica (RD3)
    create_task(1, 1, tarefa_acelerador); 
    create_task(2, 2, tarefa_controle_central);
    create_task(3, 3, tarefa_controle_estabilidade); 
    create_task(4, 4, tarefa_injecao_eletronica);
    
    os_start();
    
    while (1);
    
    return 0;
}
