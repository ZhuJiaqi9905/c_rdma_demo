#include "rdma_conn.h"

int register_mr(struct rdma_conn *conn, enum ibv_access_flags access_flag) {
  int ret = 0;
  conn->mr = ibv_reg_mr(conn->pd, conn->region, conn->region_len, access_flag);
  if (conn->mr == NULL) {
    report_error(errno, "ibv_register_mr");
    ret = -1;
  }
  return ret;
}

int deregister_mr(struct rdma_conn *conn) {
  int ret;
  ret = ibv_dereg_mr(conn->mr);
  if (ret != 0) {
    report_error(errno, "ibv_dereg_mr");
  }
  return ret;
}