/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <sched.h>
#include <devices.h>
#include <list.h>

#include <zeos_interrupt.h>

// avisamos de que existen los handlers
extern void keyboard_handler();
extern void system_call_handler();
extern void clock_handler();
extern void writeMSR(int msr, int value);
extern void syscall_handler_sysenter();



// ticks para el reloj
int zeos_ticks = 0;

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
 /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
	//registramos en la idt el handler de teclado asociandolo
	//al interrupt de teclado (el numero 33)
	setInterruptHandler(33, keyboard_handler, 0);
	setInterruptHandler(32, clock_handler, 0);

	writeMSR(0x174, __KERNEL_CS);
	writeMSR(0x175, INITIAL_ESP);
	writeMSR(0x176, (unsigned long)syscall_handler_sysenter);

  set_idt_reg(&idtR);

}

void keyboard_routine() {
	unsigned char c;
	unsigned char scan_code;

	c = inb(0x60);
	
	// Solo hacemos algo cuando baja la letra, es decir
	// cuando el bit 7 de inb es 0 (Make)
	if ((c & 0x80) == 0)  {

		//task_switch(idle_task);

		// 7 LSB = scan code
		scan_code = c & 0x7F;
		// ver q tecla es en ascii
		char ascii_char = char_map[scan_code];

    
		//imprimimos
		if (ascii_char != '\0') {
      int c = cbuffer_write(ascii_char);
      if(c < 0)
      {
        return; // cbuffer lleno
      }
      // Desbloquear un proceso bloqueado si los hay
      if(!list_empty(&blockedIO))
      {
        struct list_head *blocked_task_list = list_first(&blockedIO);
        struct task_struct *blocked_task = list_entry(blocked_task_list, struct task_struct, listIO);
        
        // Cambiar estado a READY y mover a readyqueue
        blocked_task->status = ST_READY;
        //list_del(blocked_task_list); //SE HACE EN SYS_READ PA QUE ESPERE HASTA LEER TODO ANTES DE CAMBIAR DE PROC LECTOR
        list_add_tail(&(blocked_task->list), &readyqueue);
      }
			printc_xy(0,0,ascii_char);
		} else {
			printc_xy(0,0,'C');
		}
	}
}


void  clock_routine(void) {
	//zeos_show_clock();
  update_sched_data_rr();
	zeos_ticks++;
}

