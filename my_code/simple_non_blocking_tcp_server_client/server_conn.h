#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/queue.h>

struct Connection {
  int conn_fd;
  int timer_fd;
  STAILQ_ENTRY(Connection) _next;
};

STAILQ_HEAD(Connections, Connection);
struct Connections conns_queue;

void init_cons_queue(void) { STAILQ_INIT(&conns_queue); }

int insert_connection(struct Connection *conn) {
  struct Connection *tmp_conn =
      (struct Connection *)malloc(sizeof(struct Connection));
  if (!tmp_conn) {
    return ENOMEM;
  }

  *tmp_conn = *conn;

  STAILQ_INSERT_TAIL(&conns_queue, tmp_conn, _next);

  return 0;
}

void remove_connection(struct Connection *conn) {
  STAILQ_REMOVE(&conns_queue, conn, Connection, _next);
  free(conn);
}
