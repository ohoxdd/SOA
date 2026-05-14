/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <hardware.h>

extern void writeMSR(int msr, int value);

char initial_stack[KERNEL_STACK_SIZE]; // Space for the initial system stack
struct list_head readyqueue;
struct list_head blocked;
struct list_head blockedIO;
int latestPID;
int quantum;

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

	idle->task.quantum = INIT_QUANTUM;

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

	//Nueva TP? -- Revisar q esto funcione correctamente --
	int OS2_frame = alloc_frame();
	page_table_entry *OS2Address = (page_table_entry *)(OS2_frame << 12);
	clear_page_table(OS2Address);

	//TP Usuario
	int Us_frame = alloc_frame();
	page_table_entry *UsAdress = (page_table_entry *)(Us_frame << 12);
	clear_page_table(UsAdress);

	set_kernel_pages(OsAddress);
	set_user_pages(UsAdress);

	//Traducciones fisica logica pa mas adelante
	set_ss_pag(OsAddress,Dir,Dir,0);
	set_ss_pag(OsAddress,Os_frame,Os_frame,0);
	set_ss_pag(OsAddress,OS2_frame,OS2_frame,0);
	set_ss_pag(OsAddress,Us_frame,Us_frame,0);

	// Asignar SO a la primera entrada DIR
	DirAddress[0].entry = 0;
	DirAddress[0].bits.pbase_addr = Os_frame;
	DirAddress[0].bits.present = 1;
	DirAddress[0].bits.rw = 1;
	DirAddress[0].bits.user = 0;

	// Asignar SO a la primera entrada DIR
	DirAddress[1].entry = 0;
	DirAddress[1].bits.pbase_addr = OS2_frame;
	DirAddress[1].bits.present = 1;
	DirAddress[1].bits.rw = 1;
	DirAddress[1].bits.user = 0;

	//Asignar USER a la segunda entrada DIR
	DirAddress[2].entry = 0;
	DirAddress[2].bits.pbase_addr = Us_frame;
	DirAddress[2].bits.present = 1;
	DirAddress[2].bits.rw = 1;
	DirAddress[2].bits.user = 1;


	//Alocatar el Process Control Block(lo q guarda estado de cada proceso diria)
	union task_union *task1 = (union task_union *)(alloc_frame() << 12);
	int pcb_frame = ((unsigned int) task1) >> 12;
	set_ss_pag(OsAddress, pcb_frame, pcb_frame, 0);

	task1->task.PID = 1;
	task1->task.quantum = INIT_QUANTUM;
	task1->task.status = ST_RUN;
	INIT_LIST_HEAD(&(task1->task.children)); //Init lista children
	task1->task.parent = NULL; //Padre en NULL pq es la primera task
	list_add(&(task1->task.siblings),&(task1->task.children));

	latestPID = task1->task.PID;
	quantum   = task1->task.quantum;

	//esp apunta al nuevo max
	tss.esp0 = (unsigned long)(task1) + KERNEL_STACK_SIZE * sizeof(unsigned long);

	//Se actualiza con writeMSR para SysEnter
	writeMSR(0x175,tss.esp0);

	task1->task.dir_pages_baseAddr = DirAddress;
	//printk("test"); //pa saber cuando estoy a punto de hacer el set_cr3
	set_cr3(DirAddress);

	init_task = &(task1->task);
	//list_add_tail(&init_task->list, &readyqueue);
}

void init_sched()
{
	INIT_LIST_HEAD(&readyqueue); //declaración queue para procesos en ready
	INIT_LIST_HEAD(&blocked);
	INIT_LIST_HEAD(&blockedIO);
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
	
	tss.esp0 = (unsigned long)(new) + KERNEL_STACK_SIZE * sizeof(unsigned long);
	writeMSR(0x175,tss.esp0);
	set_cr3(new->task.dir_pages_baseAddr);
	struct task_struct *old_task = current();
	task_switch_part2(&current()->kernel_esp , new->task.kernel_esp);
}

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
	return list_entry(l ,struct task_struct , list);
}	

int needs_sched_rr(void)
{
	if(quantum<=0 && !list_empty(&readyqueue)) 
		return 1;
	return 0;
}

void update_sched_data_rr(void)
{
	quantum--;
	if(needs_sched_rr()) {
		if(idle_task != current()) //pq idle es caso excepcional
			update_process_state_rr(current(), &readyqueue);
			
		sched_next_rr();
	}
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue)
{
	struct list_head * temp_list = &t->list;

	if(temp_list->next != NULL && temp_list->prev != NULL)//comprueba q esté en una lista mirando los valores de antes/despues
		list_del(temp_list);

	//si se ha dado valor de dst_queue, se le pasa el list_head de task
	if(dst_queue)
	{
		list_add_tail(temp_list, dst_queue); //ESTO ES LO Q AÑADE DE NUEVO EL CURRENT AL HACER CAMBIO
		t->status = ST_READY;
	}
}

void sched_next_rr(void)
{
	struct task_struct *next_task;

	if(!list_empty(&readyqueue)) //Si la lista no esta vacia pone el siguiente proceso en running
	{
		struct list_head *next_list = list_first(&readyqueue);
		list_del(next_list);//se elimina de la lista
		next_task = list_head_to_task_struct(next_list); //se carga el valor q tenga el list_head en next_task
	}
	else//si readyqueue esta vacio, se activa idle
	{
		next_task = idle_task;
	}
	//list_add_tail(&(current()->list),&readyqueue);
	quantum = next_task->quantum;
	next_task->status = ST_RUN;
	task_switch(next_task);
}

int get_quantum (struct task_struct *t){
	return t->quantum;
}

void set_quantum (struct task_struct *t, int new_quantum){
	t->quantum = new_quantum;
}

