
#pragma once 

#include "upyun_epoll_common.h"
#include "upyun_socket.h"
/* 
  epoll event 
  return struct epoll_event 
*/

epoll_node_t* epoll_init(int event_num);
epoll_connection_node_t* create_connection_node(int fd);
int epoll_add_listen(int listenfd);
int epoll_process_events(void* userdata);
