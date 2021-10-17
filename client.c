#include "rdma_common.h"
#include <stdio.h>
#include <stdlib.h>
int main() {
  int num = 2000;
  int ret = EXIT_FAILURE;
  char server_ip[] = "10.0.12.25";
  char server_port[] = "7900";
  int cqe = 10;
  int buf_len = 65536;
  uint8_t *send_buf = (uint8_t *)malloc(buf_len);
  uint8_t *recv_buf = (uint8_t *)malloc(buf_len);
  struct rdma_conn *conn = create_rdma_conn();
  if (conn == NULL) {
    goto out0;
  }
  // set up
  if (create_event_channel(conn) != 0) {
    goto out1;
  }
  printf("event_channel created\n");
  if (create_cm_id(conn) != 0) {
    goto out2;
  }
  printf("cm_id created\n");
  if (client_bind(conn, server_ip, server_port) != 0) {
    goto out3;
  }
  printf("binded\n");
  if (alloc_pd(conn) != 0) {
    goto out3;
  }
  printf("alloc pd\n");
  if (create_cq(conn, cqe) != 0) {
    goto out4;
  }
  printf("cq created\n");
  if (create_qp(conn, 0, 10, 10, 1, 1) != 0) {
    goto out5;
  }
  printf("qp created\n");
  if (register_mr(conn, (void *)send_buf, buf_len,
                  IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                      IBV_ACCESS_REMOTE_WRITE,
                  (void *)recv_buf, buf_len,
                  IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                      IBV_ACCESS_REMOTE_WRITE) != 0) {
    goto out6;
  }
  printf("mr registered\n");
  if (client_connect(conn) != 0) {
    goto out7;
  }
  // connected
  printf("connected\n");

  struct ibv_wc wc;
  // for (int i = 0; i < 1; ++i) {
  //   if (post_send(conn, send_buf, buf_len, 0) != 0) {
  //     goto out7;
  //   }
  //   if (await_completion(conn, &wc) != 1) {
  //     goto out7;
  //   }
  //   if (wc.status != IBV_WC_SUCCESS || wc.opcode != IBV_WC_SEND) {
  //     goto out7;
  //   }
  // }
  // disconnected
  if (client_disconnect(conn) != 0) {
    goto out7;
  }
  printf("disconnect\n");
  ret = EXIT_SUCCESS;
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
  exit(ret);
}