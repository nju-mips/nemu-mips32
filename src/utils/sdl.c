#include <SDL/SDL.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

#include "device.h"

SDL_Surface *screen;

static struct itimerval it;

extern void keyboard_enqueue(SDL_EventType, SDLKey);
static void device_update(int signum) {
  SDL_Event event = {0};
  SDL_PollEvent(&event);
  switch (event.type) {
  /* If a key was pressed */
  case SDL_KEYUP:
  case SDL_KEYDOWN:
    serial_enqueue_SDLKey(event.type, event.key.keysym.sym);
#if CONFIG_KEYBOARD
    keyboard_enqueue(event.type, event.key.keysym.sym);
#endif
    break;
  case SDL_QUIT:
    nemu_exit();
  default:
    /* do nothing */
    break;
  }

  int ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}

void sdl_clear_event_queue() {
  SDL_Event event;
  while (SDL_PollEvent(&event))
    ;
}

void init_sdl() {
  int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
  Assert(ret == 0, "SDL_Init failed");

  screen =
      SDL_SetVideoMode(WINDOW_W, WINDOW_H, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

  SDL_WM_SetCaption("NEMU-MIPS32", NULL);

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_handler = device_update;
  ret = sigaction(SIGVTALRM, &s, NULL);
  Assert(ret == 0, "Can not set signal handler");

  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 1000000 / TIMER_HZ;
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 0;
  ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}
