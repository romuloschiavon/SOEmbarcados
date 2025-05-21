#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "pipe.h"
#include "config.h"
#include "io.h"

TASK tarefa_acelerador() // Prioridade 1 (configurada em main.c)
{
    unsigned int adc_value;
    // Inicializa o ADC para o canal AN0 (onde o potenciômetro está conectado)
    // A configuração básica do ADCON1 já foi feita em user_config.
    // Aqui chamamos adc_init para selecionar o canal e configurar ADCON2.
    adc_init(0); // Canal AN0

    while (1) {
        adc_value = adc_read(); // Lê o valor do potenciômetro (0-1023)

        // Aciona o LED RD0 se o valor do potenciômetro for maior que 50% (aprox. 512)
        if (adc_value > 512) {
            LATDbits.LATD0 = 1; // Liga LED
        } else {
            LATDbits.LATD0 = 0; // Desliga LED
        }
        
        // TODO: Enviar adc_value via pipe para tarefa_controle_central
        
        delay(100); // Pequeno delay para não sobrecarregar e permitir visualização
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
        LATDbits.LATD2 = ~LATDbits.LATD2; // Toggle LED RD3
        delay(1000); // Delay para visualização
   }
}

TASK tarefa_controle_estabilidade() // Prioridade 2
{
    while (1) {
        LATDbits.LATD3 = ~LATDbits.LATD3; // Toggle LED RD2
        delay(1000); // Delay para visualização
   }
}

void user_config()
{
    TRISDbits.TRISD0 = 0; // RD0 como saída (LED tarefa_acelerador)
    TRISDbits.TRISD1 = 0; // RD1 como saída (LED tarefa_controle_central)
    TRISDbits.TRISD2 = 0; // RD2 como saída (LED tarefa_controle_estabilidade)
    TRISDbits.TRISD3 = 0; // RD3 como saída (LED tarefa_injecao_eletronica)

    // Configuração dos pinos conforme item 2 do README
    // Acelerador (Potenciômetro)
    TRISAbits.TRISA0 = 1; // RA0/AN0 como entrada

    // A configuração detalhada do ADCON0, ADCON1, ADCON2 será feita em adc_init() em io.c
    // ADCON1 = 0b00001110; // Removido daqui

    // Bicos Injetores (PWM)
    TRISCbits.TRISC2 = 0; // RC2/CCP1 como saída para PWM1
    TRISCbits.TRISC1 = 0; // RC1/CCP2 como saída para PWM2

    // Freio (Interrupção Externa)
    TRISBbits.TRISB0 = 1; // RB0/INT0 como entrada
    
    // Configurações adicionais de interrupção (INTCON, INTCON2) seriam feitas
    // na API de inicialização da interrupção externa.

    // Define as tarefas como funções globais para
    // evitar que o compilador as retire na fase
    // de geração de otimização.
    asm("global _tarefa_acelerador, _tarefa_controle_central, _tarefa_injecao_eletronica, _tarefa_controle_estabilidade");
}