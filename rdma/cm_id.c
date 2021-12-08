#include "rdma_conn.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
int create_cm_id(struct rdma_conn *conn) {
  int ret = rdma_create_id(conn->event_channel, &conn->cm_id, (void *)conn,
                           RDMA_PS_TCP);
  if (ret != 0) {
    report_error(errno, "rdma_create_id");
  }
  return ret;
}

int migrate_cm_id(struct rdma_conn *conn, struct rdma_cm_id *new_cm_id) {
  int ret = rdma_migrate_id(new_cm_id, conn->event_channel);
  if (ret != 0) {
    report_error(errno, "rdma_migrate_id");
    goto out0;
  }
  conn->cm_id = new_cm_id;
  conn->cm_id->context = (void *)conn;
out0:
  return ret;
}

int destroy_cm_id(struct rdma_conn *conn) {
  int ret;
  ret = rdma_destroy_id(conn->cm_id);
  if (ret != 0) {
    report_error(errno, "rdma_destroy_id");
  }
  return ret;
}

int client_bind(struct rdma_conn *conn, const char *server_ip,
                const char *server_port) {
  int ret = -1;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons((uint64_t)atoi(server_port));
  if (inet_pton(AF_INET, server_ip, &addr.sin_addr) != 1) {
    report_error(errno, "inet_pton");
    ret = 0;
    goto out0;
  }

  ret = rdma_resolve_addr(conn->cm_id, NULL, (struct sockaddr *)&addr,
                          TIMEOUT_MS);
  if (ret != 0) {
    report_error(errno, "rdma_resolve_addr");
    goto out0;
  }
  struct rdma_cm_event event;
  ret = await_cm_event(conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_ADDR_RESOLVED) {
    ret = -1;
    goto out0;
  }

  ret = rdma_resolve_route(conn->cm_id, TIMEOUT_MS);
  if (ret != 0) {
    report_error(errno, "rdma_resolve_route");
    goto out0;
  }
  ret = await_cm_event(conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_ROUTE_RESOLVED) {
    ret = -1;
  }
out0:
  return ret;
}

int server_bind(struct rdma_conn *conn, const char *server_ip,
                const char *server_port) {
  int ret = 0;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons((uint64_t)atoi(server_port));
  if (inet_pton(AF_INET, server_ip, &addr.sin_addr) != 1) {
    report_error(errno, "inet_pton");
    ret = -1;
    goto out0;
  }
  ret = rdma_bind_addr(conn->cm_id, (struct sockaddr *)&addr);
  if (ret != 0) {
    report_error(errno, "rdma_bind_addr");
    goto out0;
  }
  ret = rdma_listen(conn->cm_id, MAX_BACKLOG);
  if (ret != 0) {
    report_error(errno, "rdma_listen");
    goto out0;
  }

out0:
  return ret;
}

static void setup_conn_params(struct rdma_conn_param *params) {
  memset(params, 0, sizeof(*params));

  params->private_data = NULL;
  params->private_data_len = 0;
  params->responder_resources = 2;
  params->initiator_depth = 2;
  params->retry_count = 5;
  params->rnr_retry_count = 5;
}

int client_connect(struct rdma_conn *conn) {
  struct rdma_conn_param client_params;
  int ret;

  setup_conn_params(&client_params);
  ret = rdma_connect(conn->cm_id, &client_params);
  if (ret != 0) {
    report_error(errno, "rdma_connect");
  }
  struct rdma_cm_event event;
  ret = await_cm_event(conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_ESTABLISHED) {
    ret = -1;
  }
  return ret;
}

int server_accept(struct rdma_conn *conn) {
  struct rdma_conn_param server_params;
  int ret;

  setup_conn_params(&server_params);
  ret = rdma_accept(conn->cm_id, &server_params);
  if (ret != 0) {
    report_error(errno, "rdma_accept");
  }
  struct rdma_cm_event event;
  ret = await_cm_event(conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_ESTABLISHED) {
    ret = -1;
  }
  return ret;
}

int client_disconnect(struct rdma_conn *conn) {
  int ret;
  errno = 0;
  ret = rdma_disconnect(conn->cm_id);
  if (ret != 0) {
    report_error(errno, "rdma_disconnect");
  }
  struct rdma_cm_event event;
  ret = await_cm_event(conn, &event);
  if (ret != 0 || event.event != RDMA_CM_EVENT_DISCONNECTED) {
    ret = -1;
  }
  return ret;
}
void report_error(int err, const char *verb_name) {
  // fprintf(stderr, "error: %s. errno: %d.", verb_name, err);
  printf("error: %s. errno: %d.", verb_name, err);
}


