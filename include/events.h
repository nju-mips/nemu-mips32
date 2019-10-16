#ifndef EVENTS_H
#define EVENTS_H

#include "device.h"

enum {
  EVENT_STDIN_DATA = 0,
  EVENT_SDL_KEY_DOWN = 1,
  EVENT_SDL_KEY_UP = 2,
  EVENT_TIMER = 3,
  EVENT_CTRL_C = 4,
  EVENT_CTRL_Z = 5,
  NR_EVENTS,
};

typedef void (*event_handler_t)(void *, int);

typedef struct event_t {
  int notify_queue_size;
  event_handler_t notify_queue[10];
} event_t;

void event_add_handler(int event_type, event_handler_t handler);

#endif
