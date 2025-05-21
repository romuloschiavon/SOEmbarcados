#ifndef SYNC_H
#define	SYNC_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

// Estrutura de dados para o semáforo
typedef struct sem {
    int16_t s_value;
    uint8_t s_queue[MAX_USER_TASKS];
    uint8_t s_size;
    uint8_t s_pos_out;
} sem_t;

// Estrutura de dados para variáveis mutex
typedef struct mutex {
    bool flag;
    
    // forma dois
    uint8_t s_queue[MAX_USER_TASKS];
    uint8_t s_size;
    uint8_t s_pos_out;
} mutex_t;

// API para o semáforo
void sem_init(sem_t *sem, int16_t value);
void sem_wait(sem_t *sem);
void sem_post(sem_t *sem);

// API para o mutex
void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);


#endif	/* SYNC_H */

