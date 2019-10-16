#ifndef EVENTS_H
#define EVENTS_H

#include "device.h"

enum {
  EVENT_STDIN_DATA = 0,
  EVENT_SDL_KEY_DOWN = 1,
  EVENT_SDL_KEY_UP = 2,
  EVENT_TIMER = 3,
  NR_EVENTS,
};

typedef struct event_t {
  int notify_queue_size;
  device_t *notify_queue[10];
} event_t;

void device_bind_event(device_t *dev, int event_type);

#endif
