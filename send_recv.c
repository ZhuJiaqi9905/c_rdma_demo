#include "rdma_common.h"

int post_send(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id) {
  struct ibv_sge sge;
  sge.addr = (uint64_t)addr;
  sge.length = length;
  sge.lkey = conn->mr_send->lkey;
  struct ibv_send_wr wr;
  struct ibv_send_wr *bad_wr;
  int ret;
  wr.wr_id = wr_id;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_SEND;
  wr.send_flags = IBV_SEND_SIGNALED;
  ret = ibv_post_send(conn->qp, &wr, &bad_wr);
  if (ret != 0) {
    report_error(errno, "ibv_post_send");
  }
  return ret;
}

int post_recv(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id) {
  struct ibv_sge sge;
  sge.addr = (uint64_t)addr;
  sge.length = length;
  sge.lkey = conn->mr_recv->lkey;
  struct ibv_recv_wr wr;
  struct ibv_recv_wr *bad_wr;
  int ret;
  wr.wr_id = wr_id;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  ret = ibv_post_recv(conn->qp, &wr, &bad_wr);
  if (ret != 0) {
    report_error(errno, "ibv_post_send");
  }
  return ret;
}
