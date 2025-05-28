#ifndef USER_APP_H
#define	USER_APP_H

#include "types.h"

// Tarefas
TASK tarefa_acelerador(void);
TASK tarefa_controle_central(void);
TASK tarefa_injecao_eletronica(void);
TASK tarefa_controle_estabilidade(void);

// Freio
void ISR_FREIO(void);

// Config do user
void user_config(void);

// ===== RECURSOS DE COMUNICAÃÃO E SINCRONIZAÃÃO =====
pipe_t pipe_acelerador_central;   // ComunicaÃ§Ã£o unidirecional
mutex_t mutex_injecao;            // ProteÃ§Ã£o de recursos compartilhados
sem_t sem_freio;                  // SinalizaÃ§Ã£o de eventos
sem_t sem_injecao;                // SinalizaÃ§Ã£o de novos dados de injeÃ§Ã£o

// VariÃ¡vel protegida pelo mutex
unsigned int g_tempo_injecao = 0;

// variaveis de controle
volatile uint8_t estabilidade = 0;
volatile uint8_t freio = 0;


#endif	/* USER_APP_H */

