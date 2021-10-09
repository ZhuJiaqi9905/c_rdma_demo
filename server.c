#include "rdma_common.h"
#include <stdio.h>
#include <stdlib.h>

static int connect_and_operate(struct rdma_cm_id *id) {
  int num = 2000;
  int cqe = 10;
  int buf_len = 65536;
  uint8_t *send_buf = (uint8_t *)malloc(buf_len);
  uint8_t *recv_buf = (uint8_t *)malloc(buf_len);
  int ret = -1;
  struct rdma_conn *conn = create_rdma_conn();
  if (conn == NULL) {
    goto out0;
  }
  if (create_event_channel(conn) != 0) {
    goto out1;
  }
  if (migrate_cm_id(conn, id) != 0) {
    goto out2;
  }
  if (alloc_pd(conn) != 0) {
    goto out3;
  }
  if (create_cq(conn, cqe) != 0) {
    goto out4;
  }
  if (create_qp(conn, 0, 10, 10, 1, 1) != 0) {
    goto out5;
  }
  if (register_mr(conn, (void *)send_buf, buf_len,
                  IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                      IBV_ACCESS_REMOTE_WRITE,
                  (void *)recv_buf, buf_len,
                  IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                      IBV_ACCESS_REMOTE_WRITE) != 0) {
    goto out6;
  }
  for (int i = 0; i < cqe; ++i) {
    post_recv(conn, recv_buf, buf_len, 0);
  }
  if (server_accept(conn) != 0) {
    goto out7;
  }
  // connected
  printf("connected");
  struct ibv_wc wc;
  for (int i = 0; i < num; ++i) {
    if (await_completion(conn, &wc) != 1) {
      goto out7;
    }
    if (wc.status != IBV_WC_SUCCESS || wc.opcode != IBV_WC_RECV) {
      goto out7;
    }
    post_recv(conn, recv_buf, buf_len, 0);
  }

  ret = 0;
out7:
  deregister_mr(conn);
out6:
  destroy_qp(conn);
out5:
  destroy_cq(conn);
out4:
  dealloc_pd(conn);
out3:
  destroy_cm_id(conn);
out2:
  destroy_event_channel(conn);
out1:
  destroy_rdma_conn(conn);
out0:
  free(send_buf);
  free(recv_buf);
  return 0;
}
int main() {
  int ret = EXIT_FAILURE;
  char server_ip[] = "10.0.12.24";
  char server_port[] = "7900";
  struct rdma_conn *l_conn = create_rdma_conn();
  if (l_conn == NULL) {
    goto out0;
  }
  // set up
  if (create_event_channel(l_conn) != 0) {
    goto out1;
  }

  if (create_cm_id(l_conn) != 0) {
    goto out2;
  }
  if (server_bind(l_conn, server_ip, server_port) != 0) {
    goto out3;
  }
  struct rdma_cm_event *event = await_cm_event(l_conn);
  if (event != NULL || event->event != RDMA_CM_EVENT_CONNECT_REQUEST) {
    goto out3;
  }
  // connect to the client and do operation
  if (connect_and_operate(event->id) != 0) {
    goto out3;
  }

  ret = EXIT_SUCCESS;
out3:
  destroy_cm_id(l_conn);
out2:
  destroy_event_channel(l_conn);
out1:
  destroy_rdma_conn(l_conn);
out0:
  exit(ret);
}