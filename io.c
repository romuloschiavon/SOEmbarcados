#include "io.h"
#include "kernel.h"
#include "config.h"

#ifndef FOSC
    #define FOSC 8000000UL
#endif

// Variáveis para PWM via Timer1 (terceiro bico - RC0)
static unsigned int timer1_pwm_duty = 0;
static unsigned int timer1_pwm_period = 1023;
static unsigned int timer1_pwm_counter = 0;


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
    }    if (pr2_value_long > 255) {
        PR2 = 255; // Satura em 255 se a frequência for muito baixa para o Fosc
    } else if (pr2_value_long < 0) {
        PR2 = 0; // Evita valor negativo, frequência muito alta
    } else {
        PR2 = (unsigned char)pr2_value_long;
    }
      // 2. Configurar o módulo CCP para modo PWM    
    if (module == PWM_MODULE_CCP1) {
        CCP1CONbits.CCP1M = 0b1100; // Configura CCP1 para modo PWM 
        TRISCbits.TRISC2 = 0;       // RC2/CCP1 como saída
    }
    else if (module == PWM_MODULE_CCP2) {
        CCP2CONbits.CCP2M = 0b1100; // Configura CCP2 para modo PWM
        TRISCbits.TRISC1 = 0;       // RC1/CCP2 como saída
    }
    else if (module == PWM_MODULE_TIMER1) {
        // Configura Timer1 para PWM no RC0
        TRISCbits.TRISC0 = 0;       // RC0 como saída
        // Timer1 será usado para gerar PWM via interrupção
    }

    TMR2 = 0;
    PIR1bits.TMR2IF = 0;
}

void pwm_set_duty_cycle(pwm_module_t module, unsigned int duty_cycle_value)
{
    if (duty_cycle_value > 1023) duty_cycle_value = 1023; // Satura em 10 bits
    
    if (module == PWM_MODULE_CCP1) 
    {
        CCPR1L = (unsigned char)(duty_cycle_value >> 2);      // 8 MSBs
        CCP1CON = (CCP1CON & 0xCF) | (unsigned char)((duty_cycle_value & 0x03) << 4); // 2 LSBs nos bits 5 e 4
    }
    else if (module == PWM_MODULE_CCP2) {
        CCPR2L = (unsigned char)(duty_cycle_value >> 2);      // 8 MSBs
        CCP2CON = (CCP2CON & 0xCF) | (unsigned char)((duty_cycle_value & 0x03) << 4); // 2 LSBs nos bits 5 e 4
    }
    else if (module == PWM_MODULE_TIMER1) {
        // Atualiza duty cycle para PWM via Timer1
        timer1_pwm_duty = duty_cycle_value;
    }
}

void pwm_stop(pwm_module_t module)
{
    // Para parar o PWM, desliga o Timer2
    T2CONbits.TMR2ON = 0;
}

// Função para atualizar PWM via Timer1 (terceiro bico)
void timer1_pwm_update(void) {
    if (timer1_pwm_counter < timer1_pwm_duty) {
        LATCbits.LATC0 = 1; // Liga terceiro bico (RC0)
    } else {
        LATCbits.LATC0 = 0; // Desliga terceiro bico (RC0)
    }
    
    timer1_pwm_counter++;
    if (timer1_pwm_counter >= timer1_pwm_period) {
        timer1_pwm_counter = 0;
    }
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
    
    // Pequeno delay para estabilização (usando loop simples)
    for(int i = 0; i < 1000; i++) {
        Nop();
    }
}

unsigned int adc_read(void) {
    ADCON0bits.GO_DONE = 1; 
    while (ADCON0bits.GO_DONE); 
    return (((unsigned int)ADRESH << 8) | ADRESL);
}

// --- Implementação da API de Interrupção Externa ---
static interrupt_callback_t ext_int_callback = NULL;

void ext_interrupt_init(unsigned char int_num, unsigned char edge_type, interrupt_callback_t callback_function) {
    ext_int_callback = callback_function;
    
    if (int_num == 0) { // INT0
        // Configurar edge_type: 1 = rising edge, 0 = falling edge
        INTCON2bits.INTEDG0 = edge_type;
        
        // Limpar flag e habilitar interrupção
        INTCONbits.INT0IF = 0;
        INTCONbits.INT0IE = 1;
        
        // Habilitar interrupções globais (se não estiverem habilitadas)
        INTCONbits.GIE = 1;
        INTCONbits.PEIE = 1;
    }
}

// ISR para interrupção externa (deve ser chamada da ISR principal)
void ext_interrupt_handler(void) {
    if (INTCONbits.INT0IF) {
        INTCONbits.INT0IF = 0; // Limpa flag
        if (ext_int_callback != NULL) {
            ext_int_callback(); // Chama callback
        }
    }
}
