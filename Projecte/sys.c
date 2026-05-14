/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_ticks;

char sys_buffer[256];

int check_fd(int fd, int permissions)
{
	if (fd!=1) return -9; /*EBADF*/
	if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
	return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_write(int fd, char *buffer, int size) {
	int err;
//	char sys_buffer[256]; //hay q ponerlo como global, de momento lo dejo pa no hacer un push solo de esto. Pero es por seguridad pa que no puedan hacer stack smashing a la pila de sistema, o algo asi
	int bytes_left;
	int ret = 0;
	
	// CHECKS -- comprobar datos del usuario
	err = check_fd(fd, ESCRIPTURA); // solo escritura
	if (err < 0) return err;	// fd invalido

	if (buffer == NULL) return -EFAULT; // -EFAULT bad address
	if (size < 0) return -EINVAL;		// -EINVAL invalid argument
	
	bytes_left = size;

	while (bytes_left > 0) {
		int to_copy = (bytes_left > sizeof(sys_buffer)) ? sizeof(sys_buffer) : bytes_left;

		err = copy_from_user(buffer + ret, sys_buffer, to_copy);
		if (err < 0) return err;
		
		err = sys_write_console(sys_buffer, to_copy);
		if (err < 0) return err;

		ret += err;
		bytes_left -= err;
	}

	return ret;
}


int sys_gettime() {
	return zeos_ticks;
}

int sys_getpid() {
	return current()->PID;
}

int ret_from_fork() {
	return 0;
}

void sys_exit()//elimina current y alibera todos sus recursos
{
	struct task_struct * pcb = current();
	page_table_entry * PT = get_PT(pcb);

	pcb->PID = -1;
	pcb->quantum = -1;

	//NO HACE FALTA PQ EL Q SE ESTA EJECUTANDO NO ESTA EN LA LISTA, YA Q ES SOLO PARA LOS READY
	//list_del(&(pcb->list));

	for(int i = PAG_LOG_INIT_DATA ; i < NUM_PAG_DATA + NUM_PAG_CODE ; i++)
	{
		int frame = get_frame(PT,i);
		free_frame(frame);
		del_ss_pag(PT,i);
	}
	pcb->dir_pages_baseAddr = NULL;

	list_del(&(pcb->siblings));

	struct list_head *item, *next;
	list_for_each_safe(item,next,&(pcb->children))
	{
		struct task_struct *child = list_head_to_task_struct(item);

		list_del(item);

		child->parent = idle_task;
		list_add_tail(item , &(idle_task->children));
	}
	
  	sched_next_rr();
}

void sys_block()
{
	struct task_struct * crrnt = current();

	if(crrnt->pending_unblocks > 0)
		crrnt->pending_unblocks--;
	else
	{
		crrnt->status = ST_BLOCKED;
		list_add_tail(&(crrnt->list),&blocked);
		sched_next_rr();
	}
}

int sys_unblock(int pid)
{
	struct list_head *pos , *head;

	head = &(current()->children);
	struct task_struct *child = NULL;

	list_for_each(pos,head)
	{
		//como el task_struct está siempre al inicio de una pagina, accedemos a el con este truquito
		struct task_struct *t = (struct task_struct *)((unsigned long)pos & 0xfffff000);		
		if(t->PID == pid)
		{
			if(t->status == ST_BLOCKED)
			{
				t->status = ST_READY;
				list_del(&(t->list));
				list_add_tail(&(t->list),&readyqueue);
			}
			else
			{
				t->pending_unblocks++;
			}
			return 0;
		}
	}
	return -1;
}


int sys_fork()
{
	struct task_struct *parent = current(); // PCB del padre
	union task_union *child;
	page_table_entry *SPT = (page_table_entry *) (parent->dir_pages_baseAddr[0].bits.pbase_addr << 12);
	page_table_entry *SPT2 = (page_table_entry *) (parent->dir_pages_baseAddr[1].bits.pbase_addr << 12);

	int Childframe = alloc_frame();
	if(Childframe < 0)
	{
		return -ENOMEM; //devuelve error si no queda espacio para otro proceso
	}
	child = (union task_union *) (Childframe << 12);
	set_ss_pag(SPT, Childframe, Childframe, 0);
	set_ss_pag(SPT2,Childframe, Childframe, 0);
	set_cr3(get_DIR(parent));
	copy_data(parent, child, sizeof(union task_union)); //se copia tol union de padre al frame del hijo

	/**/
    int child_pd_frame = alloc_frame();
    if (child_pd_frame < 0) {return -ENOMEM; }
	set_ss_pag(SPT,child_pd_frame,child_pd_frame,0);
	set_ss_pag(SPT2,child_pd_frame,child_pd_frame,0);
	set_cr3(get_DIR(parent));
    child->task.dir_pages_baseAddr = (page_table_entry *) (child_pd_frame << 12);
    clear_page_table(child->task.dir_pages_baseAddr);
    
    // Compartir el mapa del sistema(Todos los procesos apuntan al mismo)
    child->task.dir_pages_baseAddr[0] = parent->dir_pages_baseAddr[0];
	child->task.dir_pages_baseAddr[1] = parent->dir_pages_baseAddr[1];

    //Asignar Tabla de Páginas de Usuario
    int child_upt_frame = alloc_frame();
    if (child_upt_frame < 0) {return -ENOMEM;}
	set_ss_pag(SPT,child_upt_frame,child_upt_frame,0);
	set_ss_pag(SPT2,child_upt_frame,child_upt_frame,0);
	set_cr3(get_DIR(parent));
    
    child->task.dir_pages_baseAddr[2].entry = 0;
    child->task.dir_pages_baseAddr[2].bits.pbase_addr = child_upt_frame;
    child->task.dir_pages_baseAddr[2].bits.present = 1;
    child->task.dir_pages_baseAddr[2].bits.user = 1;
    child->task.dir_pages_baseAddr[2].bits.rw = 1;	

    // Obtener punteros a las tablas físicas
    page_table_entry *parent_PT = (page_table_entry *) (parent->dir_pages_baseAddr[2].bits.pbase_addr << 12);
    page_table_entry *child_PT  = (page_table_entry *) (child->task.dir_pages_baseAddr[2].bits.pbase_addr << 12);

    // Herencia de código, padre e hijo lo comparten
    for (int i = 0; i < NUM_PAG_CODE; i++) {
        child_PT[NUM_PAG_DATA + i] = parent_PT[NUM_PAG_DATA + i];
    }

	for(int i = 0 ; i < NUM_PAG_DATA ; i++)
	{
		int new_frame = alloc_frame();
		if(new_frame < 0) // Free todos los allocate usados si no hay espacio para todos los necesarios
		{
			for(int j = 0 ; j < i ; j++)
			{
				del_ss_pag(child_PT,j);
				free_frame(get_frame(child_PT,j));
				
			}
			free_frame(Childframe);
			free_frame(child_pd_frame);
			free_frame(child_upt_frame);
			return -ENOMEM;
		}
		set_ss_pag(child_PT,i,new_frame,1);// linkar logica a fisicas hijo
		
		int temp_logical_page = NUM_PAG_CODE + NUM_PAG_DATA + 1; // pagina de memoria del padre q no use (Uso la siguiente despues de datos)
		
		set_ss_pag(parent_PT,temp_logical_page, new_frame, 1); // se linka la nueva logica del hijo a una vacia del padre para poder acceder

		//se copian los datos de las paginas del padre a la temporal q apunta a la fisica del hijo.
		copy_data((void*)((PAG_LOG_INIT_DATA + i) << 12),
		(void*)(L_USER_START + ((temp_logical_page) << 12)), PAGE_SIZE);

		del_ss_pag(parent_PT,temp_logical_page); // se elimina la traducción a fisica del hijo
		set_cr3(get_DIR(parent));
	}
	latestPID++;
	child->task.PID = latestPID;
	child->task.quantum = INIT_QUANTUM;
	/*
	 * 0 (fake EBP)
	 * @ret (dirección a ret_from_fork)
	 * @ret (return a handler)
	 * CTXSW
	 * CTXHW
	 */
	// CTXHW(5 regs) + CTXSW(11 regs) + @retHandler = 17 = 0x11
	//Modificamos el stack para que lleve al usuario a ret_from_fork() y añadimos fake EBP para q al hacer pop en switch, no nos quite el ret q queremos
	((unsigned long*)KERNEL_ESP(child))[-0x13] = (unsigned long) 0; // Fake EBP
	((unsigned long*)KERNEL_ESP(child))[-0x12] = (unsigned long) ret_from_fork; // Dirección ret_from_fork
	child->task.kernel_esp = &((unsigned long *)KERNEL_ESP(child))[-0x13]; //Marcamos el tope del KernelESP como k_esp del hijo

	
	list_add_tail(&(child->task.list), &readyqueue);

	child->task.parent = current(); 
	child->task.pending_unblocks = 0;
	child->task.status = ST_READY;
	INIT_LIST_HEAD(&(child->task.children));//inicializa lista hijos 
	INIT_LIST_HEAD(&(child->task.listIO));  //inicializa lista blockedIO

	list_add(&(child->task.siblings), &(current()->children)); //se añade child a la lista children del padre

	return child->task.PID; //devolvemos PID hijo (esto lo hace solo el padre, el hijo va a ret_from_fork(si no peta nada))
}

int sys_read(char* b, int maxchars)
{
	if (maxchars < 0) return -EINVAL;
    if (maxchars == 0) return 0;
	
	int chars_read = 0;
	
	if(maxchars>1024)
		maxchars = 1024;
	
	while(chars_read < maxchars)
	{
		if(cbuffer.count == 0)
		{
			if(list_first(&(blockedIO)) != &(current()->listIO))
				list_add_tail(&(current()->listIO), &blockedIO);
			current()->status = ST_BLOCKED;
			sched_next_rr();  // El interrupt me añadirá a readyqueue al haber interrupt
		}
		// Leer todos los caracteres disponibles
		while (cbuffer.count != 0 && chars_read < maxchars) {
			char c = cbuffer_read();
			b[chars_read] = c;
			chars_read++;
		}
	}
	
	struct list_head *blocked_task_list = list_first(&blockedIO);
	list_del(blocked_task_list);//lo quito de blockedIO, respetando FIFO en lectura de chars
	return 1;
}
