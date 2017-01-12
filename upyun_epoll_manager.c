
#include "upyun_epoll_manager.h"
#include <pthread.h>

int upy_control_server_send(epoll_connection_node_t *c)
{
      LOG("%s:%d  begin  \n",__FUNCTION__,__LINE__); 
    int   n;
    LOG( "upy_control_server_send: send() ready, fd:%d \n",  c->fd);
    for( ;; ) {
        n = send(c->fd, &c->send_buf[c->send_bytes], c->send_offset - c->send_bytes, 0);

        if (n == 0) {
            LOG( "upy_control_server_send: client closed connection, fd:%d \n",  c->fd);

           /*
            * when the file descriptor is closed, the epoll automatically deletes
            * it from its queue, so we do not need to delete explicitly the event
            * before the closing the file descriptor
            */
            close_connection_node(c);
            return UPY_ERROR;
        }

        if (n < 0) {
            if (errno == EAGAIN) {
                //upy_event_add_timer(c->write, c->write_timeout);
                return UPY_AGAIN;
            }

            LOG( "upy_control_server_send: send() failed, fd:%d, %s \n",  c->fd, strerror(errno));
           /*
            * when the file descriptor is closed, the epoll automatically deletes
            * it from its queue, so we do not need to delete explicitly the event
            * before the closing the file descriptor
            */
            close_connection_node(c);
            return UPY_ERROR;
        }

        c->send_bytes += n;

        if (c->send_bytes == c->send_offset) {
            break;
        }
    }

    LOG( "upy_control_server_send: fd:%d, send_offset:%d, send_bytes:%d \n",
                    c->fd, c->send_offset, c->send_bytes);

    c->send_offset = 0;

    return UPY_OK;
}

int upy_send_data(epoll_connection_node_t *c,char* inbuf,int inlen){
  char* data = c->send_buf;
  memcpy(data,inbuf,inlen);
  c->send_offset = inlen;
  c->send_bytes = 0;
  return UPY_OK;
}

int upy_parse_recv_data(epoll_connection_node_t *c){

  char* data = (char*)c->buf;
  if(c->offset > 0){
   // LOG("recv data :%s,len =%d \n",data,c->offset);
     upy_send_data(c,data,c->offset);
  }else {
    LOG("recv data :null,len =%d \n",c->offset);
  }


  return UPY_OK;
}

int upy_control_server_recv(epoll_connection_node_t *c)
{
    int   n, rc;

    LOG( "upy_control_server_recv fd:%d \n",  c->fd);

    while (1) {
        n = recv(c->fd, &c->buf[c->offset], UPY_RECVBUF_MAX_SIZE - c->offset, 0);

        if (n == 0) {
            LOG( "upy_control_server_recv: client closed connection, fd:%d \n",  c->fd);

           /*
            * when the file descriptor is closed, the epoll automatically deletes
            * it from its queue, so we do not need to delete explicitly the event
            * before the closing the file descriptor
            */
            close_connection_node(c);
            return UPY_ERROR;
        }

        if (n < 0) {
            if (errno == EAGAIN) {
               // upy_event_add_timer(c->read, c->read_timeout);
                return UPY_AGAIN;
            }

            LOG( "upy_control_server_recv: recv() failed, fd:%d, %s \n", c->fd, strerror(errno));
           /*
            * when the file descriptor is closed, the epoll automatically deletes
            * it from its queue, so we do not need to delete explicitly the event
            * before the closing the file descriptor
            */
            close_connection_node(c);
            return UPY_ERROR;
        }
    LOG("%s:%d  recv data :len = %d \n",__FUNCTION__,__LINE__,n); 
        c->offset += n;
        // parse recv and send
        upy_parse_recv_data(c);
    }

    c->offset = 0;
    LOG("%s:%d  begin response \n",__FUNCTION__,__LINE__); 
    upy_control_server_send(c);
    return UPY_OK;
}

void handle_function (void *ev)
{
    LOG("%s:%d  begin \n",__FUNCTION__,__LINE__);   
  if(NULL == ev){
    LOG("%s:%d  param error = NULL \n",__FUNCTION__,__LINE__); 
    return ;
  }
  epoll_handle_op_t* ev_op = (epoll_handle_op_t*)ev;
  epoll_connection_node_t *node = ev_op->data;
  LOG("%s:%d  begin 2 \n",__FUNCTION__,__LINE__); 
  if(ev_op->timeout){
    LOG("read/write timeout, fd:%d, ignore\n",node->fd);
    return;
  }
  if(node->close){
    close_connection_node(node);
    return ;
  }
    /* read or write timer */
   // upy_event_del_timer(ev);

    if (node->send_offset) {
        LOG("handle_function: fd:%d, write handler \n",  node->fd);
        upy_control_server_send(node);
    } else {
        LOG("handle_function: fd:%d, read handler \n",  node->fd);
        upy_control_server_recv(node);
    }


}

void cleanup_handler_function (void *ev)
{
  if(NULL == ev){
    return ;
  }
  epoll_handle_op_t* ev_op = (epoll_handle_op_t*)ev;

}
void upy_epoll_empty_handler(void *ev)
{
    LOG("upy_epoll_empty_handler[%d] \n");

    return;
}

void upy_epoll_init_event(epoll_node_t *ep,int fd){
   struct epoll_event    ee;

   epoll_connection_node_t* connection_node = create_connection_node(fd);
   ee.events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLRDHUP;
   ee.data.fd = fd;
    connection_node->fd = fd;
   ee.data.ptr = (void*)connection_node; 
    if (epoll_ctl(ep->epoll_ep, EPOLL_CTL_ADD, connection_node->fd, &ee) == -1) {
        LOG("epoll_ctl(EPOLL_CTL_ADD, %d) failed \n", connection_node->fd);
        goto failed;
    }
    LOG("%s:%d  epoll_ctl add success \n",__FUNCTION__,__LINE__);
    connection_node->read->handler = upy_epoll_empty_handler;
    connection_node->write->handler = upy_epoll_empty_handler;
   int iret = upy_socket_nonblocking(fd);

    // add to epoll
    connection_node->read->handler = handle_function;
    connection_node->write->handler = handle_function;
    connection_node->cleanup_handler = cleanup_handler_function;
    connection_node->read_timeout = 1000;
    connection_node->write_timeout = 1000;

    handle_function(connection_node->read);
    return;
failed:
  close_connection_node(connection_node);
  return ;
}

void* loop_main(void* param){

  while(1){
      epoll_process_events(param);
      usleep(1);
  }

}

int upy_server_loop(epoll_node_t *ep){
    pthread_t tcp_recv_thread1;
    pthread_create(&tcp_recv_thread1, NULL, loop_main, ep);
    pthread_detach(tcp_recv_thread1);
#if 0
	GError *error = NULL;
	// create new thread for send test
	GThread *g_send_thread = g_thread_try_new(
	    "LoopMain", &loop_main, (void*)ep, &error);
	if (error != NULL) {
		JANUS_LOG(
		    LOG_ERR,
		    "package Got error  trying to launch the send handler "
		    "thread...\n",
		    error->code, error->message ? error->message : "??");
		return -1;
	}
#endif    
  return UPY_OK;
}