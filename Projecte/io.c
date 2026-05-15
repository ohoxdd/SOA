/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

#include <hardware.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;
Byte fg=2,bg=0; 
//si bg >7 parpadea en el color del bg %7
//si fg >7, más colores
void printc(char c)
{
  bochs_out(c);
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word attr = ((Word)((bg&0x0F)<<4)|(fg&0x0F)) << 8; //0xXX00
    //esto nos pone el fg y bg como 0xbg.fg.F.F (las F son pq es el caracter, si fueran otro valor con el OR haria q se impriman otros chars)
    Word ch = (Word) (c & 0x00FF) | attr;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}

void set_fg(int new_fg)
{
  fg = new_fg;
}

void set_bg(int new_bg)
{
  bg = new_bg;
}

void set_xy(int new_x, int new_y)
{
  x = new_x;
  y = new_y;
}