#ifndef CONFIG_H
#define	CONFIG_H

#define ON                  1
#define OFF                 2

#define MAX_USER_TASKS      4
#define MAX_STACK_SIZE      32

#define DEFAULT_SCHEDULER   RR_SCHEDULER

#define IDLE_DEBUG          ON

#define DYNAMIC_MEM         OFF

#define PIPE_SIZE           3

// Aplica��o exemplo

// APP_1 exemplo somente das tarefas
#define APP_1               ON

// APP_2 exemplo das tarefas com sem�foro
#define APP_2               OFF

// APP_3 exemplo das tarefas com comunica��o via pipe
#define APP_3               OFF

#endif	/* CONFIG_H */

