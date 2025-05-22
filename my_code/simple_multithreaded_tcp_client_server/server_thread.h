#include <asm-generic/errno-base.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct HandleConnectionTask {
  int accepted_fd;
  STAILQ_ENTRY(HandleConnectionTask) _next;
};

STAILQ_HEAD(task_queue_t, HandleConnectionTask);
struct task_queue_t task_queue;

pthread_mutex_t lock;
pthread_cond_t cond;

void wakeup_threads() {
  pthread_mutex_lock(&lock);
  pthread_cond_signal(&cond); // wakes the thread
  pthread_mutex_unlock(&lock);
}

/* static int thread_id = 0; */

void *thread_func(void *_) {
  (void)_;
  bool do_loop = true;
  while (do_loop) {
    pthread_mutex_lock(&lock);
    if (STAILQ_EMPTY(&task_queue)) {
      pthread_cond_wait(&cond, &lock); // sleeps here
    }
    struct HandleConnectionTask *current_task = STAILQ_FIRST(&task_queue);

    STAILQ_REMOVE_HEAD(&task_queue, _next);

    int current_task_id = current_task->accepted_fd;

    pthread_mutex_unlock(&lock);
    // do work

    printf("Starting task %d for %d...\n", current_task_id,
           current_task->accepted_fd);

    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));

    ssize_t read_bytes =
        recv(current_task->accepted_fd, buffer, sizeof(buffer) - 1, 0);
    if (read_bytes > 0) {
      printf("TASK<%d> --- Read %d bytes: %.*s\n", current_task_id,
             (int)read_bytes, (int)read_bytes, buffer);
      if (strstr(buffer, "ping")) {
        ssize_t write_bytes =
            send(current_task->accepted_fd, "pong", strlen("pong"), 0);
        if (write_bytes > 0) {
          printf("TASK<%d> --- Write %d bytes: %s\n", current_task_id,
                 (int)write_bytes, "pong");
        } else if (write_bytes == 0) {
          printf("TASK<%d> --- Connection closed by peer\n", current_task_id);
          shutdown(current_task->accepted_fd, SHUT_RDWR);
          close(current_task->accepted_fd);
          free(current_task);
          continue;
        } else {
          perror("send failed");
          shutdown(current_task->accepted_fd, SHUT_RDWR);
          close(current_task->accepted_fd);
          free(current_task);
          continue;
        }
      }
    } else if (read_bytes == 0) {
      printf("TASK<%d> --- Connection closed by peer\n", current_task_id);
      shutdown(current_task->accepted_fd, SHUT_RDWR);
      close(current_task->accepted_fd);
      free(current_task);
      continue;
    } else {
      perror("recv failed");
      shutdown(current_task->accepted_fd, SHUT_RDWR);
      close(current_task->accepted_fd);
      free(current_task);
      continue;
    }

    pthread_mutex_lock(&lock);
    STAILQ_INSERT_TAIL(&task_queue, current_task, _next);
    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

int create_threads(int threads_num) {
  pthread_t thread;
  int err;
  while (threads_num-- > 0) {
    err = pthread_create(&thread, NULL, thread_func, NULL);
    if (err) {
      return err;
    }

    pthread_detach(thread);
  }

  return 0;
}

int insert_task(struct HandleConnectionTask task) {
  pthread_mutex_lock(&lock);

  struct HandleConnectionTask *queue_task =
      (struct HandleConnectionTask *)malloc(
          sizeof(struct HandleConnectionTask));
  if (!queue_task) {
    return ENOMEM;
  }

  *queue_task = task;

  STAILQ_INSERT_TAIL(&task_queue, queue_task, _next);

  pthread_mutex_unlock(&lock);

  return 0;
}

void init_task_queue(void) {
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&cond, NULL);
  STAILQ_INIT(&task_queue);
}
