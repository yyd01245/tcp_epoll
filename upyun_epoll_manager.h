#pragma once

#include "upyun_epoll_common.h"
#include "upyun_epoll_event.h"
#include <glib.h>

int upy_control_server_send(epoll_connection_node_t *c);
int upy_control_server_recv(epoll_connection_node_t *c);
void handle_function (void *ev);
void cleanup_handler_function (void *ev);
void upy_epoll_empty_handler(void *ev);
void upy_epoll_init_event(epoll_node_t *ep,int fd);
int upy_parse_recv_data(epoll_connection_node_t *c);
int upy_send_data(epoll_connection_node_t *c,char* inbuf,int inlen);

int upy_server_loop(epoll_node_t *ep);