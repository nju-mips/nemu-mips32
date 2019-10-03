#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

size_t get_file_size(const char *img_file) {
  struct stat file_status;
  lstat(img_file, &file_status);
  if (S_ISLNK(file_status.st_mode)) {
    char *buf = malloc(file_status.st_size + 1);
    size_t size = readlink(img_file, buf, file_status.st_size);
    buf[file_status.st_size] = 0;
    size = get_file_size(buf);
    free(buf);
    return size;
  } else {
    return file_status.st_size;
  }
}

void *read_file(const char *filename) {
  size_t size = get_file_size(filename);
  int fd = open(filename, O_RDONLY);
  if (fd == -1) return NULL;

  // malloc buf which should be freed by caller
  void *buf = malloc(size);
  int len = 0;
  while (len < size) { len += read(fd, buf, size - len); }
  return buf;
}
