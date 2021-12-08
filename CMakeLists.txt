cmake_minimum_required(VERSION 3.14)
project(c_rdma C)
set(CMAKE_C_STANDARD 11)
set(MY_TEST OFF)


include_directories(/usr/local/include)
include_directories(${CMAKE_SOURCE_DIR})


link_directories(/usr/local/lib)

add_library(rdma_conn 
STATIC
    rdma/rdma_conn.h
    rdma/rdma_conn.c 
    rdma/cm_id.c
    rdma/cq.c
    rdma/event_channel.c 
    rdma/mr.c 
    rdma/pd.c 
    rdma/qp.c 
    rdma/read_write.c 
    rdma/send_recv.c 
)
add_executable(farm_client 
    farm_client.c
    farm/recv_buffer.c
    farm/send_buffer.c
)
add_executable(farm_server
farm_server.c
farm/recv_buffer.c 
farm/send_buffer.c)
target_link_libraries(rdma_conn ibverbs rdmacm)
target_link_libraries(farm_client LINK_PRIVATE rdma_conn
)
target_link_libraries(farm_server LINK_PRIVATE rdma_conn)
