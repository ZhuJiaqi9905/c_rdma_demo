#ifndef MYRDMA_CQ_H
#define MYRDMA_CQ_H
#include "rdma_conn.h"
extern int create_cq(struct rdma_conn *conn, int cqe);
extern int destroy_cq(struct rdma_conn *conn);
extern int await_completion(struct rdma_conn *conn, struct ibv_wc *wc);
#endif