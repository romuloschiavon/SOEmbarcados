#include "io.h"
#include <xc.h>

// --- PWM CONFIGURAÃÃO ---
void pwm_init(void)
{
    // ConfiguraÃ§Ã£o do pino RC2 como saÃ­da para PWM
    TRISCbits.TRISC2 = 0; // CCP1 - RC2 (saÃ­da PWM)
    
    // Configura CCP1 para modo PWM padrÃ£o (nÃ£o Enhanced)
    CCP1CON = 0b00001100; // Modo PWM padrÃ£o
    
    // Timer2 configuraÃ§Ã£o para frequÃªncia PWM
    T2CON = 0x04;         // Prescaler 1:1, Timer2 ON
    PR2 = 0xFF;           // PerÃ­odo = 255 â ~1kHz @ 4MHz
    
    // Duty cycle inicial = 0% (motor parado)
    CCPR1L = 0;
    CCP1CONbits.DC1B = 0;
    
    TMR2 = 0;            // Reset Timer2
}

void pwm_set_duty(uint8_t duty)
{
    if (duty > 255) duty = 255;
    
    // ConfiguraÃ§Ã£o do duty cycle (8-bit)
    CCPR1L = duty;                    // 8 bits MSB
    CCP1CONbits.DC1B = 0;             // 2 bits LSB = 0
}

// --- ADC CONFIGURAÃÃO ---
void adc_init(void)
{
    ADCON0 = 0b00000001;     // AN0, ADC on
    ADCON1 = 0b00001110;     // AN0 analÃ³gico, restante digital
    ADCON2 = 0b10101010;     // Justificado Ã  direita, Tac=Fosc/32
    
    // Configura RA0 como entrada para ADC
    TRISAbits.TRISA0 = 1;
}

uint16_t adc_read(void)
{
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    return (uint16_t)((ADRESH << 8) | ADRESL);
}

void ext_interrupt_init(uint8_t int_num, uint8_t edge_type)
{
    if (int_num == 0){
        TRISBbits.TRISB0 = 1;       // Configura RB0 como entrada
        INTCON2bits.INTEDG0 = edge_type; // Borda de interrupÃ§Ã£o
        INTCONbits.INT0IE = 1;      // Habilita interrupÃ§Ã£o INT0
        INTCONbits.INT0IF = 0;      // Limpa flag de interrupÃ§Ã£o
    }
    else{
        INTCONbits.PEIE = 1;
        INTCONbits.GIE = 1;
    }
}