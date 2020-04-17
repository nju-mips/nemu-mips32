#include <SDL/SDL.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "device.h"
#include "utils/console.h"

SDL_Surface *screen;
static struct itimerval it;

void update_irq(int signum) {
  SDL_Event evt = {0};
  int cnt = SDL_PeepEvents(
      &evt, 1, SDL_GETEVENT, SDL_EVENTMASK(SDL_QUIT));
  if (cnt > 0) nemu_exit();

  for (device_t *dev = get_device_list_head(); dev;
       dev = dev->next) {
    if (dev->update_irq) { dev->update_irq(); }
  }

  /* TIMER */
  int ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}

void init_timer() {
  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_handler = update_irq;
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
  screen = SDL_SetVideoMode(WINDOW_W, WINDOW_H, 32,
      SDL_HWSURFACE | SDL_DOUBLEBUF);
  SDL_WM_SetCaption("NEMU-MIPS32", NULL);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
      SDL_DEFAULT_REPEAT_INTERVAL);
}

void init_events() {
#if CONFIG_GRAPHICS
  init_sdl();
#endif
  init_console();

  init_timer();
}
