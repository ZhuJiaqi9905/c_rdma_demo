#include "rdma_conn.h"
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

struct rdma_conn *new_rdma_conn(int region_len) {
  struct rdma_conn *conn;
  conn = (struct rdma_conn *)malloc(sizeof(struct rdma_conn));
  if (conn == NULL) {
    return NULL;
  }
  conn->region_len = region_len;
  conn->region = (uint8_t *)calloc(region_len, 1);
  if (conn->region == NULL) {
    drop_rdma_conn(conn);
    return NULL;
  }
  return conn;
}
void drop_rdma_conn(struct rdma_conn *conn) {
  free(conn->region);
  free(conn);
}
