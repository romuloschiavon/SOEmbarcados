#include "kernel.h"
#include "user_app.h"


int main()
{
    os_init();
    
    // Cria tarefas de usuário
    create_task(1, 3, tarefa_1);
    create_task(2, 3, tarefa_2);
    create_task(3, 3, tarefa_3);
    
    os_start();
    
    while (1);
    
    return 0;
}
