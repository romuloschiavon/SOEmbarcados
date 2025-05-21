#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"

#if APP_1 == ON

TASK tarefa_1()
{
    while (1) {
        LATDbits.LD0 = ~PORTDbits.RD0;
    }    
}

TASK tarefa_2()
{
    while (1) {
        LATDbits.LD1 = ~PORTDbits.RD1;
    }    
}

TASK tarefa_3()
{
    while (1) {
        LATDbits.LD2 = ~PORTDbits.RD2;
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
    asm("global _tarefa_1, _tarefa_2, _tarefa_3");
}

#elif APP_2 == ON

sem_t s;

TASK tarefa_1()
{
    while (1) {
        LATDbits.LATD7 = ~PORTDbits.RD7;
        //sem_wait(&s);
        LATDbits.LD0 = ~PORTDbits.RD0;
        //change_state(WAITING);
        //delay(50);
        
    }    
}

TASK tarefa_2()
{
    while (1) {
        //sem_wait(&s);
        LATDbits.LD1 = ~PORTDbits.RD1;
        //change_state(WAITING);
    }    
}

TASK tarefa_3()
{
    while (1) {
        //sem_wait(&s);
        LATDbits.LD2 = ~PORTDbits.RD2;
        //change_state(WAITING);
    }
}

void user_config()
{
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
   
    // Inicializar o semáforo
    sem_init(&s, 0);
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_1, _tarefa_2, _tarefa_3");
}

#elif APP_3 == ON

pipe_t pipe;

TASK tarefa_1()
{
    while (1) {
        LATDbits.LD0 = ~PORTDbits.RD0;
        //LATDbits.LD0 = ^=1;
    }    
}

TASK tarefa_2()
{
    uint8_t dados[3] = {'L', 'D', 'D'};
    int i = 0;
    
    while (1) {
        write_pipe(&pipe, dados[i]);
        i = (i+1) % 3;
        LATDbits.LD1 = ~PORTDbits.RD1;        
    }    
}

TASK tarefa_3()
{
    uint8_t dado_pipe = 0;
    while (1) {
        read_pipe(&pipe, &dado_pipe);
        if (dado_pipe == 'L') {
            LATDbits.LD2 = 1;
        }
        else if (dado_pipe == 'D') {
            LATDbits.LD2 = 0;
        }
    }
}

void user_config()
{
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
   
    create_pipe(&pipe);
    
    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_1, _tarefa_2, _tarefa_3");
}

#endif
