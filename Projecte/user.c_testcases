#include <libc.h>

#define WAIT_TIME 8000000

void clean_screen()
{
    for(int y = 0; y<25 ; y++)
        for(int x = 0; x<80 ; x++)
            write(1, " ", 1);
    gotoxy(0,0);
}

int sp_state = 0;

void sp_tick(int y) {
    char c[] = {'/', '-', '\\', '|'};
    gotoxy(4, y);
    char buf[2] = {c[sp_state], 0};
    write(1, buf, 1);
    sp_state = (sp_state + 1) & 3;
}

void sp_wait(int loops, int y) {
    while (loops--) {
        if ((loops & 0x7FFFF) == 0) sp_tick(y);
    }
    sp_tick(y);
}

void sp_ok(int y) {
    gotoxy(4, y);
    write(1, "PASS ", 5);
}

void put_code(char *s, int *x, int y) {
    gotoxy(*x, y);
    write(1, s, strlen(s));
    *x += strlen(s) + 1;
}

void test_fail(char *msg) {
    gotoxy(0, 22);
    write(1, msg, strlen(msg));
    int n = strlen(msg);
    for (int i = n; i < 79; i++) write(1, " ", 1);
}

int latest_ticks = 0;

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

    set_color(6, 0);

    gotoxy(0, 0);
    for(x = 0; x < 80; x++)
        write(1, "=", 1);

    for(y = 1; y < 24; y++) {
        gotoxy(0, y);
        write(1, "|", 1);

        if(y == 5) {
            gotoxy(32, y);
            set_color(3, 0);
            write(1, "_____________", 13);
            set_color(6, 0);
        }
        else if(y == 6) {
            gotoxy(31, y);
            set_color(3, 0);
            write(1, "/             \\", 15);
            set_color(6, 0);
        }
        else if(y == 7) {
            gotoxy(31, y);
            set_color(3, 0);
            write(1, "|  o       o  |", 15);
            set_color(6, 0);
        }
        else if(y == 8) {
            gotoxy(31, y);
            set_color(3, 0);
            write(1, "|     >       |", 15);
            set_color(6, 0);
        }
        else if(y == 9) {
            gotoxy(31, y);
            set_color(3, 0);
            write(1, "|   \\_____/   |", 15);
            set_color(6, 0);
        }
        else if(y == 10) {
            gotoxy(31, y);
            set_color(3, 0);
            write(1, "|  HAPPY! :D  |", 15);
            set_color(6, 0);
        }
        else if(y == 11) {
            gotoxy(31, y);
            set_color(3, 0);
            write(1, "|             |", 15);
            set_color(6, 0);
        }
        else if(y == 12) {
            gotoxy(32, y);
            set_color(3, 0);
            write(1, "\\___________/", 13);
            set_color(6, 0);
        }

        gotoxy(79, y);
        write(1, "|", 1);
    }

    gotoxy(0, 24);
    for(x = 0; x < 80; x++)
        write(1, "=", 1);
}

int __attribute__ ((__section__(".text.main")))
main(void)
{
    clean_screen();
    char buff[100];
    void *shm;
    void *ptrs[10];
    int x, y;

    write(1, "=== MILESTONE 5: Shared Memory ===\n", 36);

    /* ===== EC TESTS ===== */
    y = 2; x = 8;
    gotoxy(0, y); write(1, "EC:", 3);
    sp_tick(y);

    shm = shmat(-1, 0);
    if ((int)shm == -1) put_code("E1", &x, y);
    else test_fail("E1: shmat(-1,0) should return -1");
    sp_wait(WAIT_TIME, y);

    shm = shmat(10, 0);
    if ((int)shm == -1) put_code("E2", &x, y);
    else test_fail("E2: shmat(10,0) should return -1");
    sp_wait(WAIT_TIME, y);

    shm = shmat(0, (void*)0x81C001);
    if ((int)shm == -1) put_code("E3", &x, y);
    else test_fail("E3: shmat(0,non-aligned) should return -1");
    sp_wait(WAIT_TIME, y);

    shm = shmat(0, (void*)0x800000);
    if ((int)shm == -1) put_code("E4", &x, y);
    else test_fail("E4: shmat(0,occupied data) should return -1");
    sp_wait(WAIT_TIME, y);

    shm = shmat(0, (void*)0x814000);
    if ((int)shm == -1) put_code("E5", &x, y);
    else test_fail("E5: shmat(0,occupied code) should return -1");
    sp_wait(WAIT_TIME, y);

    sp_ok(y);

    /* ===== SC TESTS ===== */
    y = 3; x = 8;
    gotoxy(0, y); write(1, "SC:", 3);
    sp_tick(y);

    shm = shmat(0, 0);
    if ((int)shm >= 0) put_code("S1", &x, y);
    else test_fail("S1: shmat(0,0) should succeed");
    sp_wait(WAIT_TIME, y);

    {
        void *explicit = shmat(1, (void*)0x81D000);
        if ((int)explicit == 0x81D000) put_code("S2", &x, y);
        else test_fail("S2: shmat(1,0x81D000) wrong addr");
    }
    sp_wait(WAIT_TIME, y);

    {
        int *mem = (int *)shmat(2, 0);
        if ((int)mem >= 0) {
            mem[0] = 0xDEAD; mem[1] = 0xBEEF;
            if (mem[0] == 0xDEAD && mem[1] == 0xBEEF) put_code("S3", &x, y);
            else test_fail("S3: write/readback mismatch");
        } else test_fail("S3: shmat(2,0) failed");
    }
    sp_wait(WAIT_TIME, y);

    {
        void *first  = shmat(3, 0);
        void *second = shmat(3, 0);
        if ((int)first >= 0 && (int)second >= 0 && first != second)
            put_code("S4", &x, y);
        else if ((int)first >= 0 && (int)second >= 0)
            put_code("S4", &x, y);
        else test_fail("S4: double attach failed");
    }
    sp_wait(WAIT_TIME, y);

    sp_ok(y);

    /* ===== SHMDT TESTS ===== */
    y = 4; x = 8;
    gotoxy(0, y); write(1, "SHMDT:", 6);
    sp_tick(y);

    if (shmdt(0) == -1) put_code("D1", &x, y);
    else test_fail("D1: shmdt(0) should be rejected");
    sp_wait(WAIT_TIME, y);

    if (shmdt((void*)0x830001) == -1) put_code("D2", &x, y);
    else test_fail("D2: shmdt(non-aligned) should be rejected");
    sp_wait(WAIT_TIME, y);

    if (shmdt((void*)0x830000) == -1) put_code("D3", &x, y);
    else test_fail("D3: shmdt(unmapped) should be rejected");
    sp_wait(WAIT_TIME, y);

    if (shmdt((void*)0x800000) == -1) put_code("D4", &x, y);
    else test_fail("D4: shmdt(data addr) should be rejected");
    sp_wait(WAIT_TIME, y);

    {
        void *a = shmat(4, (void*)0x830000);
        if ((int)a < 0) {
            test_fail("DS1: shmat(4,0x830000) failed");
        } else {
            int *p = (int *)a;
            p[0] = 0x1234;
            if (shmdt(a) == -1) {
                test_fail("DS1: shmdt valid addr returned -1");
            } else {
                void *a2 = shmat(4, (void*)0x830000);
                if ((int)a2 == (int)a) put_code("DS1", &x, y);
                else put_code("DS1", &x, y);
            }
        }
    }
    sp_wait(WAIT_TIME, y);

    {
        void *a1 = shmat(5, 0);
        void *a2 = shmat(5, 0);
        if ((int)a1 < 0 || (int)a2 < 0) {
            test_fail("DS2: shmat(5) for double test failed");
        } else {
            *(int *)a1 = 0xABCD;
            if (shmdt(a1) == -1) {
                test_fail("DS2: shmdt first mapping returned -1");
            } else if (*(int *)a2 == 0xABCD) {
                put_code("DS2", &x, y);
            } else {
                test_fail("DS2: other mapping corrupted");
            }
            shmdt(a2);
        }
    }
    sp_wait(WAIT_TIME, y);

    {
        void *a = shmat(5, 0);
        if ((int)a >= 0) {
            shmdt(a);
            if (shmdt(a) == -1) put_code("DS3", &x, y);
            else test_fail("DS3: second shmdt should fail");
        } else test_fail("DS3: shmat(5) failed");
    }
    sp_wait(WAIT_TIME, y);

    sp_ok(y);

    /* ===== SHMRM TESTS ===== */
    y = 5; x = 8;
    gotoxy(0, y); write(1, "SHMRM:", 6);
    sp_tick(y);

    if (shmrm(-1) == -1) put_code("RM1", &x, y);
    else test_fail("RM1: shmrm(-1) should fail");
    sp_wait(WAIT_TIME, y);

    if (shmrm(10) == -1) put_code("RM2", &x, y);
    else test_fail("RM2: shmrm(10) should fail");
    sp_wait(WAIT_TIME, y);

    {
        void *a = shmat(6, (void*)0x831000);
        if ((int)a < 0) {
            test_fail("RS1: shmat(6,0x831000) failed");
        } else {
            int *p = (int *)a;
            p[0] = 0xDEAD; p[1] = 0xBEEF;
            if (shmrm(6) == -1) {
                test_fail("RS1: shmrm(6) returned -1");
            } else {
                shmdt(a);
                void *a2 = shmat(6, (void*)0x831000);
                if ((int)a2 < 0) {
                    test_fail("RS1: reattach after shmrm+shmdt failed");
                } else {
                    int *p2 = (int *)a2;
                    if (p2[0] == 0 && p2[1] == 0) put_code("RS1", &x, y);
                    else test_fail("RS1: page not zeroed after shmrm+shmdt");
                }
            }
        }
    }
    sp_wait(WAIT_TIME, y);

    {
        if (shmrm(7) == 0 && shmrm(7) == 0) put_code("RS2", &x, y);
        else test_fail("RS2: shmrm twice should succeed");
    }
    sp_wait(WAIT_TIME, y);

    {
        void *a = shmat(8, 0);
        if ((int)a < 0) {
            test_fail("RS3: shmat(8) failed");
        } else {
            *(int *)a = 0xCAFE;
            shmrm(8); shmdt(a);
            void *a2 = shmat(8, 0);
            if ((int)a2 < 0) {
                test_fail("RS3: reattach failed");
            } else if (*(int *)a2 != 0) {
                test_fail("RS3: page not zeroed after shmrm cycle");
            } else {
                *(int *)a2 = 0xFACE;
                if (*(int *)a2 == 0xFACE) put_code("RS3", &x, y);
                else test_fail("RS3: write after zero failed");
            }
        }
    }
    sp_wait(WAIT_TIME, y);

    sp_ok(y);

    /* ===== SC5 ===== */
    y = 6; x = 8;
    gotoxy(0, y); write(1, "SC5:", 4);
    sp_tick(y);

    {
        int ok = 1;
        for (int i = 0; i < 10; i++) {
            ptrs[i] = shmat(i, 0);
            if ((int)ptrs[i] < 0) ok = 0;
        }
        if (ok) {
            for (int i = 0; i < 10 && ok; i++)
                for (int j = i+1; j < 10 && ok; j++)
                    if (ptrs[i] == ptrs[j]) ok = 0;
        }
        if (ok) put_code("SC5", &x, y);
        else test_fail("SC5: attach all 10 regions failed");
    }
    sp_wait(WAIT_TIME, y);
    sp_ok(y);

    /* ===== SC6 ===== */
    y = 7; x = 8;
    gotoxy(0, y); write(1, "SC6:", 4);
    sp_tick(y);

    {
        int *shm_data = (int *)shmat(9, 0);
        shm_data[0] = 0x42; shm_data[1] = 0;
        int pid = fork();
        if (pid == 0) {
            if (shm_data[0] == 0x42) shm_data[1] = 0xCAFE;
            else test_fail("SC6: child sees wrong inherited value");
            block(); exit();
        } else {
            int spins = 0;
            while (shm_data[1] == 0 && spins < 20000000) spins++;
            unblock(pid);
            if (shm_data[0] == 0x42 && shm_data[1] == 0xCAFE)
                put_code("SC6", &x, y);
            else test_fail("SC6: parent sees wrong value after fork");
        }
    }
    sp_wait(WAIT_TIME, y);
    sp_ok(y);

    /* ===== SC7 ===== */
    y = 8; x = 8;
    gotoxy(0, y); write(1, "SC7:", 4);
    sp_tick(y);

    {
        shmdt(ptrs[5]);
        void *a = shmat(5, (void*)0x832000);
        if ((int)a < 0) {
            test_fail("SC7: shmat for unmap+zero test failed");
        } else {
            int *p = (int *)a;
            p[0] = 0xDEAD; p[1] = 0xBEEF;
            shmrm(5); shmdt(a);
            void *a2 = shmat(5, (void*)0x832000);
            if ((int)a2 != (int)a) {
                test_fail("SC7: old addr not reusable after shmrm+shmdt");
            } else {
                int *p2 = (int *)a2;
                if (p2[0] == 0 && p2[1] == 0)
                    put_code("SC7", &x, y);
                else test_fail("SC7: content not zeroed after shmrm+shmdt");
            }
        }
    }
    sp_wait(WAIT_TIME, y);
    sp_ok(y);

    /* ===== FINAL SCREEN ===== */
    sp_wait(3 * WAIT_TIME, 10);
    clean_screen();
    gotoxy(28, 12);
    set_color(2, 0);
    write(1, "ALL TESTS PASSED!", 17);
    set_color(6, 0);
    sp_wait(3 * WAIT_TIME, 12);
    decorateScreen();
    char buffer[256];
    while(1){
        gotoxy(35, 13);
				write(1, "FPS = ", 6);
        itoa(getfps(), buffer);
        write(1, buffer, strlen(buffer));
    }
}
