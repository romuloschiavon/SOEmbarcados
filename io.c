#include "io.h"
#include <xc.h>

// --- PWM CONFIGURAÇÃO ---
void pwm_init(void)
{
    // Configura os pinos como saída digital
    TRISCbits.TRISC2 = 0; // P1A - RC2
    TRISDbits.TRISD5 = 0; // P1B - RD5
    TRISDbits.TRISD6 = 0; // P1C - RD6

    // ✅ ENHANCED PWM MODE CORRETO com todas as saídas
    CCP1CON = 0b11001100; // P1M=11 (Enhanced PWM), CCP1M=1100 (PWM mode)
    
    // Timer2 configuração
    T2CON = 0b00000100;   // ✅ Timer2 ON, Prescaler 1:1 (mais responsivo)
    PR2 = 255;            // Período PWM máximo
    
    // Duty cycle inicial zero
    CCPR1L = 0;
    CCP1CONbits.DC1B = 0;
    
    // Inicia Timer2
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
}

void pwm_set_duty(uint8_t duty)
{
    if (duty > 255) duty = 255;

    // ✅ CONFIGURAÇÃO CORRETA para 10-bit PWM
    CCPR1L = duty;                    // 8 bits MSB
    CCP1CONbits.DC1B = 0;             // 2 bits LSB = 0 (resolução 8-bit)
}

// --- ADC CONFIGURAÇÃO ---
void adc_init(void)
{
    ADCON0 = 0b00000001;     // AN0, ADC on
    ADCON1 = 0b00001110;     // AN0 analógico, restante digital
    ADCON2 = 0b10101010;     // Justificado à direita, Tac=Fosc/32
    
    // Configura RA0 como entrada para ADC
    TRISAbits.TRISA0 = 1;
}

uint16_t adc_read(void)
{
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    return (uint16_t)((ADRESH << 8) | ADRESL);
}

static void (*ext_int_callback)(void) = 0;

void ext_interrupt_init(uint8_t int_num, uint8_t edge_type, void (*callback)(void))
{
    if (int_num == 0) {
        // Configura RB0 como entrada
        TRISBbits.TRISB0 = 1;
        
        // Habilita pull-up interno (IMPORTANTE!)
        INTCON2bits.NOT_RBPU = 0;  // Habilita pull-ups do PORTB
        
        INTCONbits.INT0IE = 1;       // Habilita INT0
        INTCONbits.INT0IF = 0;       // Limpa flag
        INTCON2bits.INTEDG0 = edge_type ? 1 : 0; // 1 = borda de subida, 0 = descida
        ext_int_callback = callback;
        
        // Habilita interrupções globais
        INTCONbits.GIE = 1;
    }
}

void ext_interrupt_handler(void)
{
    if (INTCONbits.INT0IF) {
        INTCONbits.INT0IF = 0;       // Limpa flag PRIMEIRO
        if (ext_int_callback) {
            ext_int_callback();       // Chama callback
        }
    }
}

// Removido __interrupt() - agora é uma função normal
void ISR_PRINCIPAL(void)
{
    // Verifica interrupção externa INT0
    if (INTCONbits.INT0IF && INTCONbits.INT0IE) {
        ext_interrupt_handler();
    }
}