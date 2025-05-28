#include "io.h"
#include <xc.h>

// --- CONFIGURAÇÃO PWM PARA CONTROLE DO BICO INJETOR ---
void pwm_init(void)
{
    // Configura pino RC2 (CCP1) como saída para sinal PWM
    TRISCbits.TRISC2 = 0;
    
    // Configura módulo CCP1 em modo PWM padrão
    CCP1CON = 0b00001100;
    
    // Configuração do Timer2 como base de tempo para PWM
    T2CON = 0x04;         // Prescaler 1:1, Timer2 ligado
    PR2 = 0xFF;           // Período máximo = 255 (~1kHz @ 4MHz)
    
    // Inicializa com duty cycle = 0% (bico fechado)
    CCPR1L = 0;
    CCP1CONbits.DC1B = 0;
    
    TMR2 = 0;             // Reseta contador do Timer2
}

void pwm_set_duty(uint8_t duty)
{
    if (duty > 255) duty = 255;  // Limita valor máximo
    
    // Define duty cycle (quanto tempo o bico fica aberto)
    CCPR1L = duty;                    // 8 bits mais significativos
    CCP1CONbits.DC1B = 0;             // 2 bits menos significativos = 0
}

// --- CONFIGURAÇÃO ADC PARA LEITURA DO ACELERADOR ---
void adc_init(void)
{
    ADCON0 = 0b00000001;     // Canal AN0 selecionado, ADC ligado
    ADCON1 = 0b00001110;     // Apenas AN0 como analógico, resto digital
    ADCON2 = 0b10101010;     // Resultado justificado à direita, Tac=Fosc/32
    
    // Configura RA0 como entrada analógica para o potenciômetro
    TRISAbits.TRISA0 = 1;
}

uint16_t adc_read(void)
{
    ADCON0bits.GO = 1;                 // Inicia conversão ADC
    while (ADCON0bits.GO);             // Aguarda fim da conversão
    return (uint16_t)((ADRESH << 8) | ADRESL);  // Retorna resultado de 10 bits
}

// --- CONFIGURAÇÃO DE INTERRUPÇÃO EXTERNA PARA FREIO ---
void ext_interrupt_init(uint8_t int_num, uint8_t edge_type)
{
    if (int_num == 0){
        TRISBbits.TRISB0 = 1;           // RB0 como entrada (botão do freio)
        INTCON2bits.INTEDG0 = edge_type; // Define tipo de borda (0=descida, 1=subida)
        INTCONbits.INT0IE = 1;          // Habilita interrupção INT0
        INTCONbits.INT0IF = 0;          // Limpa flag pendente
    }
    else{
        // Configuração geral de interrupções para outros periféricos
        INTCONbits.PEIE = 1;
        INTCONbits.GIE = 1;
    }
}