#ifndef MYRDMA_PD_H
#define MYRDMA_PD_H
#include "rdma_conn.h"
extern int alloc_pd(struct rdma_conn *conn);
extern int dealloc_pd(struct rdma_conn *conn);
#endif