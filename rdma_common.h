#ifndef C_RDMA_DEMO_RDMA_COMMON_H
#define C_RDMA_DEMO_RDMA_COMMON_H
#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include <stdint.h>
#define MAX_WORK_REQUESTS 1
#define MAX_BACKLOG 10
#define TIMEOUT_MS 2000
struct rdma_conn {
  struct rdma_event_channel *event_channel;
  struct rdma_cm_id *cm_id;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_qp *qp;
  struct ibv_mr *mr_send;
  struct ibv_mr *mr_recv;
  uint32_t remote_rkey;
  void *send_buf;
  void *recv_buf;
  uint32_t send_len;
  uint32_t recv_len;
};
struct rdma_conn *create_rdma_conn();
void destroy_rdma_conn(struct rdma_conn *conn);

// event channel
int create_event_channel(struct rdma_conn *conn);
void destroy_event_channel(struct rdma_conn *conn);
int await_cm_event(struct rdma_conn *conn, struct rdma_cm_event *event_copy);

// cm_id
int create_cm_id(struct rdma_conn *conn);
int migrate_cm_id(struct rdma_conn *conn, struct rdma_cm_id *new_cm_id);
int destroy_cm_id(struct rdma_conn *conn);
int client_bind(struct rdma_conn *conn, const char *server_ip,
                const char *server_port);
int server_bind(struct rdma_conn *conn, const char *server_ip,
                const char *server_port);
int client_connect(struct rdma_conn *conn);
int server_accept(struct rdma_conn *conn);
int client_disconnect(struct rdma_conn *conn);
void report_error(int err, const char *verb_name);
int exchange_rkey(struct rdma_conn *conn);
// cq
int create_cq(struct rdma_conn *conn, int cqe);
int destroy_cq(struct rdma_conn *conn);
int await_completion(struct rdma_conn *conn, struct ibv_wc *wc);

// pd
int alloc_pd(struct rdma_conn *conn);
int dealloc_pd(struct rdma_conn *conn);

// mr
int register_mr(struct rdma_conn *conn, void *addr_send, size_t length_send,
                enum ibv_access_flags access_send, void *addr_recv,
                size_t length_recv, enum ibv_access_flags access_recv);
int deregister_mr(struct rdma_conn *conn);
// qp
int create_qp(struct rdma_conn *conn, int sq_sig_all, uint32_t max_send_wr,
              uint32_t max_recv_wr, uint32_t max_send_sge,
              uint32_t max_recv_sge);
void destroy_qp(struct rdma_conn *conn);
// send_recv
int post_send(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id);
int post_recv(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id);
int post_read(struct rdma_conn *conn, void *addr, uint32_t length,
              void *remote_addr, uint64_t wr_id);
int post_write(struct rdma_conn *conn, void *addr, uint32_t length,
               void *remote_addr, uint64_t wr_id);

#endif