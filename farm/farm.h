#ifndef FARM_H
#define FARM_H
#include "../rdma/rdma_conn.h"
struct farm_sender {
  struct rdma_conn *conn;
  uint8_t *send_buf;
  int send_buf_len;
  uint64_t remote_addr;
  int remote_head;
  int remote_tail;
  int remote_len;
  int *rem_recv_head;
  uint64_t wr_id;
};

int farm_write(struct farm_sender *self, const uint8_t *buf, int buf_len);
struct farm_sender *new_sender(struct rdma_conn *conn, int send_buf_len);
void drop_sender(struct farm_sender *self);


struct farm_recver {
  struct rdma_conn *conn;
  uint8_t *recv_buf;
  int recv_buf_len;
  uint64_t remote_head_copy_addr;
  int32_t *head_pos_buf;
  int head;
  int last_head;
  uint64_t wr_id;
};
int farm_read(struct farm_recver *self, uint8_t *buf, int buf_len);
struct farm_recver *new_recver(struct rdma_conn *conn, int recv_buf_len);
void drop_recver(struct farm_recver *self);
#endif