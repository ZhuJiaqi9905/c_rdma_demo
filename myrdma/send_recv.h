#ifndef MYRDMA_SEND_RECV_H
#define MYRDMA_SEND_RECV_H
#include "rdma_conn.h"
extern int post_send(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id);
extern int post_recv(struct rdma_conn *conn, void *addr, uint32_t length,
              uint64_t wr_id);
#endif