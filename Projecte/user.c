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

	/*
		[x][x][x][][][][][][][][][][][][][][][]
		^                                    ^ 
		|                                    |
		WR 							         RD
	
	*/

	char *msg = "\nEsto es un mensaje.\n\n\n\n---------> THIS WAS PRINTED IN USER MODE <---------\n";
	write(1, msg, 76);
	//write(1,msg,-1); //Triggers error write
	int pid = fork(); 
	char ublockBuffer[32];
	//itoa(getpid(), pid_buf_alt);
	if(pid == 0) //hijo
	{
		write(1,"I'm child, with pid = ",23);
		msg = "";	
		itoa(getpid(),msg);
		write(1,msg,3);
		write(1," \n",3);
		block();
		write(1,"EXITED BLOCK\n\n",15);
		exit();
	}
	if(pid != 0)
	{
		//write(1,"CHECKED",8);	 
		write(1,"Process unblocked, return: ",28);
		int ublock = unblock(pid);
		itoa(ublock,ublockBuffer);
		write(1,ublockBuffer,4);
		pid = fork();
		if(pid == 0)
		{
			write(1,"I'm child, with pid = ",23);
			msg = "";
			itoa(getpid(),msg);
			write(1,msg,3);
			exit();
			write(1,"I will never be printed\n",25);
		}
		else
		{
			write(1,"\nI am the parent proc!\n",23);
		}
	}
	if(pid == 0)
	{
		exit();
	}
	write(1,"My PID is: ",12);
	msg = "";
	itoa(getpid(),msg);
	write(1,msg,4);

	
	while(1) {}
}
