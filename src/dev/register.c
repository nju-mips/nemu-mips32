#include "dev/device.h"

static device_t *device_list_head = NULL;

device_t *get_device_list_head() { return device_list_head; }

void register_device(device_t *dev) {
  dev->next = device_list_head;
  device_list_head = dev;
}
