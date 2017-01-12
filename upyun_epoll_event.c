
#include "upyun_epoll_event.h"


epoll_node_t* epoll_init(int event_num){
  epoll_node_t *ep_node = (epoll_node_t*)malloc(sizeof(epoll_node_t));
  ep_node->epoll_ep = epoll_create(1024);
  if(ep_node->epoll_ep == -1){
    LOG("epoll_create failed \n");
    return NULL;
  }
  ep_node->epv_num = event_num;
  ep_node->epv_list = (struct epoll_event *)malloc(sizeof(struct epoll_event)*event_num );

  return ep_node;
}

epoll_connection_node_t* create_connection_node(int fd){
  epoll_connection_node_t* node = (epoll_connection_node_t*)malloc(sizeof(epoll_connection_node_t));
  if(NULL == node){
    LOG("malloc connection node error \n");
    return NULL;
  }
  memset(node,0,sizeof(epoll_connection_node_t));
  node->fd = fd;
  node->read = (epoll_handle_op_t*)malloc(sizeof(epoll_handle_op_t));
  node->write = (epoll_handle_op_t*)malloc(sizeof(epoll_handle_op_t));
  node->read->data = node;
  node->write->data = node;

  return node;

}

void close_connection_node(epoll_connection_node_t* node){
    if (node->recycle == 1) {
        return;
    }

    if (node->fd != -1) {
        LOG( "close_connection_node: fd:%d closed\n", node->fd);
        upy_socket_close(node->fd);
        node->fd = -1;
        node->close = 1;
    }
}

int epoll_add_listen(int listenfd){
   struct epoll_event    ee;


   epoll_connection_node_t* connection_node = create_connection_node(listenfd);
   ee.events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLRDHUP;
   ee.data.fd = listenfd;
   ee.data.ptr = (void*)connection_node; 


}

int epoll_process_events(void* userdata){
  epoll_node_t* ep_node = (epoll_node_t*)userdata;
  if(!ep_node){
    LOG("param error \n");
    return -1;
  }
  int event_sizes = epoll_wait(ep_node->epoll_ep,ep_node->epv_list,ep_node->epv_num,EP_TIMEOUT);
  if(event_sizes == -1){
    LOG("epoll wait size == -1 \n");
    return -1;
  }
  if(event_sizes == 0){
    //LOG("epoll wait size == 0 \n");
    return -2;
  }
  epoll_connection_node_t* connection_node = NULL;
  uint32_t            revents;
  epoll_handle_op_t *recv_op;
  epoll_handle_op_t *write_op;

  for(int i = 0;i<event_sizes; ++i){
    connection_node = ep_node->epv_list[i].data.ptr;
    if(connection_node->fd == -1){
      LOG("connection fd == -1\n");
      continue;
    }
    revents = ep_node->epv_list[i].events;
    if (revents & (EPOLLERR|EPOLLHUP)) {
      LOG( "epoll_wait() error on fd: %d\n", connection_node->fd);
    }

    recv_op = connection_node->read;
    /*
      * if the error events were returned without EPOLLIN or EPOLLOUT,
      * then add these flags to handle the events at least in one
      * active handler
    */
    if ((revents & (EPOLLERR|EPOLLHUP))
          && (revents & (EPOLLIN|EPOLLOUT)) == 0) {
        revents |= EPOLLIN|EPOLLOUT;
    }
    if (revents & EPOLLIN) {
        recv_op->timeout = 0;
        recv_op->handler(recv_op);
    }

    write_op = connection_node->write;
    if(write_op == NULL ){
      LOG("write_op->handler \n");
      return -3;
    }
    if( write_op->handler == NULL){
      LOG("write_op->handler NULL \n");
      return -4;      
    }
    if (revents & EPOLLOUT) {
        write_op->timeout = 0;
        LOG("write_op addr %p \n",write_op);
        write_op->handler(write_op);
    }
  }


}