#pragma once

#include "upyun_epoll_common.h"
#if 0
typedef struct upy_socket {

    int                  fd;                          /* socket() or */
    int                  family;                      /* AF_INET, AF_UNIX */
    int                  type;                        /* SOCK_STREAM, SOCK_DGRAM */

    char                 announce_url[upy_NAME_MAX_LEN];
    uint16_t             announce_port;

    char                 addr_text[upy_NAME_MAX_LEN];
    char                 url[upy_NAME_MAX_LEN];      /* listen IP item */
    uint16_t             port;                        /* listen/server PORT item */

    struct sockaddr_in   addr;
    socklen_t            addrlen;
}upy_socket_t;




upy_socket_t *upy_socket_malloc();
void upy_socket_free(upy_socket_t *s);
int upy_socket_new(upy_socket_t *s);

int upy_socket_nonblocking( int fd);
int upy_socket_reuseport( int fd);
int upy_socket_reuseaddr( int fd);
int upy_socket_parse_url(upy_socket_t *s, char *addr);
int upy_socket_parse_announce(struct sockaddr_in *addr, char *host);
int upy_socket_inet_addr(upy_socket_t *s);
int upy_socket_bind(upy_socket_t *s);
int upy_socket_listen( int fd, int backlog);
upy_socket_t *upy_socket_package( int family, int type);
int upy_socket_udp_send( struct sockaddr_in *addr, char *addr_text, char *data, int len, unsigned int thread_index);

#endif

typedef struct upy_socket {
  int                 fd;
  int                  family;                      /* AF_INET, AF_UNIX */
  int                  type;                        /* SOCK_STREAM, SOCK_DGRAM */
  int                  wildcard;        // addr any

  char url[16];     // ip
  int  port;
  struct sockaddr_in   addr;
  socklen_t            addrlen;
}upy_socket_t;


upy_socket_t *upy_socket_malloc();
int upy_socket_new(upy_socket_t *s);
int upy_socket_bind(upy_socket_t *s);
int upy_socket_listen( int fd,int num);
int upy_socket_reuseport( int fd);
int upy_socket_nonblocking( int fd);
void upy_socket_close( int fd);