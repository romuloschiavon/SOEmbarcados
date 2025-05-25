#include "kernel.h"
#include "user_app.h"


int main()
{
    os_init();
    
    // Cria tarefas de usuario com prioridades para teste e aplicação:
    create_task(1, 1, tarefa_acelerador); 
    create_task(2, 2, tarefa_controle_central);
    create_task(3, 3, tarefa_injecao_eletronica);

    // REMOVIDO: create_task(4, 4, tarefa_controle_estabilidade); 
    // A tarefa de controle de estabilidade será ativada apenas via interrupção de freio
    
    os_start();
    
    while (1);
    
    return 0;
}
