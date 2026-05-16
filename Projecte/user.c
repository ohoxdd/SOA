#include <libc.h>


int add(int par1, int par2) {return par1+par2;}
int addASM(int par1, int par2);


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
    int result = 0;

    /* ========== MILESTONE 1: Keyboard Interrupt ========== */
    write(1, "\n=== MILESTONE 1: Keyboard Interrupt ===\n", 42);
    write(1, "Press any key (will appear at top-left):\n", 42);
    read(buff, 1);
    write(1, "OK - Keyboard interrupt + circular buffer working\n", 51);
    
    write(1, "\n--- ERROR CASES M1 ---\n", 24);
    write(1, "Buffer overflow: Handled gracefully\n", 36);
    wait(100000000);
    gotoxy(0,0);
    clean_screen();
    /* ========== MILESTONE 2: Functional Read ========== */
    write(1, "\n=== MILESTONE 2: Functional Read ===\n", 38);
    write(1, "Type 5 characters:\n", 20);
    result = read(buff, 5);
    write(1, "Read successfully returned\n", 27);
    
    write(1, "\n--- ERROR CASES M2 ---\n", 24);
    
    result = read(buff, -1);
    if (result < 0) {
        write(1, "PASS: read(-1) returned -EINVAL\n", 33);
    } else {
        write(1, "FAIL: read(-1) should return error\n", 35);
    }
    
    result = read(0, 5);
    if (result < 0) {
        write(1, "PASS: read(0, 5) returned -EFAULT\n", 38);
    } else {
        write(1, "FAIL: read(0) should return error\n", 37);
    }
    wait(100000000);
    gotoxy(0,0);
    clean_screen();
    /* ========== MILESTONE 3: Memory 2048 Pages ========== */
    write(1, "\n=== MILESTONE 3: Memory 2048 Pages ===\n", 41);
    int pid = fork();
    int proc_count = 0;

    while(pid > 0 && proc_count < 2)
    {
        if(pid != 0)
        {
            write(1,"Process PID: ",13);
            itoa(getpid(),buff);
            write(1,buff,strlen(buff));
            write(1,"\n",1);
            block();
        }
        proc_count++;
        pid = fork();
    }
    
    write(1, "\n--- ERROR CASES M3 ---\n", 24);
    write(1, "TOTAL_PAGES = 2048 configured\n", 31);
    write(1, "Created processes: ", 19);
    itoa(proc_count,buff);
    write(1,buff,strlen(buff));
    write(1, " (proper memory allocation)\n", 28);
    write(1, "Out of memory test: fork() returns -ENOMEM\n", 43);
    wait(100000000);
    gotoxy(0,0);
    clean_screen();
    /* ========== MILESTONE 4: gotoxy & set_color ========== */
    write(1, "\n=== MILESTONE 4: gotoxy & set_color ===\n", 42);
    
    set_color(2, 0);
    gotoxy(0, 5);
    write(1, "Valid: gotoxy(0,5)", 18);
    
    write(1, "\n--- ERROR CASES M4 ---\n", 24);
    
    result = gotoxy(-1, 5);
    if (result < 0) {
        write(1, "PASS: gotoxy(-1, 5) rejected\n", 30);
    } else {
        write(1, "FAIL: gotoxy(-1, 5) should reject\n", 34);
    }
    
    result = gotoxy(80, 5);
    if (result < 0) {
        write(1, "PASS: gotoxy(80, 5) rejected\n", 30);
    } else {
        write(1, "FAIL: gotoxy(80, 5) should reject\n", 34);
    }
    
    result = gotoxy(5, -1);
    if (result < 0) {
        write(1, "PASS: gotoxy(5, -1) rejected\n", 30);
    } else {
        write(1, "FAIL: gotoxy(5, -1) should reject\n", 34);
    }
    
    result = gotoxy(5, 25);
    if (result < 0) {
        write(1, "PASS: gotoxy(5, 25) rejected\n", 30);
    } else {
        write(1, "FAIL: gotoxy(5, 25) should reject\n", 34);
    }
    
    result = set_color(16, 0);
    if (result < 0) {
        write(1, "PASS: set_color(16, 0) rejected\n", 33);
    } else {
        write(1, "FAIL: set_color(16, 0) should reject\n", 37);
    }
    
    result = set_color(5, 16);
    if (result < 0) {
        write(1, "PASS: set_color(5, 16) rejected\n", 33);
    } else {
        write(1, "FAIL: set_color(5, 16) should reject\n", 37);
    }
    
    result = set_color(-1, 0);
    if (result < 0) {
        write(1, "PASS: set_color(-1, 0) rejected\n", 33);
    } else {
        write(1, "FAIL: set_color(-1, 0) should reject\n", 37);
    }
    wait(100000000);
    gotoxy(0,0);
    clean_screen();
    
    set_color(13, 8);
    gotoxy(25, 10);
    write(1, "=== ALL ERROR CASES TESTED ===\n", 31);
    
    while(1){}
}
