#include <xc.h>
#include "timer.h"
#include "kernel.h"
#include "scheduler.h"
#include "config.h"
#include "io.h"
#include "user_app.h"

void config_timer0()
{
    // Habilita interrupÃ§Ãµes de perifericos
    INTCONbits.PEIE     = 1;
    // Habilita interrupÃ§Ã£o do timer 0
    INTCONbits.TMR0IE   = 1;
    // Seta o flag do timer em zero
    INTCONbits.TMR0IF   = 0;
    // TransiÃ§Ã£o do timer por referencia interna
    T0CONbits.T0CS      = 0;
    // Ativa preescaler para o timer zero
    T0CONbits.PSA       = 0;
    // Preesclaer 1:64
    T0CONbits.T0PS      = 0b101;
    // Valor inicial do timer
    TMR0 = 0;
}

void start_timer0()
{
    T0CONbits.TMR0ON = 1;
}

// Tratador Ãºnico de interrupÃ§Ãµes
void __interrupt() ISR_TMR0()
{   
    ISR_FREIO();
      
    // Verifica e trata interrupÃ§Ã£o do timer    
    if (INTCONbits.TMR0IF) {
        // Seta o flag do timer em zero        
        INTCONbits.TMR0IF = 0;
        // Valor inicial do timer
        TMR0 = 0;
        
        // Decrementa o delay das tarefas que estao em estado
        decrease_time();
        
        // Salva o contexto da tarefa que esta em execucao
        SAVE_CONTEXT(READY);

        // Chama o escalonador para definir qual a proxima tarefa sera executada
        scheduler();
        // Restaura o contexto da tarefa que entrara em execucao
        RESTORE_CONTEXT();
    }
}