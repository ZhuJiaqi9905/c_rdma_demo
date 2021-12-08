#include "rdma_rw_channel.h"
#include "rdma_common.h"
int write_to_remote(struct rdma_rw_channel *channel, void *addr,
                    uint32_t length) {

  uint32_t rh_offset =
      *channel->rch_addr -
      channel->rb_addr; // rh_offset is offset of remote ring buffer's head
  uint32_t rb_len = channel->rb_len;
  uint32_t rt_offset = channel->rt_offset;
  if ((rt_offset + 1) % rb_len == rh_offset) { // the queue is full
    return 0;
  }
  // uint32_t nt_offset = (rt_offset + length) % rb_len; // new tail offset
  uint32_t nt_offset = (rt_offset + length) % rb_len;
  if (rt_offset <= rh_offset &&
      nt_offset >= rh_offset) { // can not send because the remote ring buffer
                                // will overflow
    return 0;
  }
  if (rt_offset + length <= rb_len) {
    if (post_write(channel->conn, addr, length,
                   (void *)(channel->rb_addr + rt_offset), 0) != 0) {
      return -1;
    }
    struct ibv_wc wc;
    if (await_completion(channel->conn, &wc) != 1) {
      return -1;
    }
    if (wc.status != IBV_WC_SUCCESS || wc.opcode != IBV_WC_RDMA_WRITE) {
      return -1;
    }
  } else {
    uint32_t len1 = rb_len - rt_offset;
    uint32_t len2 = length - len1;
    if (post_write(channel->conn, addr, len1,
                   (void *)(channel->rb_addr + rt_offset), 0) != 0) {
      return -1;
    }
    if (post_write(channel->conn, (void *)((uint8_t *)addr + len1), len2,
                   (void *)channel->rb_addr, 0) != 0) {
      return -1;
    }
    struct ibv_wc wc;
    for (int i = 0; i < 2; ++i) {
      if (await_completion(channel->conn, &wc) != 1) {
        return -1;
      }
      if (wc.status != IBV_WC_SUCCESS || wc.opcode != IBV_WC_RDMA_WRITE) {
        return -1;
      }
    }
  }
  channel->rt_offset = nt_offset;
  return length;
}
// zero copy
int read_received_data(struct rdma_rw_channel *channel, void **addr1,
                       uint32_t *len1, void **addr2, uint32_t *len2) {
  uint32_t lh_offset = channel->lh_offset;
  uint32_t lb_len = channel->lb_len;
  uint8_t *lh_addr = (uint8_t *)channel->conn.send_buf + lh_offset;
  uint32_t length; // the length of data
  // calculate the length, it is complicated becaused of the ring buffer
  if (lb_len - lh_offset >= sizeof(uint32_t)) {
    length = *(uint32_t *)lh_addr;
    lh_addr += sizeof(uint32_t);
  } else {
    uint8_t *p_length = (uint8_t *)&length;
    for (int i = 0; i < lb_len - lh_offset; ++i) {
      p_length[i] = *(lh_addr++);
    }
    lh_addr = (uint8_t *)channel->conn.send_buf;
    for (int i = lb_len - lh_offset; i < sizeof(uint32_t); ++i) {
      p_length[i] = *(lh_addr++);
    }
  }
  *addr1 = (void *)lh_addr;
  lh_offset = (lh_offset + sizeof(uint32_t)) % lb_len;
  if (length + lh_offset <= lb_len) {
    *len1 = length;
    *addr2 = NULL;
    *len2 = 0;
  } else {
    *len1 = lb_len - lh_offset;
    *addr2 = channel->conn.send_buf;
    *len2 = length - *len1;
  }
  channel->lh_offset = (lh_offset + length) % lb_len;
  if ((channel->lh_offset - channel->lh_offset_r) > channel->lb_len / 4 ||
      (channel->lb_len - channel->lh_offset_r + channel->lh_offset) >
          channel->lb_len / 4) {
    may_update_remote_copy_head(channel);
  }
  if (may_update_remote_copy_head(channel) != 0) {
    report_error(errno, "may_update_remote_copy_head()");
    return -1;
  }
  return 0;
}

int may_update_remote_copy_head(struct rdma_rw_channel *channel) {
  // filter the case that not need to update copy head
  if (channel->lh_offset >= channel->lh_offset_r) {
    if ((channel->lh_offset - channel->lh_offset_r) < channel->lb_len / 4) {
      return 0;
    }
  } else {
    if ((channel->lb_len - channel->lh_offset_r + channel->lh_offset) <
        channel->lb_len / 4) {
      return 0;
    }
  }

  *channel->sch_addr = (uint64_t)(channel->conn.send_buf + channel->lh_offset);
  channel->lh_offset_r = channel->lh_offset;
  if (post_write(channel->conn, channel->sch_addr, sizeof(uint64_t),
                 (void *)channel->recver_rch_addr, 0) != 0) {
    return -1;
  }
  struct ibv_wc wc;
  if (await_completion(channel->conn, &wc) != 1) {
    return -1;
  }
  if (wc.status != IBV_WC_SUCCESS || wc.opcode != IBV_WC_RDMA_WRITE) {
    return -1;
  }
  return 0;
}

struct rdma_rw_channel *create_rdma_rw_channel(struct rdma_conn *r_conn){
  struct rdma_rw_channel * channel = (struct rdma_rw_channel*) malloc()
}
void destroy_rdma_rw_channel(struct rdma_rw_channel *channel);