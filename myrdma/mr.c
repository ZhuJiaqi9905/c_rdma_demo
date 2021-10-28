#include "mr.h"
#include <rdma/rdma_verbs.h>
int register_mr(struct rdma_conn *conn, void *addr_send, size_t length_send,
                enum ibv_access_flags access_send, void *addr_recv,
                size_t length_recv, enum ibv_access_flags access_recv) {
  int ret = 0;
  conn->mr_send = ibv_reg_mr(conn->pd, addr_send, length_send, access_send);
  if (conn->mr_send == NULL) {
    report_error(errno, "ibv_register_mr_send");
    ret = -1;
    goto out0;
  }
  conn->mr_recv = ibv_reg_mr(conn->pd, addr_recv, length_recv, access_recv);
  if (conn->mr_recv == NULL) {
    report_error(errno, "ibv_register_mr_recv");
    ret = -1;
  }
out0:
  return ret;
}

int deregister_mr(struct rdma_conn *conn) {
  int ret;
  ret = ibv_dereg_mr(conn->mr_send);
  if (ret != 0) {
    report_error(errno, "ibv_dereg_mr_send");
  }
  ret = ibv_dereg_mr(conn->mr_recv);
  if (ret != 0) {
    report_error(errno, "ibv_dereg_mr_recv");
  }
  return ret;
}