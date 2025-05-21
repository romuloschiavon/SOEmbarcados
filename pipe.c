#include "pipe.h"
#include "mem.h"

void create_pipe(pipe_t *p)
{
    //p->pipe_msg = (uint8_t*)SRAMalloc(PIPE_SIZE);
    p->pipe_pos_read    = 0;
    p->pipe_pos_write   = 0;
    sem_init(&p->pipe_sem_read, 0);
    sem_init(&p->pipe_sem_write, PIPE_SIZE);
}

void write_pipe(pipe_t *p, uint8_t data)
{
    di();
    
    sem_wait(&p->pipe_sem_write);
    p->pipe_msg[p->pipe_pos_write] = data;
    p->pipe_pos_write =  (p->pipe_pos_write+1) % PIPE_SIZE;
    sem_post(&p->pipe_sem_read);   
    
    ei();    
}

void read_pipe(pipe_t *p, uint8_t *data)
{
    di();
    
    sem_wait(&p->pipe_sem_read);
    *data = p->pipe_msg[p->pipe_pos_read];
    p->pipe_pos_read = (p->pipe_pos_read+1) % PIPE_SIZE;
    sem_post(&p->pipe_sem_write);
    
    ei();
}

