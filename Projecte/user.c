#include <libc.h>


int add(int par1, int par2) {return par1+par2;}
int addASM(int par1, int par2);

int latest_ticks = 0;

void clean_screen()
{
    for(int y = 0; y<25 ; y++)
	{
		for(int x = 0; x<80 ; x++)
		{
			write(1, " ", 1);
		}
	}
}

void wait(int loops)
{
    while(loops)
    {
        loops--;
    }
}

int getfps()
{
    int current_ticks = gettime();
    int ret = (latest_ticks - current_ticks)/18;
    latest_ticks = current_ticks;
    return ret;
}

void decorateScreen()
{
    int x, y;
    
    clean_screen();
    
    /* Configurar colores: cyan (6) para borde, negro (0) de fondo */
    set_color(6, 0);  /* fg=6 (cyan), bg=0 (black) */
    
    /* Línea superior: 80 iguales */
    gotoxy(0, 0);
    for(x = 0; x < 80; x++) {
        write(1, "=", 1);
    }
    
    /* Líneas 1-23 con bordes */
    for(y = 1; y < 24; y++) {
        gotoxy(0, y);
        write(1, "|", 1);
        
        /* Dibuja la cara grande en el medio (y=5 a y=12) */
        if(y == 5) {
            gotoxy(32, y);
            set_color(3, 0);  /* Yellow (3), black background */
            write(1, "_____________", 13);
            set_color(6, 0);  /* Back to cyan */
        }
        else if(y == 6) {
            gotoxy(31, y);
            set_color(3, 0);  /* Yellow */
            write(1, "/             \\", 15);
            set_color(6, 0);  /* Back to cyan */
        }
        else if(y == 7) {
            gotoxy(31, y);
            set_color(3, 0);  /* Yellow */
            write(1, "|  o       o  |", 15);
            set_color(6, 0);  /* Back to cyan */
        }
        else if(y == 8) {
            gotoxy(31, y);
            set_color(3, 0);  /* Yellow */
            write(1, "|     >       |", 15);
            set_color(6, 0);  /* Back to cyan */
        }
        else if(y == 9) {
            gotoxy(31, y);
            set_color(3, 0);  /* Yellow */
            write(1, "|   \\_____/   |", 15);
            set_color(6, 0);  /* Back to cyan */
        }
        else if(y == 10) {
            gotoxy(31, y);
            set_color(3, 0);  /* Yellow */
            write(1, "|  HAPPY! :D  |", 15);
            set_color(6, 0);  /* Back to cyan */
        }
        else if(y == 11) {
            gotoxy(31, y);
            set_color(3, 0);  /* Yellow */
            write(1, "|             |", 15);
            set_color(6, 0);  /* Back to cyan */
        }
        else if(y == 12) {
            gotoxy(32, y);
            set_color(3, 0);  /* Yellow */
            write(1, "\\___________/", 13);
            set_color(6, 0);  /* Back to cyan */
        }
        
        gotoxy(79, y);
        write(1, "|", 1);
    }
    
    /* Línea inferior: 80 iguales */
    gotoxy(0, 24);
    for(x = 0; x < 80; x++) {
        write(1, "=", 1);
    }
}




int __attribute__ ((__section__(".text.main")))
main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    //int result = add(66,1638);

	/*
		[x][x][x][][][][][][][][][][][][][][][]
		^                                    ^ 
		|                                    |
		WR 							         RD
	
	*/
    write(1,"\n",1);
    clean_screen();
    char buff[100];
    decorateScreen();
    char buffer[256];
    while(1){
        gotoxy(0,0);
        itoa(getfps(),buffer);
        write(1,buffer,strlen(buffer));
    }
}
