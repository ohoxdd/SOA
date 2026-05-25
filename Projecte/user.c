#include <libc.h>

void clean_screen()
{
    for(int y = 0; y<25 ; y++)
        for(int x = 0; x<80 ; x++)
            write(1, " ", 1);
}

void wait(int loops)
{
    while(loops) loops--;
}

int __attribute__ ((__section__(".text.main")))
main(void)
{
    write(1,"\n",1);
    clean_screen();
    char buff[100];
    void *shm;
    void *ptrs[10];
    int ok;

    write(1, "\n=== MILESTONE 5: Shared Memory (shmat) ===\n", 44);

    /* ===== ERROR CASES ===== */
    write(1, "\n--- ERROR CASES ---\n", 20);

    /* EC1: id < 0 */
    shm = shmat(-1, 0);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(-1, 0) rejected (id<0)\n", 38);
    } else {
        write(1, "FAIL: shmat(-1, 0) should return -1\n", 40);
				return 0;
    }

    /* EC2: id >= SHM_MAX_REGIONS (10) */
    shm = shmat(10, 0);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(10, 0) rejected (id>=10)\n", 40);
    } else {
        write(1, "FAIL: shmat(10, 0) should return -1\n", 40);
				return 0;
    }

    /* EC3: addr not page-aligned */
    shm = shmat(0, (void*)0x81C001);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(0, non-aligned addr) rejected\n", 43);
    } else {
        write(1, "FAIL: shmat(0, non-aligned) should return -1\n", 46);
				return 0;
    }

    /* EC4: addr points to occupied user data (0x800000) */
    shm = shmat(0, (void*)0x800000);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(0, occupied data addr) rejected\n", 45);
    } else {
        write(1, "FAIL: shmat(0, occupied data addr) should return -1\n", 52);
				return 0;
    }

    /* EC5: addr points to occupied user code (0x814000) */
    shm = shmat(0, (void*)0x814000);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(0, occupied code addr) rejected\n", 45);
    } else {
        write(1, "FAIL: shmat(0, occupied code addr) should return -1\n", 52);
				return 0;
    }

    /* ===== SUCCESS CASES ===== */
    write(1, "\n--- SUCCESS CASES ---\n", 22);

    /* SC1: 0 addr → auto-assign region 0 */
    shm = shmat(0, 0);
    if ((int)shm >= 0) {
        write(1, "PASS: shmat(0, 0) -> ", 25);
        itoa((int)shm, buff);
        write(1, buff, strlen(buff));
        write(1, "\n", 1);
    } else {
        write(1, "FAIL: shmat(0, 0) should succeed\n", 37);
				return 0;
    }

    /* SC2: Explicit valid addr (free page at 0x81D000) */
    {
        void *explicit = shmat(1, (void*)0x81D000);
        if ((int)explicit == 0x81D000) {
            write(1, "PASS: shmat(1, 0x81D000) mapped at requested addr\n", 51);
        } else {
            write(1, "FAIL: shmat(1, 0x81D000) returned unexpected addr\n", 51);
						return 0;
        }
    }

    /* SC3: Write/readback on shared memory */
    {
        int *mem = (int *)shmat(2, 0);
        if ((int)mem < 0) {
            write(1, "FAIL: shmat(2, 0) failed\n", 29);
        } else {
            mem[0] = 0xDEAD;
            mem[1] = 0xBEEF;
            if (mem[0] == 0xDEAD && mem[1] == 0xBEEF)
                write(1, "PASS: write/readback on shared memory works\n", 46);
            else {
                write(1, "FAIL: write/readback mismatch\n", 31);
								return 0;
						}
        }
    }

    /* SC4: Attach same region twice in same process */
    {
        void *first  = shmat(3, 0);
        void *second = shmat(3, 0);
        if ((int)first >= 0 && (int)second >= 0 && first != second) {
            write(1, "PASS: same region attached twice at unique addrs\n", 50);
				}
        else if ((int)first >= 0 && (int)second >= 0)
            write(1, "INFO: same region twice returned same addr (ok)\n", 49);
        else {
            write(1, "FAIL: double attach of same region failed\n", 44);
						return 0;
				}
    }

    /* SC5: Attach all 10 regions, verify unique addrs */
    write(1, "\n--- Attaching all 10 regions ---\n", 35);
    ok = 1;
    for (int i = 0; i < 10; i++) {
        ptrs[i] = shmat(i, 0);
        if ((int)ptrs[i] < 0) {
            write(1, "FAIL: shmat(", 12);
            itoa(i, buff);
            write(1, buff, strlen(buff));
            write(1, ") failed\n", 9);
            ok = 0;
        }
    }
    if (ok) {
        for (int i = 0; i < 10 && ok; i++)
            for (int j = i+1; j < 10 && ok; j++)
                if (ptrs[i] == ptrs[j]) ok = 0;
        if (ok)
            write(1, "PASS: all 10 regions attached at unique addrs\n", 48);
        else {
            write(1, "FAIL: overlapping addresses among regions\n", 44);
						return 0;
				}
    }

    /* SC6: Shared memory across fork() */
    {
        int *shm_data = (int *)shmat(9, 0);
        shm_data[0] = 0;
        shm_data[1] = 0;

        int pid = fork();
        if (pid == 0) {
            /* Child: attach same shm id 9, modify */
            int *child_data = (int *)shmat(9, 0);
            child_data[0] = 0xCAFE;
            child_data[1] = 1;
            write(1, "child: wrote 0xCAFE, blocking...\n", 33);
            block();
        } else {
            /* Parent: spin until child sets flag */
            int spins = 0;
            while (shm_data[1] == 0 && spins < 20000000) spins++;
            unblock(pid);
            if (shm_data[0] == 0xCAFE)
                write(1, "PASS: parent sees child's 0xCAFE via shm\n", 42);
            else {
                write(1, "FAIL: parent sees wrong shm value after fork\n", 46);
								return 0;
						}
        }
    }

    write(1, "\n=== ALL SHARED MEMORY TESTS COMPLETE ===\n", 43);

    while(1){}
}
