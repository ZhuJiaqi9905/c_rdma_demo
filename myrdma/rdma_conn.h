#ifndef MYRDMA_RDMA_COON_H
#define MYRDMA_RDMA_CONN_H
struct rdma_conn {
  struct rdma_event_channel *event_channel;
  struct rdma_cm_id *cm_id;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_qp *qp;
  struct ibv_mr *mr_send;
  struct ibv_mr *mr_recv;
  uint32_t remote_rkey;
};
extern struct rdma_conn *create_rdma_conn();
extern void destroy_rdma_conn(struct rdma_conn *conn);
#endif