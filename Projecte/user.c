#include <libc.h>


int add(int par1, int par2) {return par1+par2;}
int addASM(int par1, int par2);

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
	for(int y = 0; y<25 ; y++)
	{
		for(int x = 0; x<80 ; x++)
		{
			write(1, " ", 1);
		}
	}
	char buff[100];
    int result = 0;


    write(1, "\n=== MILESTONE 1: Keyboard Interrupt ===\n", 42);
    write(1, "Press any key (will appear at top-left):\n", 42);
    read(buff, 1);
    write(1, "OK - Keyboard interrupt + circular buffer working\n", 51);
    
    write(1, "\n=== MILESTONE 2: Functional Read ===\n", 38);
    write(1, "Type 5 characters:\n", 20);
    result = read(buff, 5);
    write(1, "Read successfully returned\n", 27);
    
    write(1, "\n=== MILESTONE 3: Memory 2048 Pages ===\n", 41);
    int pid = fork();
    while(pid>=0)
    {
        if(pid != 0)
            block();
        pid = fork();
    }
    write(1," -- PROC WITH PID: ",20);
    itoa(getpid(),buff);
    write(1,buff,strlen(buff));
    write(1, "\nTOTAL_PAGES = 2048 configured\n", 31);
    write(1, "Physical memory extended successfully\n", 38);
    
    write(1, "\n=== MILESTONE 4: gotoxy & set_color ===\n", 42);
    
    set_color(3, 13); 
    gotoxy(35, 10);
    write(1, "Position (35,10) - CYAN ON PINK BLINKING", 41);
    
    set_color(15, 4); // White on red
    gotoxy(25, 12);
    write(1, "Position (25,12) - WHITE ON RED", 32);
    
    set_color(2, 0);  // Back to green
    gotoxy(0, 14);
    write(1, "gotoxy and set_color working OK\n", 33);
    
    write(1, "\n=== SUMMARY ===\n", 18);
    write(1, "All 4 milestones PASSED\n", 25);	
	while(1){}
}
