#include "rdma_common.h"
int create_event_channel(struct rdma_conn *conn) {
  conn->event_channel = rdma_create_event_channel();
  int ret = 0;
  if (conn->event_channel == NULL) {
    report_error(errno, "rdma_create_event_channel");
    ret = -1;
  }
  return ret;
}

void destroy_event_channel(struct rdma_conn *conn) {
  rdma_destroy_event_channel(conn->event_channel);
}

struct rdma_cm_event *await_cm_event(struct rdma_conn *conn) {
  int ret;
  struct rdma_cm_event *cm_event = NULL;
  ret = rdma_get_cm_event(conn->event_channel, &cm_event);
  if (ret != 0) {
    report_error(errno, "rdma_get_cm_event");
    goto out0;
  }
  rdma_ack_cm_event(cm_event);

out0:
  return cm_event;
}

