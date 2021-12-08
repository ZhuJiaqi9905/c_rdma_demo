#include "rdma_conn.h"

int create_cq(struct rdma_conn *conn, int cqe) {
  int ret = 0;
  conn->cq = ibv_create_cq(conn->cm_id->verbs, cqe, NULL, NULL, 0);
  if (conn->cq == NULL) {
    report_error(errno, "ibv_create_cq");
    ret = -1;
  }
  return ret;
}

int destroy_cq(struct rdma_conn *conn) {
  int ret;
  ret = ibv_destroy_cq(conn->cq);
  if (ret != 0) {
    report_error(errno, "ibv_destroy_cq");
  }
  return ret;
}

int await_completion(struct rdma_conn *conn, struct ibv_wc *wc) {
  int ret;
  do {
    ret = ibv_poll_cq(conn->cq, 1, wc);
  } while (ret == 0);
  if (ret != 1) {
    report_error(errno, "ibv_poll_cq");
  }
  return ret;
}
