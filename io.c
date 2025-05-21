#include "io.h"
#include "kernel.h"

#ifndef FOSC
    #define FOSC 8000000UL
#endif


void pwm_init(pwm_module_t module, unsigned int frequency)
{

    unsigned char tmr2_prescaler = 1; // Valores podem ser 1, 4, 16
    unsigned long pr2_value_long;

    // Tenta com prescaler 1:1
    T2CONbits.T2CKPS = 0b00; // Prescaler 1:1
    pr2_value_long = (FOSC / (4UL * frequency * 1UL)) - 1UL;

    if (pr2_value_long > 255) { // Se PR2 estourar, aumenta prescaler
        T2CONbits.T2CKPS = 0b01; // Prescaler 1:4
        tmr2_prescaler = 4;
        pr2_value_long = (FOSC / (4UL * frequency * 4UL)) - 1UL;
    }
    if (pr2_value_long > 255) { // Se PR2 ainda estourar, aumenta prescaler novamente
        T2CONbits.T2CKPS = 0b10; // Prescaler 1:16 (ou 0b11 que também é 1:16)
        tmr2_prescaler = 16;
        pr2_value_long = (FOSC / (4UL * frequency * 16UL)) - 1UL;
    }

    if (pr2_value_long > 255) {
        PR2 = 255; // Satura em 255 se a frequência for muito baixa para o Fosc
    } else if (pr2_value_long < 0) {
        PR2 = 0; // Evita valor negativo, frequência muito alta
    } else {
        PR2 = (unsigned char)pr2_value_long;
    }

    // 2. Configurar o módulo CCP para modo PWM
    if (module == PWM_MODULE_CCP1) {
        CCP1CONbits.CCP1M = 0b1100; // Configura CCP1 para modo PWM (P1A, P1B, P1C, P1D ativos alto ou baixo)
                                    // Para operação single output PWM, os bits P1M<1:0> são irrelevantes
                                    // CCP1M<3:2> = 11 para modo PWM.
        TRISCbits.TRISC2 = 0;       // Garante que RC2/CCP1 é saída (já feito em user_config, mas bom reforçar)
    }
    else if (module == PWM_MODULE_CCP2) {
        CCP2CONbits.CCP2M = 0b1100; // Configura CCP2 para modo PWM
        TRISCbits.TRISC1 = 0;       // Garante que RC1/CCP2 é saída
    }

    TMR2 = 0;
    PIR1bits.TMR2IF = 0;
}

void pwm_set_duty_cycle(pwm_module_t module, unsigned int duty_cycle_value)
{
    if (duty_cycle_value > 1023) duty_cycle_value = 1023; // Satura em 10 bits

    if (module == PWM_MODULE_CCP1) {
        CCPR1L = (unsigned char)(duty_cycle_value >> 2);      // 8 MSBs
        CCP1CON = (CCP1CON & 0xCF) | ((unsigned char)(duty_cycle_value & 0x03) << 4); // 2 LSBs nos bits 5 e 4
    }
    else if (module == PWM_MODULE_CCP2) {
        CCPR2L = (unsigned char)(duty_cycle_value >> 2);      // 8 MSBs
        CCP2CON = (CCP2CON & 0xCF) | ((unsigned char)(duty_cycle_value & 0x03) << 4); // 2 LSBs nos bits 5 e 4
    }
}

void pwm_start(pwm_module_t module)
{
    // Para iniciar o PWM, basta ligar o Timer2
    T2CONbits.TMR2ON = 1;
    // O módulo CCP já foi configurado para PWM no init.
    // A saída PWM correspondente (RC1 ou RC2) já deve estar como saída.
}

void pwm_stop(pwm_module_t module)
{
    // Para parar o PWM, desliga o Timer2
    T2CONbits.TMR2ON = 0;

    // Opcionalmente, pode-se reconfigurar o modo do CCP se necessário
    // if (module == PWM_MODULE_CCP1) {
    //     CCP1CONbits.CCP1M = 0; // Desliga CCP1
    // }
    // else if (module == PWM_MODULE_CCP2) {
    //     CCP2CONbits.CCP2M = 0; // Desliga CCP2
    // }
}

// --- Implementação da API ADC (placeholders) ---
void adc_init(unsigned char channel) {
    // Configuração baseada no exemplo do professor
    
    // 1. Configurar canal ANx (ADCON0)
    ADCON0bits.CHS = channel; // Ex: 0b0000 para AN0
    
    // 2. Configurar tensões de referência e pinos analógicos/digitais (ADCON1)
    ADCON1bits.VCFG1 = 0; // VREF- = VSS
    ADCON1bits.VCFG0 = 0; // VREF+ = VDD
    // PCFG<3:0>: Para AN0 como analógico e os demais digitais = 0b1110
    ADCON1bits.PCFG = 0b1110; 
    
    // 3. Configurar tempo de aquisição, clock do ADC e justificação (ADCON2)
    ADCON2bits.ACQT = 0b101; // 12 TAD (Tempo de Aquisição)
    ADCON2bits.ADCS = 0b101; // Fosc/16 (Clock de Conversão do ADC)
    ADCON2bits.ADFM = 1;     // Resultado justificado à direita (ADRESH contém os 2 MSb, ADRESL os 8 LSb)
    
    // 4. Ativar o módulo ADC (ADCON0)
    ADCON0bits.ADON = 1;
    
    delay(20); // Requer _XTAL_FREQ definido e <xc.h>
}

unsigned int adc_read(void) {
    ADCON0bits.GO_DONE = 1; 
    while (ADCON0bits.GO_DONE); 
    return (((unsigned int)ADRESH << 8) | ADRESL);
}

// --- Implementação da API de Interrupção Externa (placeholders) ---
// void ext_interrupt_init(unsigned char int_num, unsigned char edge_type, interrupt_callback_t callback_function) {}
