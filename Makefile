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
# client: client.o 

client.o: client.c include/rdma_common.h

server.o: server.c include/rdma_common.h 

cm_id.o: myrdma/cm_id.c myrdma/cm_id.h

cq.o: myrdma/cq.c myrdma/cq.h

event_channel.o: myrdma/event_channel.c myrdma/event_channel.h

mr.o: myrdma/mr.c myrdma/mr.h

pd.o: myrdma/pd.c myrdma/pd.h

qp.o: myrdma/qp.c myrdma/qp.h

rdma_conn.o: myrdma/rdma_conn.c myrdma/rdma_conn.h

send_recv.o: myrdma/send_recv.c myrdma/send_recv.h

.PHONY: all clean cleanall

clean:
	rm *.o $(EXECUTABLES)


