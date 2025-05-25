# SOEmbarcados - Sistema de Controle Veicular Simulado

Este projeto implementa um sistema operacional de tempo real (RTOS) simplificado para um microcontrolador PIC18F24550, simulando um sistema de controle veicular.

## Funcionalidades do RTOS (Implementadas e a Implementar)

*   Kernel: Gerenciamento de tarefas, troca de contexto, delays. (Implementado)
*   Escalonador:
    *   Round Robin (Implementado)
    *   Escalonamento por Prioridade (Implementado - Teste inicial abaixo)
*   Sincronização: Semáforos e Mutexes. (Implementado)
*   Comunicação: Pipes para comunicação entre tarefas. (Implementado - Alocação estática)
*   Gerenciamento de Memória: Alocador dinâmico. (Implementado - Desativado por padrão)

## Lógica da Aplicação Embarcada (A Ser Implementada)

O sistema simulará as seguintes funcionalidades de um veículo:

1.  Leitura do Acelerador: Uma tarefa lê a posição do pedal do acelerador (potenciômetro).
2.  Controle Central: Uma tarefa recebe a posição do acelerador, calcula o tempo de injeção e se comunica com a tarefa de injeção.
3.  Injeção Eletrônica: Uma tarefa controla os bicos injetores (simulados por motores DC via PWM) com base no cálculo do controle central.
4.  Controle de Estabilidade: Uma tarefa de alta prioridade, acionada por interrupção externa (simulando freio), atua nos freios (simulados por LEDs).

## TODO - Próximas Implementações

### 1. Escalonador por Prioridade (Implementação Inicial Concluída)

*   [x] Implementar a lógica de seleção de tarefas baseada em prioridade no arquivo `scheduler.c`.
    *   A tarefa com maior valor numérico de prioridade (3 > 2 > 1 > 0) tem preferência.
    *   Tarefas de usuário são priorizadas sobre a tarefa Idle.
    *   A tarefa Idle (prioridade 0) é executada se nenhuma tarefa de usuário estiver pronta.
    *   Em caso de empate de prioridade entre tarefas de usuário, a tarefa com menor índice na fila de prontos (primeira adicionada/encontrada) é escolhida.
*   [x] Testar o escalonador com tarefas de exemplo (`tarefa_acelerador` (P0), `tarefa_controle_central` (P1), `tarefa_controle_estabilidade` (P2), `tarefa_injecao_eletronica` (P3)), cada uma controlando um LED (RD0-RD3) para visualização.

### 2. Configuração de Hardware e API de E/S (`io.c`, `io.h`)

*   [x] **Configurar Pinos de Hardware:**
    *   [ ] Em `user_config()` (`user_app.c`), definir as direções dos pinos (TRISx):
        *   Pino para o potenciômetro (Acelerador) como entrada analógica (e.g., RA0/AN0).
        *   Pinos para controle PWM dos motores DC (Bicos Injetores) como saída (e.g., RC2/CCP1, RC1/CCP2). *Verificar pinos exatos do PIC18F24550 para PWM*.
        *   Pinos para LEDs (Freios - Controle de Estabilidade) como saída (e.g., RD0, RD1, RD2 já usados para teste, podem ser reutilizados ou outros).
        *   Pino para o botão de interrupção externa (Freio) como entrada (e.g., RB0/INT0).
    *   [ ] **Desenvolver API de E/S:**
    *   [ ] **ADC (Conversor Analógico-Digital):**
        *   `void adc_init(unsigned char channel)`: Configura o módulo ADC (ADCON0, ADCON1, ADCON2) para o canal especificado, justificação, tempo de aquisição.
        *   `unsigned int adc_read()`: Inicia a conversão, aguarda o fim (ADCON0bits.GO_DONE) e retorna o valor (ADRESH:ADRESL).
    *   [ ] **PWM (Modulação por Largura de Pulso - Módulo CCP):**
        *   `void pwm_init(unsigned char module_ccp, unsigned int frequency)`: Configura o módulo CCPx (e.g., CCP1CON para CCP1) para modo PWM. Configura o Timer2 (T2CON, PR2) para a frequência desejada.
        *   `void pwm_set_duty_cycle(unsigned char module_ccp, unsigned int duty_cycle_value)`: Ajusta o duty cycle (CCPRxL e bits DCxB do CCPxCON).
        *   `void pwm_start(unsigned char module_ccp)`: Habilita o Timer2 (T2CONbits.TMR2ON) e, se necessário, o pino CCPx.
        *   `void pwm_stop(unsigned char module_ccp)`: Desabilita o Timer2 ou o módulo CCPx.
    *   [ ] **Interrupção Externa:**
        *   `void ext_interrupt_init(unsigned char int_num, unsigned char edge_type, void (*callback_function)(void))`: Configura a interrupção externa (INTCON, INTCON2, INTCON3). `int_num` (0, 1 ou 2), `edge_type` (borda de subida/descida). Armazena o `callback_function`.
        *   Na ISR de alto nível, verificar o flag da interrupção (e.g., INTCONbits.INT0IF), limpá-lo e chamar o callback correspondente. Habilitar GIE/GIEH e PEIE/GIEL, e a interrupção específica (e.g., INTCONbits.INT0IE).

### 3. Implementação da Lógica das Tarefas da Aplicação (`user_app.c`)

*   [ ] **`tarefa_acelerador`:**
    *   [ ] No loop:
        *   Ler o valor do ADC (potenciômetro) usando `adc_read()`.
        *   Enviar o valor lido (possivelmente mapeado/escalonado) para `tarefa_controle_central` via `pipe` (usar `write_pipe()`).
        *   Chamar `delay()` para ceder CPU.
    *   [ ] **`tarefa_controle_central`:**
    *   [ ] No loop:
        *   Aguardar e receber dados da `tarefa_acelerador` via `pipe` (usar `read_pipe()`).
        *   Calcular o "tempo de abertura dos bicos" (e.g., um valor de 0-255 para duty cycle PWM) com base no valor do acelerador.
        *   Bloquear `mutex_injecao`.
        *   Escrever o valor calculado em uma variável global compartilhada (e.g., `g_tempo_injecao`).
        *   Liberar `mutex_injecao`.
        *   Chamar `delay()`.
    *   [ ] **`tarefa_injecao_eletronica`:**
    *   [ ] No loop:
        *   Bloquear `mutex_injecao`.
        *   Ler o valor de `g_tempo_injecao`.
        *   Liberar `mutex_injecao`.
        *   Aplicar o valor lido como duty cycle para os 3 "bicos injetores" (simulados por PWM). Usar `pwm_set_duty_cycle()` para cada módulo PWM correspondente.
        *   Chamar `delay()`.
    *   [ ] **`tarefa_controle_estabilidade`:**
    *   [ ] Esta tarefa deve ter a maior prioridade de usuário.
    *   [ ] A função de callback da interrupção externa (freio) deve:
        *   Limpar o flag da interrupção.
        *   Colocar `tarefa_controle_estabilidade` no estado `READY` (se não estiver já). Isso pode ser feito por um semáforo ou diretamente alterando o estado da TCB e notificando o scheduler se necessário (ou deixar que o próximo `yield`/`delay`/tick do timer acione o scheduler). Uma forma simples é a ISR postar em um semáforo no qual a tarefa está esperando.
    *   [ ] No loop da tarefa:
        *   Aguardar em um semáforo (que é liberado pela ISR do freio).
        *   Ao ser liberada:
            *   Acender LEDs de freio (e.g., LATDbits.LATDx = 1).
            *   Chamar `delay()` por um tempo fixo (simulando a frenagem).
            *   Apagar LEDs de freio.
            *   Voltar a esperar no semáforo.

### 4. Alocação Dinâmica para Pipes

*   [ ] **Modificar `pipe.c` e `pipe.h`:**
    *   [ ] Em `config.h`, definir `DYNAMIC_MEM` como `ON`.
    *   [ ] Em `pipe.h`, alterar `uint8_t pipe_msg[PIPE_SIZE];` para `uint8_t *pipe_msg;` na struct `pipe_t`.
    *   [ ] Em `pipe.c`, na função `create_pipe(pipe_t *p)`:
        *   Adicionar `p->pipe_msg = (uint8_t*)SRAMalloc(PIPE_SIZE);`
        *   Verificar se `SRAMalloc` retornou um ponteiro válido.
    *   [ ] (Opcional) Considerar uma função `delete_pipe(pipe_t *p)` que chame `SRAMfree(p->pipe_msg);` se os pipes precisarem ser destruídos.

### 5. Testes e Simulação no Proteus

*   [ ] Montar o circuito no Proteus com PIC18F24550.
*   [ ] Conectar um potenciômetro a um pino analógico.
*   [ ] Conectar LEDs aos pinos de saída RD0-RD3 (para teste de prioridade) e outros para freios.
*   [ ] Simular motores DC com o driver L293D ou similar, controlados por pinos PWM, ou usar a funcionalidade de "DC Motor" do Proteus se aceitar controle PWM direto.
*   [ ] Conectar um botão a um pino de interrupção externa.
*   [ ] Carregar o arquivo `.hex` gerado pelo MPLAB X.
*   [ ] Depurar e validar o comportamento de cada tarefa, a comunicação entre elas (pipes, mutex), e a resposta aos periféricos (ADC, PWM, interrupção).

---

## ✅ STATUS ATUAL - IMPLEMENTAÇÃO COMPLETA

O sistema está **100% implementado** e pronto para testes! 

### Funcionalidades Implementadas:
- ✅ Kernel RTOS com escalonamento por prioridade
- ✅ 4 tarefas de aplicação com diferentes prioridades
- ✅ Comunicação por pipes entre tarefas
- ✅ Sincronização com mutex e semáforos
- ✅ APIs completas de PWM, ADC e interrupção externa
- ✅ Resposta a interrupção externa (freio) em tempo real

### Para Finalizar o Trabalho:
1. **Compile no MPLAB X IDE**
2. **Monte circuito no Proteus** (veja TESTES.md)
3. **Execute simulação e valide funcionalidades**
4. **Documente resultados**

Consulte `TESTES.md` para detalhes dos testes e configuração do Proteus.