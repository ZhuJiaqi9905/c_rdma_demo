#include "rdma_common.h"
#include <infiniband/verbs.h>
int post_send(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id) {
  memset(&conn->sge, 0, sizeof(conn->sge));
  conn->sge.addr = (uint64_t)addr;
  conn->sge.length = length;
  conn->sge.lkey = conn->mr_send->lkey;
  memset(&conn->send_wr, 0, sizeof(conn->send_wr));
  struct ibv_send_wr *bad_wr;
  int ret;
  conn->send_wr.wr_id = wr_id;
  conn->send_wr.next = NULL;
  conn->send_wr.sg_list = &conn->sge;
  conn->send_wr.num_sge = 1;
  conn->send_wr.opcode = IBV_WR_SEND;
  conn->send_wr.send_flags = IBV_SEND_SIGNALED;
  ret = ibv_post_send(conn->qp, &conn->send_wr, &bad_wr);
  if (ret != 0) {
    report_error(errno, "ibv_post_send");
  }
  return ret;
}

int post_recv(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id) {

  memset(&conn->sge, 0, sizeof(conn->sge));
  conn->sge.addr = (uint64_t)addr;
  conn->sge.length = length;
  conn->sge.lkey = conn->mr_recv->lkey;

  struct ibv_recv_wr *bad_wr;
  int ret;
  memset(&conn->recv_wr, 0, sizeof(conn->recv_wr));
  conn->recv_wr.wr_id = wr_id;
  conn->recv_wr.next = NULL;
  conn->recv_wr.sg_list = &conn->sge;
  conn->recv_wr.num_sge = 1;

  ret = ibv_post_recv(conn->qp, &conn->recv_wr, &bad_wr);
  if (ret != 0) {
    report_error(errno, "ibv_post_recv");
  }
  return ret;
}
