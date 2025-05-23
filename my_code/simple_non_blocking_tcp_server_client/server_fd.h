#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/poll.h>

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
  if (fds_buf.len == fds_buf.size) {
    struct pollfd *tmpfds =
        (struct pollfd *)realloc(fds_buf.fds, fds_buf.size * 2);
    if (!tmpfds) {
      return ENOMEM;
    }
    fds_buf.size *= 2;
    fds_buf.fds = tmpfds;
  }

  fds_buf.fds[fds_buf.len++] = *fd;

  return 0;
}

#include <stdio.h>
void remove_fd(int fd) {
  int i = fds_buf.len;
  printf("Starting remove_fd: fd->fd=%d, fds_buf.len=%d\n", fd, i);

  while (i-- > 0) {
    printf("Checking index %d\n", i);
    if (fd == fds_buf.fds[i].fd) {
      printf("Found matching fd at index %d\n", i);
      break;
    }
  }

  if (i <= 0) {
    printf("fd is at index 0 or not found, skipping removal.\n\n");
    return;
  }

  i++; // Set up i for overwrite
  printf("Shifting elements starting from index %d\n", i);

  for (; i < fds_buf.len; i++) {
    fds_buf.fds[i - 1] = fds_buf.fds[i];
    printf("Shifted fd from index %d to %d\n", i, i - 1);
  }

  fds_buf.len--;
  printf("Decremented fds_buf.len to %d\n", fds_buf.len);

  if ((fds_buf.len > fds_buf.size / 2) && (fds_buf.size % 2 == 0)) {
    printf("Attempting to shrink buffer from size %d to %d\n", fds_buf.size,
           fds_buf.size / 2);
    struct pollfd *tmpfds =
        (struct pollfd *)realloc(fds_buf.fds, fds_buf.size / 2);
    if (!tmpfds) {
      printf("realloc failed, keeping current buffer\n");
      return;
    }
    fds_buf.size /= 2;
    fds_buf.fds = tmpfds;
    printf("Buffer shrunk to new size %d\n", fds_buf.size);
  }

  printf("remove_fd completed successfully\n\n");
}

void __remove_fd(struct pollfd *fd) {
  int i = fds_buf.len;
  while (i-- > 0) {
    if (fd == &fds_buf.fds[i]) {
      break;
    }
  }

  printf("%d\n\n", i);
  // We cannot delete first file descriptor, this is our main socket
  if (i <= 0) {
    return;
  }

  i++; // Set up i for overwrite
  for (; i < fds_buf.len; i++) {
    fds_buf.fds[i - 1] = fds_buf.fds[i];
  }

  fds_buf.len--;

  if ((fds_buf.len > fds_buf.size / 2) && (fds_buf.size % 2 == 0)) {
    struct pollfd *tmpfds =
        (struct pollfd *)realloc(fds_buf.fds, fds_buf.size / 2);
    if (!tmpfds) {
      return;
    }
    fds_buf.size /= 2;
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
