#ifndef UTILS_FILE_H
#define UTILS_FILE_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

/* file */
size_t get_file_size(const char *img_file);
void *read_file(const char *filename);
ssize_t read_s(int fd, const void *buf, size_t count);
ssize_t write_s(int fd, void *buf, size_t count);

#endif
