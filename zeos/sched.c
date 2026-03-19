/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <hardware.h>

extern void writeMSR(int msr, int value);

char initial_stack[KERNEL_STACK_SIZE]; // Space for the initial system stack
struct task_struct * init_task;
struct task_struct * idle_task;



void cpu_idle(void)
{
	__sti();
	while(1)
	{
	;
	}
}

void init_idle (void)
{
		//Tabla DIR
	int Dir = alloc_frame();//indice TP
	page_table_entry *DirAddress = (page_table_entry *)(Dir << 12); //la dirección son los 20B de mayor peso con 0 en los 12 de menor peso
	clear_page_table(DirAddress);
	DirAddress[0].entry = init_task->dir_pages_baseAddr[0].entry;

	union task_union *idle = (union task_union *)(alloc_frame() << 12);

	int pcb_frame = ((unsigned int) idle >> 12);

	//direccio de la TP del OS, la pillamos del Dir de task1.
	page_table_entry *OsAddress = (page_table_entry *) (init_task->dir_pages_baseAddr[0].bits.pbase_addr << 12);

	set_ss_pag(OsAddress,pcb_frame,pcb_frame,0);

	//En el stack se pone cpu_idle como la @ donde saltar, y 0 donde apunta el k_esp
	idle->stack[KERNEL_STACK_SIZE - 1] = cpu_idle;
	idle->stack[KERNEL_STACK_SIZE - 2] = 0;

	//Marca el esp como donde tenemos el 0 del cpu_idle, para ocupar lo minimo posible
	idle->task.kernel_esp = (unsigned long)&idle->stack[KERNEL_STACK_SIZE - 2];

	//PID es 0
	idle->task.PID = 0;

	//Se le da el DIR que antes hemos creado para este proceso
	idle->task.dir_pages_baseAddr = DirAddress;

	//Se guarda la idle task como global
	idle_task = &(idle->task);	
}

void init_task1(void)
{
	//Tabla DIR
	int Dir = alloc_frame();//indice TP
	page_table_entry *DirAddress = (page_table_entry *)(Dir << 12); //la dirección son los 20B de mayor peso con 0 en los 12 de menor peso
	clear_page_table(DirAddress);

	//TP OS
	int Os_frame = alloc_frame();
	page_table_entry *OsAddress = (page_table_entry *)(Os_frame << 12);
	clear_page_table(OsAddress);

	//TP Usuario
	int Us_frame = alloc_frame();
	page_table_entry *UsAdress = (page_table_entry *)(Us_frame << 12);
	clear_page_table(UsAdress);

	set_kernel_pages(OsAddress);
	set_user_pages(UsAdress);

	//Traducciones fisica logica pa mas adelante
	int stack_frame = ((unsigned int) initial_stack) >> 12;
	set_ss_pag(OsAddress, stack_frame, stack_frame, 0);
	set_ss_pag(OsAddress,Dir,Dir,0);
	set_ss_pag(OsAddress,Os_frame,Os_frame,0);
	set_ss_pag(OsAddress,Us_frame,Us_frame,0);

	// Asignar SO a la primera entrada DIR
	DirAddress[0].entry = 0;
	DirAddress[0].bits.pbase_addr = Os_frame;
	DirAddress[0].bits.present = 1;
	DirAddress[0].bits.rw = 1;
	DirAddress[0].bits.user = 0;

	//Asignar USER a la segunda entrada DIR
	DirAddress[1].entry = 0;
	DirAddress[1].bits.pbase_addr = Us_frame;
	DirAddress[1].bits.present = 1;
	DirAddress[1].bits.rw = 1;
	DirAddress[1].bits.user = 1;

	//Alocatar el Process Control Block(lo q guarda estado de cada proceso diria)
	union task_union *task1 = (union task_union *)(alloc_frame() << 12);
	int pcb_frame = ((unsigned int) task1) >> 12;
	set_ss_pag(OsAddress, pcb_frame, pcb_frame, 0);

	//PID task1
	task1->task.PID = 1;

	//esp apunta al nuevo max
	tss.esp0 = (unsigned long)(task1) + KERNEL_STACK_SIZE * sizeof(unsigned long);

	//Se actualiza con writeMSR para SysEnter
	writeMSR(0x175,tss.esp0);

	task1->task.dir_pages_baseAddr = DirAddress;

	set_cr3(DirAddress);

	init_task = &(task1->task);
}

void init_sched()
{

}

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t)
{
       return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t)
{
       return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

void inner_task_switch(union task_union *new)
{
	set_cr3(new->task.dir_pages_baseAddr);
	tss.esp0 = (unsigned long)(new) + KERNEL_STACK_SIZE * sizeof(unsigned long);
	writeMSR(0x175,tss.esp0);
	task_switch_part2(&current()->kernel_esp , new->task.kernel_esp);
}