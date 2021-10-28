#ifndef MYRDMA_CM_ID_H
#define MYRDMA_CM_ID_H
#include "rdma_conn.h"
// cm_id
extern int create_cm_id(struct rdma_conn *conn);
extern int migrate_cm_id(struct rdma_conn *conn, struct rdma_cm_id *new_cm_id);
extern int destroy_cm_id(struct rdma_conn *conn);
extern int client_bind(struct rdma_conn *conn, const char *server_ip,
                const char *server_port);
extern int server_bind(struct rdma_conn *conn, const char *server_ip,
                const char *server_port);
extern int client_connect(struct rdma_conn *conn);
extern int server_accept(struct rdma_conn *conn);
extern int client_disconnect(struct rdma_conn *conn);
extern void report_error(int err, const char *verb_name);
#endif