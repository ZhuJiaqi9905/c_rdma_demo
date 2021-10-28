#ifndef MYRDMA_MR_H
#define MYRDMA_MR_H
#include "rdma_conn.h"

extern int register_mr(struct rdma_conn *conn, void *addr_send,
                       size_t length_send, enum ibv_access_flags access_send,
                       void *addr_recv, size_t length_recv,
                       enum ibv_access_flags access_recv);
extern int deregister_mr(struct rdma_conn *conn);

#endif