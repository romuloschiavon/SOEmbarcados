#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"

TASK tarefa_acelerador() // Prioridade 0 (mais baixa entre as de usuário)
{
    while (1) {
        LATDbits.LATD0 = ~LATDbits.LATD0; // Toggle LED RD0
        delay(1000); // Delay para visualização
    }    
}

TASK tarefa_controle_central() // Prioridade 1
{
    while (1) {
        LATDbits.LATD1 = ~LATDbits.LATD1; // Toggle LED RD1
        delay(1000); // Delay para visualização
    }    
}

TASK tarefa_injecao_eletronica() // Prioridade 3 (mais alta)
{
    while (1) {
        LATDbits.LATD3 = ~LATDbits.LATD3; // Toggle LED RD3
        delay(1000); // Delay para visualização
   }
}

TASK tarefa_controle_estabilidade() // Prioridade 2
{
    while (1) {
        LATDbits.LATD2 = ~LATDbits.LATD2; // Toggle LED RD2
        delay(1000); // Delay para visualização
   }
}

void user_config()
{
    TRISDbits.TRISD0 = 0; // RD0 como saída (LED)
    TRISDbits.TRISD1 = 0; // RD1 como saída (LED)
    TRISDbits.TRISD2 = 0; // RD2 como saída (LED)
    TRISDbits.TRISD3 = 0; // RD3 como saída (LED)
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}