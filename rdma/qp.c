#include "rdma_conn.h"
int create_qp(struct rdma_conn *conn, int sq_sig_all, uint32_t max_send_wr,
              uint32_t max_recv_wr, uint32_t max_send_sge,
              uint32_t max_recv_sge) {
  struct ibv_qp_init_attr init_attr;
  int ret;
  memset(&init_attr, 0, sizeof(struct ibv_qp_init_attr));
  // init_attr.qp_context = (void *)conn;
  
  init_attr.send_cq = conn->cq;
  init_attr.recv_cq = conn->cq;
  init_attr.srq = NULL;
  init_attr.sq_sig_all = sq_sig_all;
  init_attr.cap.max_send_wr = max_send_wr;
  init_attr.cap.max_recv_wr = max_recv_wr;
  init_attr.cap.max_send_sge = max_send_sge;
  init_attr.cap.max_recv_sge = max_recv_sge;
  init_attr.cap.max_inline_data = 16;
  init_attr.qp_type = IBV_QPT_RC;
  ret = rdma_create_qp(conn->cm_id, conn->pd, &init_attr);
  conn->qp = conn->cm_id->qp;
  if (ret != 0) {
    report_error(errno, "rdma_create_qp");
  }
  return ret;
}

void destroy_qp(struct rdma_conn *conn) { rdma_destroy_qp(conn->cm_id); }
