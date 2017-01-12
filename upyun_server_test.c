#include "upyun_epoll_event.h"
#include "upyun_socket.h"
#include "upyun_epoll_manager.h"


int main(){
  upy_socket_t *s = upy_socket_malloc();
  int iret  = upy_socket_new(s);
  if(iret < 0){
    return -1;
  }
  s->port = 9999;
  upy_socket_reuseport(s->fd);
  iret = upy_socket_bind(s);
  if(iret < 0){
    return -1;
  }
  iret = upy_socket_listen(s->fd,10);
  int fd = -1;
  socklen_t            addrlen;
  struct sockaddr_in   addr;

  epoll_node_t *ep = epoll_init(1024);
  upy_server_loop(ep);
  while(1){
    fd = accept(s->fd, (struct sockaddr *)&addr, &addrlen);
    if (fd == -1) {
        if (errno == EAGAIN) {
            LOG("utun_control_server_accept: accept() not ready, fd:%d", s->fd);
            return UPY_AGAIN;
        }

        LOG( "utun_control_server_accept: accept() failed, fd:%d, %s", s->fd, strerror(errno));
        return UPY_ERROR;
    }
    upy_epoll_init_event(ep,fd);

  }

  return 0;
}