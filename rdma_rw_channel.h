#ifndef RDMA_RW_CHANNEL_H
#define RDMA_RW_CHANNEL_H
#include <stdint.h>
struct rdma_rw_channel {
  struct rdma_conn *conn;
  uint64_t *rch_addr; // rc_addr is an address of the receiver's copy of head.
                      // the address is in our buffer and is updated by remote.
  uint64_t rb_addr;   // receiver buffer addr, which is set by initial post send
  uint32_t rt_offset; // receiver tail offset. the addr of receiver's tail can
                      // be computed by rb_addr + rt_offset
  uint32_t rb_len;    // receiver's recvbuffer length;
  uint32_t lh_offset; // local head offset
  uint32_t lb_len;    // local recvbuff length;
  uint32_t lh_offset_r; // remote 端能看到的buffer
                        // head的offset。其更新是延迟于lh_offset的。
  uint64_t *sch_addr;   // sender用来发送head addr的内存地址
  uint64_t recver_rch_addr;
};
struct rdma_rw_channel *create_rdma_rw_channel(struct rdma_conn *r_conn);
void destroy_rdma_rw_channel(struct rdma_rw_channel *channel);
int write_to_remote(struct rdma_rw_channel *channel, void *addr,
                    uint32_t length);
int read_received_data(struct rdma_rw_channel *channel, void **addr1,
                       uint32_t *len1, void **addr2, uint32_t *len2);
int may_update_remote_copy_head(struct rdma_rw_channel *channel);
#endif