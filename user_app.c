#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"

TASK tarefa_acelerador()
{
    while (1) {
        
    }    
}

TASK tarefa_controle_central()
{
    while (1) {
        
    }    
}

TASK tarefa_injecao_eletronica()
{
    while (1) {
        
   }
}

TASK tarefa_controle_estabilidade()
{
    while (1) {
        
   }
}

void user_config()
{
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}