#include "pd.h"
#include <rdma/rdma_verbs.h>
int alloc_pd(struct rdma_conn *conn) {
  int ret = 0;
  conn->pd = ibv_alloc_pd(conn->cm_id->verbs);
  if (conn->pd == NULL) {
    ret = -1;
    report_error(errno, "ibv_alloc_pd");
  }
  return ret;
}

int dealloc_pd(struct rdma_conn *conn) {
  int ret;
  ret = ibv_dealloc_pd(conn->pd);
  if (ret != 0) {
    report_error(errno, "ibv_dealloc_pd");
  }
  return ret;
}