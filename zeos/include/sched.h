/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define KERNEL_STACK_SIZE	1024
#define INIT_QUANTUM 10

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };
extern int latestPID;
extern int quantum;

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr;
  unsigned long kernel_esp;
  struct list_head list;
  int quantum;
  enum state_t status;
  int pending_unblocks;
  struct task_struct * parent;
  struct list_head children;
  struct list_head siblings;
};



union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};


#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]
extern struct task_struct * idle_task;
extern struct list_head blocked;
extern struct list_head readyqueue;
extern char initial_stack[KERNEL_STACK_SIZE];
#define INITIAL_ESP             (DWord) &initial_stack[KERNEL_STACK_SIZE]

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

// Funcion q devuelve la dirección de memoria de un task_struct dado la @ de un list_head
struct task_struct *list_head_to_task_struct(struct list_head *l);

//Function to update the relevant information to take scheduling decisions
void update_sched_data_rr (void);

//Function to decide if it is necessary to change the current process (1==yes , 0==no)
int needs_sched_rr (void);

//Function to update the current state of a process to a new state
void update_process_state_rr (struct task_struct *t, struct list_head *dst_queue);

//Function to select the next process to execute, to extract it from the ready queue and to invoke the context switch process
void sched_next_rr (void);

int get_quantum (struct task_struct *t);
void set_quantum (struct task_struct *t, int new_quantum);


#endif  /* __SCHED_H__ */
