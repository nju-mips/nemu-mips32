#include <SDL/SDL.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "device.h"
#include "events.h"
#include "utils.h"

SDL_Surface *screen;
static struct itimerval it;

static event_t events[NR_EVENTS];

void event_bind_handler(int event_type, event_handler_t handler) {
  assert(handler);
  assert(0 <= event_type && event_type < NR_EVENTS);

  event_t *evt = &events[event_type];
  assert(!evt->handler);
  evt->handler = handler;
}

int notify_event(int event_type, void *data, int len) {
  assert(0 <= event_type && event_type < NR_EVENTS);

  event_t *evt = &events[event_type];
  if (!evt->handler) return -1;
  return evt->handler(data, len);
}

bool detect_sdl_event() {
  SDL_Event event = {0};
  if (!SDL_PollEvent(&event)) return false;

  int sdlk_data[2] = {event.type, event.key.keysym.sym};
  switch (event.type) {
  /* If a key was pressed */
  case SDL_KEYUP:
    notify_event(EVENT_SDL_KEY_UP, sdlk_data, sizeof(sdlk_data));
    break;
  case SDL_KEYDOWN:
    notify_event(EVENT_SDL_KEY_DOWN, sdlk_data, sizeof(sdlk_data));
    break;
  case SDL_QUIT: nemu_exit();
  default:
    /* do nothing */
    break;
  }
  return true;
}

bool detect_stdin() {
  /* read stdin */
  int n = nchars_stdin();
  if (n > 0) {
    char *buf = malloc(n);
    int ret = read(0, buf, n);
    assert(ret == n);

    notify_event(EVENT_STDIN_DATA, buf, n);

    free(buf);
  }
  return n > 0;
}

void update_timer() {
  /* TIMER */
  int ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");

  notify_event(EVENT_TIMER, NULL, 0);
}

void detect_event(int signum) {
  detect_sdl_event();
  detect_stdin();
  update_timer();
}

#if 0
static void ctrl_code_handler(int no) {
  if (no == SIGINT) {
    /* https://en.wikipedia.org/wiki/Control-C */
    char data = '\x03';
    notify_event(EVENT_CTRL_C, &data, 1);
  } else if (no == SIGTSTP) {
    /* https://en.wikipedia.org/wiki/Substitute_character */
    char data = '\x1a';
    notify_event(EVENT_CTRL_Z, &data, 1);
  }
}
#endif

void init_timer() {
  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_handler = detect_event;
  int ret = sigaction(SIGVTALRM, &s, NULL);
  Assert(ret == 0, "Can not set signal handler");

  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 1000000 / TIMER_HZ;
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}

void init_sdl() {
  /* sdl */
  int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
  Assert(ret == 0, "SDL_Init failed");
  screen =
      SDL_SetVideoMode(WINDOW_W, WINDOW_H, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
  SDL_WM_SetCaption("NEMU-MIPS32", NULL);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

void init_events() {
#if CONFIG_GRAPHICS
  init_sdl();
#endif
  init_console();

  init_timer();

#if 0
  signal(SIGINT, ctrl_code_handler);
  signal(SIGTSTP, ctrl_code_handler);
#endif
}
