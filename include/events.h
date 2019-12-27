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
  EVENT_PACKET_IN = 6,
  NR_EVENTS,
};

typedef int (*event_handler_t)(const void *, int);

typedef struct event_t {
  int notify_queue_size;
  event_handler_t handler;
} event_t;

void event_bind_handler(int event_type, event_handler_t handler);
int notify_event(int event_type, void *data, int len);

#endif
