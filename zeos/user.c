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
	msg = "PID = ";
	write(1,msg,7);
	int pid = fork(); 
	int checkH = 0, checkP = 0;
	int n = 0;
	char ublockBuffer[32];

	while(1) {
		n++;
		//itoa(getpid(), pid_buf_alt);
		if(pid == 0) //hijo
		{
			write(1,"WORKED\n\n\n\n\n",12);
			checkH = 1;
			block();
			write(1,"EXITED BLOCK :))\n\n",19);
		}
		if(pid != 0 && checkP == 0)
		{
			write(1,"CHECKED",8);
			checkP = 1;
		}
		if(pid != 0 && n%10000000 == 0)
		{
			int ublock = unblock(pid);
			itoa(ublock,ublockBuffer);
			write(1,"Process unblock return: ",25);
			write(1,ublockBuffer,4);
		}
	}
}
