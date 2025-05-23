#include <errno.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>

struct FdBuffer {
  int size;
  int len;
  struct pollfd *fds;
};

struct FdBuffer fds_buf = {0};

int init_fds_queue(void) {
  if (fds_buf.fds) {
    return 0;
  }

  fds_buf.len = 0;
  fds_buf.size = 16;
  fds_buf.fds = (struct pollfd *)malloc(sizeof(struct pollfd) * fds_buf.size);
  if (!fds_buf.fds) {
    return ENOMEM;
  }

  return 0;
}

int insert_fd(struct pollfd *fd) {
  if (fds_buf.len == fds_buf.size - 1) {
    struct pollfd *tmpfds = (struct pollfd *)realloc(
        fds_buf.fds, fds_buf.size * 2 * sizeof(struct pollfd));
    if (!tmpfds) {
      return ENOMEM;
    }
    fds_buf.size *= 2;
    fds_buf.fds = tmpfds;
  }

  fds_buf.fds[fds_buf.len++] = *fd;

  return 0;
}

void remove_fd(int fd) {
  int i = fds_buf.len;
  while (i-- > 0) {
    if (fd == fds_buf.fds[i].fd) {
      break;
    }
  }

  // We cannot delete first file descriptor, this is our main socket
  if (i <= 0) {
    return;
  }

  close(fd);

  i++; // Set up i for overwrite
  for (; i < fds_buf.len; i++) {
    fds_buf.fds[i - 1] = fds_buf.fds[i];
  }

  fds_buf.len--;

  uint32_t new_size = (fds_buf.size * sizeof(struct pollfd));
  if ((fds_buf.len > new_size / 2) && (new_size % 2 == 0)) {
    struct pollfd *tmpfds = (struct pollfd *)realloc(fds_buf.fds, new_size / 2);
    if (!tmpfds) {
      return;
    }
    fds_buf.size = new_size / 2;
    fds_buf.fds = tmpfds;
  }
}

int mypoll(void) {
  return poll(fds_buf.fds, fds_buf.len, -1); // Wait indefinitely
}

struct pollfd *find_fd(int fd) {
  for (int i = 0; i < fds_buf.len; i++) {
    if (fds_buf.fds[i].fd == fd) {
      return &fds_buf.fds[i];
    }
  }

  return NULL;
}
