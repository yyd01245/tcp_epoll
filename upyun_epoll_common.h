#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

//#define __USE_GNU
#include <pthread.h>
#include <jansson.h>

#define LOG(...) printf(__VA_ARGS__)
// lOG(1, ...)

#define EP_TIMEOUT 500

#define UPY_OK      0
#define UPY_ERROR    -1
#define UPY_AGAIN  -2
#define UPY_RECVBUF_MAX_SIZE 4096
#define UPY_RECVBUF_MAX_SIZE 4096

typedef struct epoll_node{
  int epoll_ep;
  int epv_num;
  struct epoll_event  *epv_list;

}epoll_node_t;


typedef void (*event_handler_pt) (void *ev);
typedef void (*connection_handle_pt)(void* ch);

// 连接的处理结构
typedef struct epoll_handle_op{
  void* data;
  event_handler_pt handler;

  unsigned                timer_set:1;   /* add timer:1, del timer: 0 */
  unsigned                active:1;      /* only for listenning fd */
  unsigned                timeout:1;

}epoll_handle_op_t;


//连接的结构
typedef struct epoll_connection_node{
  int fd;
  void *data;

  epoll_handle_op_t* read;
  epoll_handle_op_t* write;
  // cleanup
  connection_handle_pt cleanup_handler;

  unsigned int            read_timeout;
  unsigned int            write_timeout;
  unsigned int            keepalive_timeout;

    char                    buf[UPY_RECVBUF_MAX_SIZE];
    size_t                  offset;
  /* send */
  char                    send_buf[UPY_RECVBUF_MAX_SIZE];  /* write buf */
  size_t                  send_offset;
  size_t                  send_bytes;

  unsigned int            keepalive:1;
  unsigned int            close:1;
  unsigned int            destroy:1;
  unsigned int            recycle:1;

}epoll_connection_node_t;