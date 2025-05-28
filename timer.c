#include <xc.h>
#include "timer.h"
#include "kernel.h"
#include "scheduler.h"
#include "config.h"
#include "io.h"
#include "user_app.h"

void config_timer0()
{
    // Habilita interrupções de periféricos
    INTCONbits.PEIE     = 1;
    // Habilita interrupção do timer 0
    INTCONbits.TMR0IE   = 1;
    // Limpa flag de interrupção
    INTCONbits.TMR0IF   = 0;
    // Timer usa clock interno
    T0CONbits.T0CS      = 0;
    // Ativa prescaler para o timer
    T0CONbits.PSA       = 0;
    // Prescaler 1:64 para gerar tick de aproximadamente 1ms
    T0CONbits.T0PS      = 0b101;
    // Valor inicial do timer
    TMR0 = 0;
}

void start_timer0()
{
    T0CONbits.TMR0ON = 1;
}

// Tratador único de interrupções do sistema
void __interrupt() ISR_TMR0()
{   
    // Checa interrupções externas (freio)
    ISR_FREIO();
      
    // Verifica e trata interrupção do timer    
    if (INTCONbits.TMR0IF) {
        // Limpa flag de interrupção        
        INTCONbits.TMR0IF = 0;
        // Reinicia contador
        TMR0 = 0;
        
        // Decrementa contador de delay das tarefas
        decrease_time();
        
        // Salva contexto da tarefa atual
        SAVE_CONTEXT(READY);

        // Chama escalonador para próxima tarefa
        scheduler();
        
        // Restaura contexto da nova tarefa
        RESTORE_CONTEXT();
    }
}