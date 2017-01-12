CC = gcc
CFLAGS = -g -O0 -Wall -pthread -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu/  -lglib-2.0
INCLUDE = -I./ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LIB_BASE = upyun_epoll_event.o  upyun_socket.o upyun_epoll_manager.o

OBJ = server

all: $(OBJ)

server: upyun_server_test.o $(LIB_BASE)
	$(CC) $(CFLAGS) -o $@ $^
upyun_server_test.o: upyun_server_test.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^

upyun_epoll_event.o: upyun_epoll_event.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^
upyun_epoll_common.o: upyun_epoll_common.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^
upyun_socket.o: upyun_socket.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^
upyun_epoll_manager.o: upyun_epoll_manager.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^


clean: 
	rm -f $(OBJ) *.o
  
