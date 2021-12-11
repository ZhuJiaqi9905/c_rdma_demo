#include "farm.h"
#include <stdio.h>
#include <stdlib.h>
static int exchange_metadata(struct farm_recver *self, const uint8_t *my_addr);
static int floor8(int num);
static int floor4(int num);
static int try_udpate_head_to_remote(struct farm_recver *self);

static int try_udpate_head_to_remote(struct farm_recver *self) {
  if ((self->last_head < self->head &&
       self->head - self->last_head < self->recv_buf_len / 2) ||
      (self->last_head > self->head &&
       self->recv_buf_len - self->last_head + self->head <
           self->recv_buf_len / 2)) {
    return 0;
  }
  *self->head_pos_buf = self->head;
  post_write(self->conn, (void *)self->head_pos_buf, 4,
             self->remote_head_copy_addr, self->wr_id);
  self->wr_id += 1;
  struct ibv_wc wc;
  if (await_completion(self->conn, &wc) != 1 || wc.status != IBV_WC_SUCCESS ||
      wc.opcode != IBV_WC_RDMA_WRITE) {
    return -1;
  }
  self->last_head = self->head;
}
static int exchange_metadata(struct farm_recver *self, const uint8_t *my_addr) {
  struct rdma_conn *conn = self->conn;
  int ret = -1;
  ret = post_recv(conn, conn->region, 12, 0);
  if (ret != 0) {
    report_error(errno, "exchange_metadata 1");
    return ret;
  }
  *(uint64_t *)(conn->region + 12) = (uint64_t)my_addr;
  *(int32_t *)(conn->region + 20) = self->recv_buf_len;
  *(uint32_t *)(conn->region + 24) = conn->mr->rkey;
  ret = post_send(conn, conn->region + 12, 16, 1);
  if (ret != 0) {
    report_error(errno, "exchange_metadata 2");
    return ret;
  }
  struct ibv_wc wc;
  for (int i = 0; i < 2; ++i) {
    ret = await_completion(conn, &wc);
    if (ret != 1 || wc.status != IBV_WC_SUCCESS) {
      report_error(errno, "client_exchange_rkey 3");
      return -1;
    }
    if (wc.opcode == IBV_WC_RECV) {
      self->remote_head_copy_addr = *(uint64_t *)conn->region;
      conn->remote_rkey = *(uint32_t *)(conn->region + 8);
    }
  }
  memset(conn->region, 0, 28);
}
int farm_read(struct farm_recver *self, uint8_t *buf, int buf_len) {
  int data_len = *(int32_t *)(&self->recv_buf[self->head]);
  if (data_len == 0) {
    return 0;
  }
    *(int32_t *)(&self->recv_buf[self->head]) = 0;
  int msg_len = floor8(data_len + 4);
  printf("[ recver ] head: %d, ", self->head);
  self->head = (self->head + 4) % self->recv_buf_len;
  int read_len = data_len;
  if (data_len > buf_len) {
    read_len = buf_len;
  }
  printf(" read_len: %d", read_len);
  int floor_read_len = floor4(read_len);
  for (int i = 0; i < read_len; ++i) {
    buf[i] = self->recv_buf[self->head + i];
    self->recv_buf[self->head + i] = 0;
  }
  for (int i = read_len; i < floor_read_len; ++i) {
    self->recv_buf[self->head + i] = 0;
  }
  if (data_len > read_len) {
    int remain_len = data_len - read_len;
    self->head = (self->head + floor_read_len - 4) % self->recv_buf_len;
    *(int32_t *)(self->recv_buf + self->head) = remain_len;
  } else {
    for (int i = read_len; i < msg_len - 4; ++i) {
      self->recv_buf[self->head + i] = 0;
    }
    self->head = (self->head + msg_len - 4) % self->recv_buf_len;
  }
  printf("new head: %d\n", self->head);
  if (try_udpate_head_to_remote(self) == -1) {
    report_error(errno, "try_update_head_to_remote");
  }
  return read_len;
}

struct farm_recver *new_recver(struct rdma_conn *conn, int recv_buf_len) {
  struct farm_recver *recver =
      (struct farm_recver *)malloc(sizeof(struct farm_recver));
  recver->conn = conn;
  recver->head = 0;
  recver->last_head = 0;
  recver->recv_buf = conn->region + 4;
  recver->head_pos_buf = (int32_t *)conn->region;
  recver->recv_buf_len = recv_buf_len / 8 * 8;
  recver->wr_id = 0;
  exchange_metadata(recver, recver->recv_buf);
  return recver;
}
void drop_recver(struct farm_recver *self) { free(self); }
static int floor8(int num) { return num % 8 == 0 ? num : (num + 7) / 8 * 8; }
static int floor4(int num) { return num % 4 == 0 ? num : (num + 3) / 4 * 4; }