#include <libc.h>

#define WAIT_TIME 19999999

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

/*
int strlen(char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}
*/

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
    write(1, "\n--- ERROR CASES ---\n\n", 21);

    /* EC1: id < 0 */
    shm = shmat(-1, 0);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(-1, 0) rejected (id<0)\n", 38);
    } else 
        write(1, "FAIL: shmat(-1, 0) should return -1\n", 40);

		wait(WAIT_TIME);

    /* EC2: id >= SHM_MAX_REGIONS (10) */
    shm = shmat(10, 0);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(10, 0) rejected (id>=10)\n", 40);
    } else 
        write(1, "FAIL: shmat(10, 0) should return -1\n", 40);

		wait(WAIT_TIME);

    /* EC3: addr not page-aligned */
    shm = shmat(0, (void*)0x81C001);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(0, non-aligned addr) rejected\n", 43);
    } else 
        write(1, "FAIL: shmat(0, non-aligned) should return -1\n", 46);

		wait(WAIT_TIME);

    /* EC4: addr points to occupied user data (0x800000) */
    shm = shmat(0, (void*)0x800000);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(0, occupied data addr) rejected\n", 45);
    } else 
        write(1, "FAIL: shmat(0, occupied data addr) should return -1\n", 52);

		wait(WAIT_TIME);

    /* EC5: addr points to occupied user code (0x814000) */
    shm = shmat(0, (void*)0x814000);
    if ((int)shm == -1) {
        write(1, "PASS: shmat(0, occupied code addr) rejected\n", 45);
    } else 
        write(1, "FAIL: shmat(0, occupied code addr) should return -1\n", 52);

		wait(WAIT_TIME);

    /* ===== SUCCESS CASES ===== */
    write(1, "\n--- SUCCESS CASES ---\n", 22);

    /* SC1: 0 addr → auto-assign region 0 */
    shm = shmat(0, 0);
    if ((int)shm >= 0) {
        write(1, "PASS: shmat(0, 0) -> ", 25);
        itoa((int)shm, buff);
        write(1, buff, strlen(buff));
        write(1, "\n", 1);
    } else 
        write(1, "FAIL: shmat(0, 0) should succeed\n", 37);

		wait(WAIT_TIME);

    /* SC2: Explicit valid addr (free page at 0x81D000) */
		void *explicit = shmat(1, (void*)0x81D000);
		if ((int)explicit == 0x81D000) {
				write(1, "PASS: shmat(1, 0x81D000) mapped at requested addr\n", 51);
		} else 
				write(1, "FAIL: shmat(1, 0x81D000) returned unexpected addr\n", 51);
				

		wait(WAIT_TIME);

    /* SC3: Write/readback on shared memory */
		int *mem = (int *)shmat(2, 0);
		if ((int)mem < 0) {
				write(1, "FAIL: shmat(2, 0) failed\n", 29);
		} else {
				mem[0] = 0xDEAD;
				mem[1] = 0xBEEF;
				if (mem[0] == 0xDEAD && mem[1] == 0xBEEF)
						write(1, "PASS: write/readback on shared memory works\n", 46);
				else 
						write(1, "FAIL: write/readback mismatch\n", 31);
		}

		wait(WAIT_TIME);

    /* SC4: Attach same region twice in same process */
		void *first  = shmat(3, 0);
		void *second = shmat(3, 0);
		if ((int)first >= 0 && (int)second >= 0 && first != second) {
				write(1, "PASS: same region attached twice at unique addrs\n", 50);
		}
		else if ((int)first >= 0 && (int)second >= 0)
				write(1, "INFO: same region twice returned same addr (ok)\n", 49);
		else 
				write(1, "FAIL: double attach of same region failed\n", 44);
				

		wait(WAIT_TIME);

		clean_screen();
    write(1, "\n===== SHMDT TESTS =====\n", 25);

    /* EC_shmdt1: detach 0 addr */
    if (shmdt(0) == -1)
        write(1, "PASS: shmdt(0) rejected\n", 25);
    else
        write(1, "FAIL: shmdt(0) should return -1\n", 33);

		wait(WAIT_TIME);

    /* EC_shmdt2: non-aligned addr */
    if (shmdt((void*)0x830001) == -1)
        write(1, "PASS: shmdt(non-aligned) rejected\n", 35);
    else
        write(1, "FAIL: shmdt(non-aligned) should return -1\n", 43);

		wait(WAIT_TIME);

    /* EC_shmdt3: addr not mapped */
    if (shmdt((void*)0x830000) == -1)
        write(1, "PASS: shmdt(unmapped addr) rejected\n", 37);
    else
        write(1, "FAIL: shmdt(unmapped addr) should return -1\n", 45);

		wait(WAIT_TIME);

    /* EC_shmdt4: addr occupied by user data, not shm */
    if (shmdt((void*)0x800000) == -1)
        write(1, "PASS: shmdt(data addr) rejected\n", 33);
    else
        write(1, "FAIL: shmdt(data addr) should return -1\n", 41);

		wait(WAIT_TIME);

    /* SC_shmdt1: attach, write, detach, verify gone (reattach succeeds) */
    {
        void *a = shmat(4, (void*)0x830000);
        if ((int)a < 0) {
            write(1, "FAIL: shmat(4, 0x830000) for shmdt test failed\n", 48);
        } else {
            int *p = (int *)a;
            p[0] = 0x1234;
            if (shmdt(a) == -1)
                write(1, "FAIL: shmdt valid addr returned -1\n", 36);
            else {
                void *a2 = shmat(4, (void*)0x830000);
                if ((int)a2 == (int)a)
                    write(1, "PASS: shmat after shmdt reuses same addr\n", 43);
                else
                    write(1, "INFO: shmat after shmdt used diff addr (still ok)\n", 51);
            }
        }
    }

		wait(WAIT_TIME);
    /* SC_shmdt2: double attach, detach one, other still accessible */
    {
        void *a1 = shmat(5, 0);
        void *a2 = shmat(5, 0);
        if ((int)a1 < 0 || (int)a2 < 0) {
            write(1, "FAIL: shmat(5) for double test failed\n", 39);
        } else {
            *(int *)a1 = 0xABCD;
            int ret = shmdt(a1);
            if (ret == -1)
                write(1, "FAIL: shmdt first mapping returned -1\n", 39);
            else if (*(int *)a2 == 0xABCD)
                write(1, "PASS: other mapping still valid after shmdt\n", 45);
            else
                write(1, "FAIL: other mapping corrupted after shmdt\n", 43);
            shmdt(a2);
        }
    }

		wait(WAIT_TIME);
    /* SC_shmdt3: double detach — second should fail */
    {
        void *a = shmat(5, 0);
        if ((int)a >= 0) {
            shmdt(a);
            if (shmdt(a) == -1)
                write(1, "PASS: second shmdt on same addr rejected\n", 43);
            else
                write(1, "FAIL: second shmdt should return -1\n", 37);
        }
    }

		wait(WAIT_TIME);
		clean_screen();
    write(1, "\n===== SHMRM TESTS =====\n", 25);

    /* EC_shmrm1: id < 0 */
    if (shmrm(-1) == -1)
        write(1, "PASS: shmrm(-1) rejected\n", 26);
    else
        write(1, "FAIL: shmrm(-1) should return -1\n", 34);

		wait(WAIT_TIME);
    /* EC_shmrm2: id >= SHM_MAX_REGIONS */
    if (shmrm(10) == -1)
        write(1, "PASS: shmrm(10) rejected\n", 26);
    else
        write(1, "FAIL: shmrm(10) should return -1\n", 34);

		wait(WAIT_TIME);
    /* SC_shmrm1: shmrm → last shmdt zeroes content */
    {
        void *a = shmat(6, (void*)0x831000);
        if ((int)a < 0) {
            write(1, "FAIL: shmat(6, 0x831000) for shmrm test failed\n", 48);
        } else {
            int *p = (int *)a;
            p[0] = 0xDEAD;
            p[1] = 0xBEEF;

            int r = shmrm(6);
            if (r == -1) {
                write(1, "FAIL: shmrm(6) returned -1\n", 28);
            } else {
                shmdt(a);
                void *a2 = shmat(6, (void*)0x831000);
                if ((int)a2 < 0) {
                    write(1, "FAIL: shmat(6) after shmrm+shmdt failed\n", 41);
                } else {
                    int *p2 = (int *)a2;
                    if (p2[0] == 0 && p2[1] == 0)
                        write(1, "PASS: shm page zeroed after shmrm+last shmdt\n", 46);
                    else
                        write(1, "FAIL: shm page not zeroed after shmrm+last shmdt\n", 49);
                }
            }
        }
    }

		wait(WAIT_TIME);
    /* SC_shmrm2: shmrm idempotent — call twice */
    {
        if (shmrm(7) == 0 && shmrm(7) == 0)
            write(1, "PASS: shmrm twice on same id returns 0\n", 40);
        else
            write(1, "FAIL: shmrm twice should both succeed\n", 39);
    }

		wait(WAIT_TIME);
    /* SC_shmrm3: full cycle: attach → write → shmrm → detach → attach → verify zeroed → write new → verify */
    {
        void *a = shmat(8, 0);
        if ((int)a < 0) {
            write(1, "FAIL: shmat(8) for cycle test\n", 31);
        } else {
            *(int *)a = 0xCAFE;
            shmrm(8);
            shmdt(a);

            void *a2 = shmat(8, 0);
            if ((int)a2 < 0) {
                write(1, "FAIL: shmat(8) after cycle failed\n", 35);
            } else if (*(int *)a2 != 0) {
                write(1, "FAIL: page not zeroed after shmrm+detach cycle\n", 48);
            } else {
                *(int *)a2 = 0xFACE;
                if (*(int *)a2 == 0xFACE)
                    write(1, "PASS: full shmrm cycle: zeroed then writable\n", 47);
                else
                    write(1, "FAIL: write after zero failed\n", 31);
            }
        }
    }

		wait(WAIT_TIME);
		clean_screen();
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
        else
            write(1, "FAIL: overlapping addresses among regions\n", 44);
    }

		wait(WAIT_TIME);
    /* SC6: Shared memory across fork() */
    {
        int *shm_data = (int *)shmat(9, 0);
        shm_data[0] = 0;
        shm_data[1] = 0;

        int pid = fork();
        if (pid == 0) {
            int *child_data = (int *)shmat(9, 0);
            child_data[0] = 0xCAFE;
            child_data[1] = 1;
            write(1, "child: wrote 0xCAFE, blocking...\n", 33);
						exit();
        } else {
            int spins = 0;
            while (shm_data[1] == 0 && spins < 20000000) spins++;
            unblock(pid);
            if (shm_data[0] == 0xCAFE)
                write(1, "PASS: parent sees child's 0xCAFE via shm\n", 42);
            else
                write(1, "FAIL: parent sees wrong shm value after fork\n", 46);
        }
    }

    write(1, "\n=== ALL SHMAT + SHMDT + SHMRM TESTS COMPLETE ===\n", 49);

    while(1){}
}
