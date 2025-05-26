#include <xc.h>
#include "timer.h"
#include "kernel.h"
#include "scheduler.h"
#include "config.h"
#include "io.h"

void config_timer0()
{
    // Habilita interrupções de perifericos
    INTCONbits.PEIE     = 1;
    // Habilita interrupção do timer 0
    INTCONbits.TMR0IE   = 1;
    // Seta o flag do timer em zero
    INTCONbits.TMR0IF   = 0;
    // Transição do timer por referencia interna
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

// Tratador de interrupcao do timer
void __interrupt() ISR_TMR0()
{
    di();
    
    // Verifica e trata interrupção externa (freio)
    ext_interrupt_handler();
      // Verifica e trata interrupção do timer    
    if (INTCONbits.TMR0IF) {
        // Seta o flag do timer em zero
        INTCONbits.TMR0IF   = 0;
        // Valor inicial do timer
        TMR0 = 0;
        
        // Atualiza PWM via Timer1 para o terceiro bico (RC0)
        timer1_pwm_update();
                
        // Decrementa o delay das tarefas que estao em estado
        decrease_time();
        
        // Salva o contexto da tarefa que esta em execucao
        SAVE_CONTEXT(READY);

        // Chama o escalonador para definir qual a proxima tarefa sera executada
        scheduler();
        // Restaura o contexto da tarefa que entrara em execucao
        RESTORE_CONTEXT();
    }
    
    ei();
}