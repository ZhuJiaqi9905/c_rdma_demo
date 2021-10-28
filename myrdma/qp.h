#ifndef MYRDMA_QP_H
#define MYRDMA_QP_H
#include "rdma_conn.h"
extern int create_qp(struct rdma_conn *conn, int sq_sig_all, uint32_t max_send_wr,
              uint32_t max_recv_wr, uint32_t max_send_sge,
              uint32_t max_recv_sge);
extern void destroy_qp(struct rdma_conn *conn);
#endif