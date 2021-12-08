#include "farm/farm.h"
#include "rdma/rdma_conn.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
int main() {
  int num = 20;
  int ret = EXIT_FAILURE;
  char server_ip[] = "10.0.12.25";
  char server_port[] = "7900";
  int cqe = 10;
  int region_len = 4096;

  struct rdma_conn *conn = new_rdma_conn(region_len);

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
  if (register_mr(conn, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                            IBV_ACCESS_REMOTE_WRITE) != 0) {
    goto out6;
  }
  // printf("send_buf: %p, mr_addr: %p\n", send_buf, conn->mr_send->addr);
  printf("mr registered\n");
  if (client_connect(conn) != 0) {
    goto out7;
  }
  // connected
  printf("connected\n");
  struct farm_sender *sender = new_sender(conn, 4092);

  int data_len = 7000;
  uint8_t *data = (uint8_t *)malloc(data_len);
  printf("start send\n");
  struct timeval t_start;
  struct timeval t_end;
  struct timeval t_result;
  gettimeofday(&t_start, NULL);
  int s = 0;
  while (s < data_len) {
    int res = farm_write(sender, data, data_len);
    if (res != 0) {
      printf("write %d bytes", res);
    }
    s += res;
  }
  gettimeofday(&t_end, NULL);
  timersub(&t_end, &t_start, &t_result);
  printf("write finish\n");

  double duration = t_result.tv_sec + (1.0 * t_result.tv_usec) / 1000000;
  double size = data_len;
  double throughput = size / duration;
  printf("duration: %lfs, size: %lfMB, throuthput: %lfMB/s", duration, size,
         throughput);
  // disconnected
  if (client_disconnect(conn) != 0) {
    goto out8;
  }
  printf("disconnect\n");
  ret = EXIT_SUCCESS;
out8:
  drop_sender(sender);
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
  drop_rdma_conn(conn);
out0:
  free(data);
  exit(ret);
}