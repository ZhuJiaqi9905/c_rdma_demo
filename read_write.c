#include "rdma_common.h"
#include <infiniband/verbs.h>

int post_read(struct rdma_conn *conn, void *addr, uint32_t length,
              void *remote_addr, uint64_t wr_id) {
  struct ibv_sge sg;
  struct ibv_send_wr wr;
  struct ibv_send_wr *bad_wr;

  sg.addr = (uint64_t)addr;
  sg.length = length;
  sg.lkey = lkey;
  memset(&wr, 0, sizeof(wr));
  wr.wr_id = 0;
  wr.sg_list = &sg;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_RDMA_READ;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.wr.rdma.remote_addr = remote_addr;
  wr.wr.rdma.rkey = remote_rkey;
  int ret = ibv_post_send(qp, &wr, &bad_wr);
  if (ret) {
    report_error(errno, "ibv_post_send(); post_read");
  }
  return ret;
}
int post_write(struct rdma_conn *conn, void *addr, uint32_t length,
               void *remote_addr, uint64_t wr_id) {
  struct ibv_sge sg;
  struct ibv_send_wr wr;
  struct ibv_send_wr *bad_wr;

  sg.addr = (uint64_t)addr;
  sg.length = length;
  sg.lkey = conn->mr_send->lkey;
  memset(&wr, 0, sizeof(wr));
  wr.wr_id = 0;
  wr.sg_list = &sg;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_RDMA_WRITE;
  wr.send_flags = IBV_SEND_SIGNALED;
  wr.wr.rdma.remote_addr = remote_addr;
  wr.wr.rdma.rkey = conn->remote_rkey;
  int ret = ibv_post_send(qp, &wr, &bad_wr);
  if (ret) {
    report_error(errno, "ibv_post_send(); post_write");
    return -1;
  }
  return ret;
}
