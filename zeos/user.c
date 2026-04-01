#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2) {return par1+par2;}
int addASM(int par1, int par2);

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    //int result = add(66,1638);

	char *msg = "\nEsto es un mensaje.\n\n\n\n---------> THIS WAS PRINTED IN USER MODE <---------";
	write(1, msg, 76);
	//write(1,msg,-1); //Triggers error write
	char pid_buf[16];
	itoa(getpid(), pid_buf);
	msg = "PID = ";
	write(1,msg,7);
	write(1, pid_buf, 16);
  while(1) {
	
		/*
		int t = gettime();
		if (t >= 500) {
			write(1, msg, 16);
		}
		*/
	}
}
