/*
 * mm.h - Capçalera del mòdul de gestió de memòria
 */

#ifndef __MM_H__
#define __MM_H__

#include <types.h>
#include <mm_address.h>

 
int init_frames( void );
int alloc_frame( void );
void free_frame( unsigned int frame );

void clear_page_table(page_table_entry* process_PT);
void set_user_pages(page_table_entry* process_PT);
void set_kernel_pages (page_table_entry* process_PT);
void free_user_pages(page_table_entry* process_PT);

extern Descriptor  *gdt;

extern TSS         tss; 

void init_mm();
void set_cr3(page_table_entry *dir);
void set_pe_flag();

void setGdt();

void setTSS();

void set_ss_pag(page_table_entry *PT, unsigned page,unsigned frame, int user);
void del_ss_pag(page_table_entry *PT, unsigned page);
unsigned int get_frame(page_table_entry *PT, unsigned int page);
void set_kernel_pag(unsigned int frame);
void init_kernel_page_tables(page_table_entry* pt0, page_table_entry* pt1);

extern page_table_entry* kernel_page_tables[KERNEL_PAGE_TABLES];

// M5. Shared memory
//
#define SHM_MAX_REGIONS 10
#define SHM_PAGES_START 28 

int alloc_shm_frames(void);

struct shm_frame {
		int frame;
		int refcount;
		int pending_clear;
};

extern struct shm_frame shm_frames[SHM_MAX_REGIONS];

#endif  /* __MM_H__ */
