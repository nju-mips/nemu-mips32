#include <SDL/SDL.h>
#include "device.h"
#include "nemu.h"
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

SDL_Surface *screen;

static uint64_t jiffy = 0;
static struct itimerval it;

extern void serial_enqueue(SDL_EventType, char);
extern void keyboard_enqueue(SDL_KeyboardEvent *evt);
extern void update_screen();

static void device_update(int signum) {
  jiffy ++;

  if(jiffy % (TIMER_HZ / VGA_HZ) == 0) {
	update_screen();
  }

  SDL_Event event;
  SDL_PollEvent(&event);
  switch(event.type) {
	// If a key was pressed
	case SDL_KEYUP:
	case SDL_KEYDOWN:
	  serial_enqueue(event.type, event.key.keysym.sym);
	  keyboard_enqueue(&(event.key));
	  break;
	case SDL_QUIT:
	  printf("[NEMU] receive SDL_QUIT, exit(0)\n");
	  exit(0);
	default:
	  // do nothing
	  break;
  }

  int ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}

void sdl_clear_event_queue() {
  SDL_Event event;
  while(SDL_PollEvent(&event));
}

void init_sdl() {
  int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
  Assert(ret == 0, "SDL_Init failed");

  screen = SDL_SetVideoMode(WINDOW_W, WINDOW_H, 32, 
	  SDL_HWSURFACE | SDL_DOUBLEBUF);

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


/////////////////////////////////////////////////////////////////
//                       dev simulation                        //
/////////////////////////////////////////////////////////////////
void init_device() {
  init_sdl();
}
