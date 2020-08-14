#ifndef UTILS_SDLKEY_H
#define UTILS_SDLKEY_H

#include <SDL/SDL.h>

uint32_t SDLKey_to_scancode(SDL_EventType type, SDLKey key);
const char *SDLKey_to_ascii(SDL_EventType type, SDLKey key);

#endif
