#include "rdma_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
static int connect_and_operate(struct rdma_cm_id *id) {
  int num = 2000;
  int cqe = 10;
  int buf_len = 65536;
  uint8_t *send_buf = (uint8_t *)malloc(buf_len);
  uint8_t *recv_buf = (uint8_t *)malloc(buf_len);
  for (int i = 0; i < buf_len; ++i) {
    send_buf[i] = 1;
    recv_buf[i] = 1;
  }
  int ret = -1;
  struct rdma_conn *conn = create_rdma_conn();
  if (conn == NULL) {
    goto out0;
  }
  if (create_event_channel(conn) != 0) {
    goto out1;
  }
  printf("create_event_channel\n");
  if (migrate_cm_id(conn, id) != 0) {
    goto out2;
  }
  printf("migrate_cm_id\n");
  if (alloc_pd(conn) != 0) {
    goto out3;
  }
  printf("alloc pd\n");
  if (create_cq(conn, cqe) != 0) {
    goto out4;
  }
  printf("create cq\n");
  if (create_qp(conn, 0, 10, 10, 1, 1) != 0) {
    goto out5;
  }
  printf("create qp\n");
  if (register_mr(conn, (void *)send_buf, buf_len,
                  IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                      IBV_ACCESS_REMOTE_WRITE,
                  (void *)recv_buf, buf_len,
                  IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                      IBV_ACCESS_REMOTE_WRITE) != 0) {
    goto out6;
  }
  printf("register mr\n");
  for (int i = 0; i < 10; ++i) {
    post_recv(conn, recv_buf, buf_len, 0);
  }
  if (server_accept(conn) != 0) {
    goto out7;
  }
  printf("accept\n");
  // connected
  printf("connected");
  printf("my rkey is %d\n", conn->mr_recv->rkey);
  exchange_rkey(conn);
  printf("received rkey is %d\n", conn->remote_rkey);
  struct ibv_wc wc;
  struct timeval t_start;
  struct timeval t_end;
  struct timeval t_result;

  for (int i = 0; i < num; ++i) {
    if (i == 0) {
      gettimeofday(&t_start, NULL);
    }
    if (await_completion(conn, &wc) != 1) {
      goto out7;
    }
    if (wc.status != IBV_WC_SUCCESS || wc.opcode != IBV_WC_RECV) {
      goto out7;
    }
    post_recv(conn, recv_buf, buf_len, 0);
  }
  gettimeofday(&t_end, NULL);
  timersub(&t_end, &t_start, &t_result);
  double duration = t_result.tv_sec + (1.0 * t_result.tv_usec) / 1000000;
  double size = 1.0 * num * buf_len / (1024 * 1024);
  double throughput = size / duration;
  printf("duration: %lfs, size: %lfMB, throuthput: %lfMB/s", duration, size,
         throughput);
  // disconnect
  struct rdma_cm_event event;
  ret = await_cm_event(conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_DISCONNECTED) {
    goto out7;
  }
  printf("disconnect\n");
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
  return ret;
}
int main() {
  int ret = EXIT_FAILURE;
  char server_ip[] = "10.0.12.25";
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
  printf("binded\n");
  struct rdma_cm_event event;
  ret = await_cm_event(l_conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_CONNECT_REQUEST) {
    printf("event num: %d", event.event);
    goto out3;
  }
  printf("receive connection request\n");
  // connect to the client and do operation
  if (connect_and_operate(event.id) != 0) {
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