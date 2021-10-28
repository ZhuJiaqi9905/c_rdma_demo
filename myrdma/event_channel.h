#ifndef MYRDMA_EVENT_CHANNEL_H
#define MYRDMA_EVENT_CHANNEL_H
#include "rdma_conn.h"
extern int create_event_channel(struct rdma_conn *conn);
extern void destroy_event_channel(struct rdma_conn *conn);
extern int await_cm_event(struct rdma_conn *conn, struct rdma_cm_event *event_copy);

#endif