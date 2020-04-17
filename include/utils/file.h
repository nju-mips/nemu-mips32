#ifndef UTILS_FILE_H
#define UTILS_FILE_H

#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

/* file */
size_t get_file_size(const char *img_file);
void *read_file(const char *filename);
ssize_t read_s(int fd, const void *buf, size_t count);
ssize_t write_s(int fd, void *buf, size_t count);

uint32_t SDLKey_to_scancode(SDL_EventType type, SDLKey key);
const char *SDLKey_to_ascii(SDL_EventType type, SDLKey key);

#endif
