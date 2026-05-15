/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

/** Screen functions **/
/**********************/

void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);
void set_fg(int new_fg);
void set_bg(int new_bg);
void set_xy(int new_x, int new_y);

#endif  /* __IO_H__ */
