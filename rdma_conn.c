#include "rdma_common.h"
#include <ctype.h>
#include <errno.h>
#include <infiniband/arch.h>
#include <inttypes.h>
#include <netdb.h>
#include <rdma/rdma_cma.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

struct rdma_conn *create_rdma_conn() {
  struct rdma_conn *conn;
  conn = (struct rdma_conn *)malloc(sizeof(struct rdma_conn));
  return conn;
}
void destroy_rdma_conn(struct rdma_conn *conn) { free(conn); }
