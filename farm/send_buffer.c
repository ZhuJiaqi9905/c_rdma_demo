#include "farm.h"
#include <stdio.h>
#include <stdlib.h>
static void upate_remote_head(struct farm_sender *self);
static int exchange_metadata(struct farm_sender *self, const uint8_t *my_addr);
static int floor8(int num);

static int exchange_metadata(struct farm_sender *self, const uint8_t *my_addr) {
  struct rdma_conn *conn = self->conn;
  int ret = -1;
  ret = post_recv(conn, conn->region, 16, 0);
  if (ret != 0) {
    report_error(errno, "exchange_metadata 1");
    return ret;
  }
  *(uint64_t *)(conn->region + 16) = (uint64_t)my_addr;
  *(uint32_t *)(conn->region + 24) = conn->mr->rkey;
  ret = post_send(conn, conn->region + 16, 12, 1);
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
      self->remote_addr = *(uint64_t *)conn->region;
      self->remote_len = *(int32_t *)(conn->region + 8);
      conn->remote_rkey = *(uint32_t *)(conn->region + 12);
    }
  }
  memset(conn->region, 0, 28);
}
struct farm_sender *new_sender(struct rdma_conn *conn, int send_buf_len) {
  struct farm_sender *sender =
      (struct farm_sender *)malloc(sizeof(struct farm_sender));
  sender->conn = conn;
  sender->send_buf = conn->region + 4;
  sender->send_buf_len = send_buf_len / 8 * 8;
  sender->remote_head = 0;
  sender->remote_tail = 0;
  sender->rem_recv_head = (int *)conn->region;
  sender->wr_id = 0;
  exchange_metadata(sender, (uint8_t *)sender->rem_recv_head);
  return sender;
}

void drop_sender(struct farm_sender *self) { free(self); }
static void upate_remote_head(struct farm_sender *self) {
  self->remote_head = *self->rem_recv_head;
}

int farm_write(struct farm_sender *self, const uint8_t *buf, int buf_len) {
  if ((self->remote_tail + 8) % self->remote_len == self->remote_head) {
    upate_remote_head(self);
  }
  if ((self->remote_tail + 8) % self->remote_len == self->remote_head) {
    return 0;
  }
  printf("start write: remote_head: %d, remote_tail: %d\n", self->remote_head,
         self->remote_tail);
  int write_len = buf_len;
  int msg_len = floor8(write_len + 4);
  int capacity;
  if (self->remote_tail >= self->remote_head && self->remote_head == 0) {
    capacity = self->remote_len - self->remote_tail - 8;
  } else if (self->remote_tail >= self->remote_head && self->remote_head != 0) {
    capacity = self->remote_len - self->remote_tail;
  } else {
    capacity = self->remote_head - self->remote_tail - 8;
  }
  if (capacity < msg_len) {
    write_len = capacity - 4;
    msg_len = capacity;
  }
  *(int32_t *)self->send_buf = write_len;
  for (int i = 0; i < write_len; ++i) {
    self->send_buf[i + 4] = buf[i];
  }
  printf("[sender] write_len %d, msg_len %d\n", write_len, msg_len);
  if (post_write(self->conn, self->send_buf, write_len + 4,
                 self->remote_addr + self->remote_tail, self->wr_id) != 0) {
    return -1;
  }
  struct ibv_wc wc;
  if (await_completion(self->conn, &wc) != 1 || wc.status != IBV_WC_SUCCESS ||
      wc.opcode != IBV_WC_RDMA_WRITE) {
    return -1;
  }
  self->wr_id += 1;
  printf("[sender] post write complete\n");
  self->remote_tail = (self->remote_tail + msg_len) % self->remote_len;
  return write_len;
}

static int floor8(int num) { return num % 8 == 0 ? num : (num + 7) / 8 * 8; }