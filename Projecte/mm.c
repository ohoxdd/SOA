/*
 * mm.c - Memory Management: Paging & segment memory management
 */

#include "include/mm.h"
#include <types.h>
#include <mm.h>
#include <segment.h>
#include <hardware.h>
#include <sched.h>
#include <io.h>
#include <zeos_mm.h>

int __end_system __attribute__((__section__(".data.last"))); // Magical symbol to force a final symbol after all the others to keep the BSS section intact in the binary file. Otherwise, the BSS section is deleted, which is a problem because we require its space for some structures during the initialization step (for example: init_idt)
extern unsigned int *p_sys_size;
extern unsigned int *p_usr_size;
int first_kernel;
int last_kernel;

/* Bytemap to mark the free physical pages */
Byte phys_mem[TOTAL_PAGES];
#define FREE_FRAME 0
#define USED_FRAME 1

/* Kernel page tables for mapping extended memory (0-2047 frames) */
page_table_entry* kernel_page_tables[KERNEL_PAGE_TABLES];


/* SEGMENTATION */
/* Memory segements description table */
Descriptor  *gdt = (Descriptor *) GDT_START;
/* Register pointing to the memory segments table */
Register    gdtR;

/* TSS */
TSS         tss; 

extern char* itoa(int, char*);

/***********************************************/
/************** PAGING MANAGEMENT **************/
/***********************************************/
#define PAGE_TABLE_ENTRIES 1024
/* Initializes the page table */
void clear_page_table(page_table_entry* process_PT)
{
  int i;
  for (i=0; i<PAGE_TABLE_ENTRIES; i++) {
    process_PT[i].entry = 0;
  }
}


/* Initialize pages for initial process (user pages) */
void set_user_pages(page_table_entry* process_PT)
{
 int pag; 
 int new_ph_pag;

  /* CODE */
  for (pag=0;pag<NUM_PAG_CODE;pag++){
	new_ph_pag=alloc_frame();
    process_PT[NUM_PAG_DATA+pag].entry = 0;
    process_PT[NUM_PAG_DATA+pag].bits.pbase_addr = new_ph_pag;
    process_PT[NUM_PAG_DATA+pag].bits.user = 1;
    process_PT[NUM_PAG_DATA+pag].bits.present = 1;
  }
  
  /* DATA */ 
  for (pag=0;pag<NUM_PAG_DATA;pag++){
	new_ph_pag=alloc_frame();
    process_PT[pag].entry = 0;
    process_PT[pag].bits.pbase_addr = new_ph_pag;
    process_PT[pag].bits.user = 1;
    process_PT[pag].bits.rw = 1;
    process_PT[pag].bits.present = 1;
  }
  show_PT_range(process_PT, 0, NUM_PAG_DATA+NUM_PAG_CODE, " Reserved for User memory\n");
}

void set_kernel_pages (page_table_entry* process_PT)
{
  int i;
  /* Init kernel pages */
  for (i=first_kernel; i<last_kernel; i++) { // Logical page equal to physical page (frame)
    set_ss_pag(process_PT, i, i, 0);
  }
  set_ss_pag(process_PT, 0x90, 0x90, 0); /* 0x90000 Mapped GDT */
  set_ss_pag(process_PT, 0xb8, 0xb8, 0); /* 0xb8000 Mapped screen */

  printk("\n");
  show_PT_range(process_PT, first_kernel, last_kernel-first_kernel+1, " Reserved for kernel memory\n");
  show_PT_range(process_PT, 0x90, 1, " Reserved for GDT\n");
  show_PT_range(process_PT, 0xb8, 1, " Reserved for Screen buffer\n");
}

/* init_kernel_page_tables - Initialize kernel page table array pointers */
void init_kernel_page_tables(page_table_entry* pt0, page_table_entry* pt1)
{
    kernel_page_tables[0] = pt0;
    if (KERNEL_PAGE_TABLES > 1) {
        kernel_page_tables[1] = pt1;
    }
}

/* Enable paging, modifying the CR0 register */
void set_pe_flag()
{
  unsigned int cr0 = read_cr0();
  cr0 |= 0x80000000;
  write_cr0(cr0);
}


/* Initializes paging for the system address space */
void init_mm()
{
  first_kernel = KERNEL_START>>12;
  last_kernel = ((KERNEL_START + *p_sys_size + *p_usr_size)>>12) + 1;

  init_frames();
	alloc_shm_frames();

}
/***********************************************/
/************** SEGMENTATION MANAGEMENT ********/
/***********************************************/
void setGdt()
{
  /* Configure TSS base address, that wasn't initialized */
  gdt[KERNEL_TSS>>3].lowBase = lowWord((DWord)&(tss));
  gdt[KERNEL_TSS>>3].midBase  = midByte((DWord)&(tss));
  gdt[KERNEL_TSS>>3].highBase = highByte((DWord)&(tss));

  gdtR.base = (DWord)gdt;
  gdtR.limit = 256 * sizeof(Descriptor);

  set_gdt_reg(&gdtR);
}

/***********************************************/
/************* TSS MANAGEMENT*******************/
/***********************************************/
void setTSS()
{
  tss.PreviousTaskLink   = NULL;
  tss.esp0               = INITIAL_ESP;
  tss.ss0                = __KERNEL_DS;
  tss.esp1               = NULL;
  tss.ss1                = NULL;
  tss.esp2               = NULL;
  tss.ss2                = NULL;
  tss.cr3                = NULL;
  tss.eip                = 0;
  tss.eFlags             = INITIAL_EFLAGS; /* Enable interrupts */
  tss.eax                = NULL;
  tss.ecx                = NULL;
  tss.edx                = NULL;
  tss.ebx                = NULL;
  tss.esp                = USER_ESP;
  tss.ebp                = tss.esp;
  tss.esi                = NULL;
  tss.edi                = NULL;
  tss.es                 = __USER_DS;
  tss.cs                 = __USER_CS;
  tss.ss                 = __USER_DS;
  tss.ds                 = __USER_DS;
  tss.fs                 = NULL;
  tss.gs                 = NULL;
  tss.LDTSegmentSelector = KERNEL_TSS;
  tss.debugTrap          = 0;
  tss.IOMapBaseAddress   = NULL;

  set_task_reg(KERNEL_TSS);
}

 
/* Initializes the ByteMap of free physical pages.
 * The kernel pages are marked as used */
int init_frames( void )
{
    int i;
    /* Mark pages as Free */
    for (i=0; i<TOTAL_PAGES; i++) {
        phys_mem[i] = FREE_FRAME;
    }
    // Manually reserve kernel frames...
    for (i=first_kernel; i<last_kernel; i++) {
        phys_mem[i] = USED_FRAME;
    }

    // Special pages used by bootsect or devices...
    phys_mem[0x90] = USED_FRAME; /* 0x90000 GDT */
    for (i=0xA0; i<0x100; i++) {/* 0xA0000-0xBFFFF screen, 0xC0000-0xFFFFF BIOS */
        phys_mem[i] = USED_FRAME;
    }
    return 0;
}

/* alloc_frame - Search a free physical page (== frame) and mark it as USED_FRAME. 
 * Returns the frame number or -1 if there isn't any frame available. */
int alloc_frame( void )
{
    int i;
    for (i=last_kernel; i<TOTAL_PAGES; i++) {
        if (phys_mem[i] == FREE_FRAME) {
            phys_mem[i] = USED_FRAME;
            return i;
        }
    }

    return -1;
}

void free_user_pages( page_table_entry* process_PT )
{
 int pag;
    /* DATA */
 for (pag=0;pag<NUM_PAG_DATA;pag++){
	 free_frame(process_PT[PAG_LOG_INIT_DATA+pag].bits.pbase_addr);
         process_PT[PAG_LOG_INIT_DATA+pag].entry = 0;
 }
}


/* free_frame - Mark as FREE_FRAME the frame  'frame'.*/
void free_frame( unsigned int frame )
{
    if (frame<TOTAL_PAGES)
      phys_mem[frame]=FREE_FRAME;
}

/* set_ss_pag - Associates logical page 'page' with physical page 'frame' and 'user' access enabled/disabled */
void set_ss_pag(page_table_entry *PT, unsigned page,unsigned frame, int user)
{
	PT[page].entry=0;
	PT[page].bits.pbase_addr=frame;
	PT[page].bits.user=user;
	PT[page].bits.rw=1;
	PT[page].bits.present=1;

}

/* del_ss_pag - Removes mapping from logical page 'logical_page' */
void del_ss_pag(page_table_entry *PT, unsigned logical_page)
{
  PT[logical_page].entry=0;
}

/* get_frame - Returns the physical frame associated to page 'logical_page' */
unsigned int get_frame (page_table_entry *PT, unsigned int logical_page){
     return PT[logical_page].bits.pbase_addr; 
}

/* Mapea  automaticamente, la división decide q tabla se usa, el modulo elige sobre 1024  */
void set_kernel_pag(unsigned int frame)
{
    int table_index = frame / PAGE_TABLE_ENTRIES;
    int entry_index = frame % PAGE_TABLE_ENTRIES;
    
    if (table_index >= KERNEL_PAGE_TABLES) { // No deberia pasar
        return;
    }
    
    if (kernel_page_tables[table_index] == NULL) { // No deberia pasar
        return;
    }
    
    set_ss_pag(kernel_page_tables[table_index], entry_index, frame, 0);
}

struct shm_frame shm_frames[SHM_MAX_REGIONS];
int alloc_shm_frames() {
		for (int i = 0; i < SHM_MAX_REGIONS; i++) {
				int frame = alloc_frame();
				if (frame < 0) {
						for (--i; i >= 0; i--) 
								free_frame(shm_frames[i].frame);
						return -1;
				}

				struct shm_frame shared_frame;
				shared_frame.frame = frame;
				shared_frame.refcount = 0;
				shared_frame.pending_clear = 0;

				shm_frames[i] = shared_frame;
		}

		return 0;
}
