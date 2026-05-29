#include <libc.h>

void clean_screen(void) {
  for (int y = 0; y < 25; y++)
    for (int x = 0; x < 80; x++)
      write(1, " ", 1);
  gotoxy(0, 0);
}

struct shm_game {
  volatile char last_key;
  char _pad[3];
  volatile int game_running;
};

#define KEY_UP 'w'
#define KEY_DOWN 's'
#define KEY_LEFT 'a'
#define KEY_RIGHT 'd'

#define MAX_SEG 100
#define MOVE_TICKS 8
#define VERTICAL_TICKS 14
#define FPS_INTERVAL 18

static struct {
  struct seg {
    int x, y;
  } segs[MAX_SEG];
  int head, tail, len;
  int dir_x, dir_y;
  int food_x, food_y;
  int score;
  int last_move, last_fps;
  int move_count;
  struct shm_game *gs;
} g;

void putc(int x, int y, char c, int fg, int bg) {
  set_color(fg, bg);
  gotoxy(x, y);
  write(1, &c, 1);
}

void pstr(int x, int y, char *s, int fg, int bg) {
  set_color(fg, bg);
  gotoxy(x, y);
  write(1, s, strlen(s));
}

void draw_walls(void) {
  int x, y;
  set_color(6, 0);
  for (x = 0; x < 80; x++) {
    gotoxy(x, 1);
    write(1, "#", 1);
  }
  for (x = 0; x < 80; x++) {
    gotoxy(x, 23);
    write(1, "#", 1);
  }
  for (y = 2; y < 23; y++) {
    gotoxy(0, y);
    write(1, "#", 1);
  }
  for (y = 2; y < 23; y++) {
    gotoxy(79, y);
    write(1, "#", 1);
  }
}

void draw_head(void) { putc(g.segs[g.head].x, g.segs[g.head].y, 'O', 2, 0); }

void erase_tail(void) {
  int t = g.tail;
  putc(g.segs[t].x, g.segs[t].y, ' ', 0, 0);
}

void draw_food(void) { putc(g.food_x, g.food_y, '$', 14, 0); }

void draw_stats(void) {
  char buf[20];
  set_color(15, 0);
  gotoxy(0, 0);
  write(1, "FPS:", 4);
  itoa(g.move_count, buf);
  write(1, buf, strlen(buf));
  write(1, " SCORE:", 7);
  itoa(g.score, buf);
  write(1, buf, strlen(buf));
  write(1, " LEN:", 5);
  itoa(g.len, buf);
  write(1, buf, strlen(buf));
  for (int i = 0; i < 50; i++)
    write(1, " ", 1);
  g.move_count = 0;
  g.last_fps = gettime();
}

int rng_seed = 42;

int rng(void) {
  rng_seed = (rng_seed * 1103515245 + 12345) & 0x7FFFFFFF;
  return rng_seed;
}

int body_at(int x, int y, int incl_tail) {
  int i = incl_tail ? g.tail : (g.tail + 1) % MAX_SEG;
  while (1) {
    if (g.segs[i].x == x && g.segs[i].y == y)
      return 1;
    if (i == g.head)
      break;
    i = (i + 1) % MAX_SEG;
  }
  return 0;
}

void place_food(void) {
  do {
    g.food_x = 1 + (rng() % 78);
    g.food_y = 2 + (rng() % 21);
  } while (body_at(g.food_x, g.food_y, 1));
  draw_food();
}

void game_over(void) {
  g.gs->game_running = 0;
  set_color(4, 0);
  gotoxy(33, 11);
  write(1, "GAME OVER", 9);
  gotoxy(31, 12);
  write(1, "Score: ", 7);
  char buf[10];
  itoa(g.score, buf);
  write(1, buf, strlen(buf));
  pstr(28, 14, "Prem R per reiniciar", 15, 0);
  pstr(24, 15, "o una altra tecla per sortir", 15, 0);
}

void move_snake(void) {
  struct seg *s = &g.segs[g.head];
  int nx = s->x + g.dir_x;
  int ny = s->y + g.dir_y;

  if (nx < 1 || nx > 78 || ny < 2 || ny > 22) {
    game_over();
    return;
  }

  int eating = (nx == g.food_x && ny == g.food_y);

  if (body_at(nx, ny, eating)) {
    game_over();
    return;
  }

  int nh = (g.head + 1) % MAX_SEG;
  g.segs[nh].x = nx;
  g.segs[nh].y = ny;
  g.head = nh;

  draw_head();

  int prev = (g.head - 1 + MAX_SEG) % MAX_SEG;
  if (prev != g.tail)
    putc(g.segs[prev].x, g.segs[prev].y, 'o', 2, 0);

  if (eating) {
    g.len++;
    g.score++;
    place_food();
  } else {
    erase_tail();
    g.tail = (g.tail + 1) % MAX_SEG;
  }
  g.move_count++;
  g.last_move = gettime();
}

void init_game(void) {
  rng_seed = gettime();
  clean_screen();
  draw_walls();

  g.head = 2;
  g.tail = 0;
  g.len = 3;
  g.dir_x = 1;
  g.dir_y = 0;
  g.segs[0].x = 10;
  g.segs[0].y = 12;
  g.segs[1].x = 11;
  g.segs[1].y = 12;
  g.segs[2].x = 12;
  g.segs[2].y = 12;
  g.score = 0;
  g.move_count = 0;

  putc(g.segs[1].x, g.segs[1].y, 'o', 2, 0);
  draw_head();
  place_food();

  g.last_move = gettime();
  g.last_fps = gettime();
  draw_stats();
}

void engine(void) {
  init_game();

  while (g.gs->game_running) {
    char key = g.gs->last_key;
    if (key) {
      g.gs->last_key = 0;
      if (key == KEY_UP && g.dir_y == 0) {
        g.dir_x = 0;
        g.dir_y = -1;
      }
      if (key == KEY_DOWN && g.dir_y == 0) {
        g.dir_x = 0;
        g.dir_y = 1;
      }
      if (key == KEY_LEFT && g.dir_x == 0) {
        g.dir_x = -1;
        g.dir_y = 0;
      }
      if (key == KEY_RIGHT && g.dir_x == 0) {
        g.dir_x = 1;
        g.dir_y = 0;
      }
    }

    int now = gettime();
    int move_ticks = g.dir_x != 0 ? MOVE_TICKS : VERTICAL_TICKS;
    if (now - g.last_move >= move_ticks) {
      move_snake();
      if (!g.gs->game_running)
        break;
    }

    if (now - g.last_fps >= FPS_INTERVAL)
      draw_stats();
  }
}

void input_handler(void) {
  char buf[1];
  while (1) {
    if (!g.gs->game_running) {
      if (buf[0] != 'r' && buf[0] != 'R')
        read(buf, 1);
      if (buf[0] == 'r' || buf[0] == 'R') {
        clean_screen();
        g.gs->last_key = 0;
        g.gs->game_running = 1;
        int pid = fork();
        if (pid == 0) { engine(); exit(); }
        continue;
      }
      exit();
    }
    read(buf, 1);
    if (!g.gs->game_running)
      continue;
    char c = buf[0];
    if (c == 'w' || c == 'a' || c == 's' || 'd')
      g.gs->last_key = c;
  }
}

int __attribute__((__section__(".text.main"))) main(void) {
  g.gs = (struct shm_game *)shmat(0, 0);
  g.gs->last_key = 0;
  g.gs->game_running = 0;

  clean_screen();
  pstr(20, 11, "WASD per moure's. Menja el $. No moris.", 15, 0);
  pstr(18, 12,
       "De veritat he d'explicar-te com jugar Snake?", 15, 0);
  pstr(24, 14, "Prem qualsevol tecla per comencar", 14, 0);

  char buf[1];
  read(buf, 1);

  clean_screen();
  g.gs->game_running = 1;

  int pid = fork();
  if (pid == 0) {
    engine();
    exit();
  }

  input_handler();
  return 0;
}
