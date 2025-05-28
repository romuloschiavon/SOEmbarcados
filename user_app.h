#ifndef USER_APP_H
#define	USER_APP_H

#include "types.h"

// Declaração das tarefas do sistema
TASK tarefa_acelerador(void);
TASK tarefa_controle_central(void);
TASK tarefa_injecao_eletronica(void);
TASK tarefa_controle_estabilidade(void);

// Rotina de interrupção do freio
void ISR_FREIO(void);

// Configuração inicial do usuário
void user_config(void);

// Recursos de comunicação e sincronização entre tarefas
pipe_t pipe_acelerador_central;   // Comunicação entre acelerador e controle central
mutex_t mutex_injecao;            // Proteção da variável de tempo de injeção
sem_t sem_freio;                  // Sinalização do freio de emergência
sem_t sem_injecao;                // Sinalização de novos dados de injeção

// Variável compartilhada protegida pelo mutex
unsigned int g_tempo_injecao = 0;

// Variáveis de controle do sistema
volatile uint8_t estabilidade = 0; // Flag do sistema de estabilidade ativo
volatile uint8_t freio = 0;        // Flag do freio de emergência ativo

#endif	/* USER_APP_H */

