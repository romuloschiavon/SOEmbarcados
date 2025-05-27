#include "io.h"
#include <xc.h>

// --- PWM CONFIGURAÇÃO ---
void pwm_init(void)
{
    // Configura os pinos como saída digital
    TRISCbits.TRISC2 = 0; // P1A - RC2
    TRISDbits.TRISD5 = 0; // P1B - RD5
    TRISDbits.TRISD6 = 0; // P1C - RD6

    CCP1CON = 0b00001100; // PWM mode, P1A ativo
    T2CON = 0b00000101;   // Timer2 ON, Preescale 1:4
    PR2 = 255;

    CCPR1L = 0;
    CCP1CONbits.DC1B = 0;
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
}

void pwm_set_duty(uint8_t duty)
{
    if (duty > 255) duty = 255;

    CCPR1L = duty;                  // 8 bits mais significativos do duty
    CCP1CONbits.DC1B = duty & 0x03; // 2 bits menos significativos
}

// --- ADC CONFIGURAÇÃO ---
void adc_init(void)
{
    ADCON0 = 0b00000001;     // AN0, ADC on
    ADCON1 = 0b00001110;     // AN0 analógico, restante digital
    ADCON2 = 0b10101010;     // Justificado à direita, Tac=Fosc/32
}

uint16_t adc_read(void)
{
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    return ((ADRESH << 8) | ADRESL);
}

static void (*ext_int_callback)(void) = 0;

void ext_interrupt_init(uint8_t int_num, uint8_t edge_type, void (*callback)(void))
{
    if (int_num == 0) {
        INTCONbits.INT0IE = 1;       // Habilita INT0
        INTCONbits.INT0IF = 0;       // Limpa flag
        INTCON2bits.INTEDG0 = edge_type ? 1 : 0; // 1 = borda de subida, 0 = descida
        ext_int_callback = callback;
    }
}

void ext_interrupt_handler(void)
{
    if (INTCONbits.INT0IF) {
        INTCONbits.INT0IF = 0;
        if (ext_int_callback) {
            ext_int_callback();
        }
    }
}
