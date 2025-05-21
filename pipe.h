#ifndef PIPE_H
#define	PIPE_H

#include "sync.h"
#include "types.h"
#include "config.h"

typedef struct pipe {
    uint8_t pipe_msg[PIPE_SIZE];
    //uint8_t *pipe_msg;
    uint8_t pipe_pos_read;
    uint8_t pipe_pos_write;
    sem_t pipe_sem_read;
    sem_t pipe_sem_write;
} pipe_t;


void create_pipe(pipe_t *p);
void write_pipe(pipe_t *p, uint8_t data);
void read_pipe(pipe_t *p, uint8_t *data);



#endif	/* PIPE_H */

