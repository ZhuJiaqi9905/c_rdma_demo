CC = gcc
CFLAGS = -Wall -O -g
LDLIBS := ${LDLIBS} -lrdmacm -libverbs

EXECUTABLES = client server

all: $(EXECUTABLES)

client: client.o cm_id.o cq.o event_channel.o \
		mr.o pd.o qp.o rdma_conn.o \
		send_recv.o

server: server.o cm_id.o cq.o event_channel.o \
		mr.o pd.o qp.o rdma_conn.o \
		send_recv.o 

client.o: client.c rdma_common.h

server.o: server.c rdma_common.h

cm_id.o: cm_id.c rdma_common.h

cq.o: cq.c rdma_common.h

event_channel.o: event_channel.c rdma_common.h

mr.o: mr.c rdma_common.h

pd.o: pd.c rdma_common.h

qp.o: qp.c rdma_common.h

rdma_conn.o: rdma_conn.c rdma_common.h

send_recv.o: send_recv.c rdma_common.h

.PHONY: all clean cleanall

clean:
	rm *.o $(EXECUTABLES)


