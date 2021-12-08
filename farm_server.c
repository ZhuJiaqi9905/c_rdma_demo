#include "farm/farm.h"
#include "rdma/rdma_conn.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
static int connect_and_operate(struct rdma_cm_id *id) {
  int num = 20;
  int cqe = 10;
  int region_len = 4096;
  int ret = -1;
  struct rdma_conn *conn = new_rdma_conn(region_len);
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
  if (register_mr(conn, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
                            IBV_ACCESS_REMOTE_WRITE) != 0) {
    goto out6;
  }
  printf("register mr\n");

  if (server_accept(conn) != 0) {
    goto out7;
  }
  printf("accept\n");
  // connected
  printf("connected\n");

  struct farm_recver *recver = new_recver(conn, 4092);

  int buf_len = 4096;
  uint8_t *buf = (uint8_t *)malloc(buf_len);
  printf("start recv\n");
  struct timeval t_start;
  struct timeval t_end;
  struct timeval t_result;
  gettimeofday(&t_start, NULL);

  int rlen = 0;
  while (1) {
    rlen = 0;
    while (rlen == 0) {
      rlen = farm_read(recver, buf, buf_len);
    }
    printf("read %d bytes", rlen);
  }
  gettimeofday(&t_end, NULL);
  timersub(&t_end, &t_start, &t_result);
  double duration = t_result.tv_sec + (1.0 * t_result.tv_usec) / 1000000;
  double size = rlen;
  double throughput = size / duration;
  printf("duration: %lfs, size: %lfMB, throuthput: %lfMB/s\n", duration, size,
         throughput);
  // disconnect
  struct rdma_cm_event event;
  ret = await_cm_event(conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_DISCONNECTED) {
    goto out8;
  }
  printf("disconnect\n");
  ret = 0;
out8:
  drop_recver(recver);
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

  return ret;
}
int main() {
  int ret = EXIT_FAILURE;
  char server_ip[] = "10.0.12.25";
  char server_port[] = "7900";
  struct rdma_conn *l_conn = new_rdma_conn(4096);
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
  drop_rdma_conn(l_conn);
out0:
  exit(ret);
}