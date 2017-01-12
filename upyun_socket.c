
/*
 * Copyright (C) yan.sheng@upai.com(cjhust)
 */


#include "upyun_socket.h"

#if 0
int upy_atoi(char *line, size_t n){
    utun_int_t  value, cutoff, cutlim;

    if (n == 0) {
        return UTUN_ERROR;
    }

    if (UTUN_PTR_SIZE == 4) {
        cutoff = NGX_MAX_INT32_T_VALUE / 10;
        cutlim = NGX_MAX_INT32_T_VALUE % 10;
    } else {
        cutoff = NGX_MAX_INT64_T_VALUE / 10;
        cutlim = NGX_MAX_INT64_T_VALUE % 10;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return UTUN_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim)) {
            return UTUN_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    return value;
}

upy_socket_t *
upy_socket_malloc()
{
    upy_socket_t  *s;

    s = (upy_socket_t *)malloc(sizeof(upy_socket_t));
    if (s == NULL) {
        return NULL;
    }
    memset(s, 0, sizeof(upy_socket_t));
    return s;
}


void
upy_socket_free(upy_socket_t *s)
{
    if (s == NULL) {
        return;
    }

    if (s->fd != -1) {
        upy_socket_close( s->fd);
    }

    free(s);
    s = NULL;
}


int
upy_socket_new(upy_socket_t *s)
{
    s->fd = socket(s->family, s->type, 0);
    if (s->fd == -1) {
        return UPY_ERROR;
    }

    return UPY_OK;
}








int
upy_socket_reuseaddr( int fd)
{
    int optval = 1;

    /* time_wait optimize */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        LOG( "socket reuseaddr() failed, %s", strerror(errno));
        return UPY_ERROR;
    }

    return UPY_OK;
}


int
upy_socket_parse_url(upy_socket_t *s, char *addr)
{
    int      n, host_len;
    char    *p;

    host_len = strlen(addr);
    memcpy(s->addr_text, addr, host_len);

    /* IPV4 addr */
    s->family = AF_INET;
    p = strchr(addr, ':');
    if (p == NULL) {

        if (strchr(addr, '.') != NULL ) {    /* ip only */
            s->port = 80;
        } else {                             /* port only */
            s->wildcard = 1;
            n = upy_atoi(addr, host_len);
            if (n == UPY_ERROR) {
                LOG( "inet addr %s ERROR", addr);
                return UPY_ERROR;
            }

            if (n < 1 || n > 65535) {
                LOG( "inet addr %s ERROR", addr);
                return UPY_ERROR;
            }

            s->port = n;
        }
    } else {                                 /* ip + port mode */
        host_len = p - addr;

        p++;
        s->port = atoi(p);
    }

    memcpy(s->url, addr, host_len);

    return UPY_OK;
}


int
upy_socket_parse_announce(struct sockaddr_in *addr, char *host)
{
    int     host_len;
    char   *p, buf[upy_NAME_MAX_LEN];

    host_len = strlen(host);

    p = strchr(host, ':');
    if (p == NULL) {

        if (strchr(host, '.') != NULL ) {    /* ip only */
            addr->sin_port = 80;
        } else {                             /* port only */
            LOG( "announce addr not support port-only mode");
            return UPY_ERROR;
        }
    } else {                                 /* ip + port mode */
        host_len = p - host;

        p++;
        addr->sin_port = atoi(p);
    }

    memcpy(buf, host, host_len);
    buf[host_len] = '\0';

    if (inet_pton(AF_INET, buf, &addr->sin_addr) != 1) {
        LOG( "socket inet_pton() failed, %s", strerror(errno));
        return UPY_ERROR;
    }
    addr->sin_addr.s_addr = ntohl(addr->sin_addr.s_addr);

    return UPY_OK;
}


int
upy_socket_inet_addr(upy_socket_t *s)
{
    struct sockaddr_in  *in;

    if (s->url == NULL){
        return -1;
    }

    in = &s->addr;

    in->sin_family = s->family;
    in->sin_port = htons(s->port);
    if (s->wildcard == 1) {
        in->sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(s->family, s->url, &in->sin_addr) != 1) {
            LOG( "socket inet_pton() failed, %s", strerror(errno));
            return -2;
        }
    }
    s->addrlen = sizeof(s->addr);

    return UPY_OK;
}


int
upy_socket_bind(upy_socket_t *s)
{
    if (upy_socket_inet_addr(s) != UPY_OK) {
        return -1;
    }

    if (bind(s->fd, (struct sockaddr *)&s->addr, s->addrlen) == -1) {
        LOG( "socket bind() failed, %s", strerror(errno));
        return -2;
    }

    return UPY_OK;
}


int
upy_socket_listen( int fd, int backlog)
{
    if (listen(fd, backlog) != 0) {
        LOG( "socket listen() failed, %s", strerror(errno));
        return UPY_ERROR;
    }

    return UPY_OK;
}


upy_socket_t *
upy_socket_package( int family, int type)
{
    upy_socket_t    *s;

    s = upy_socket_malloc();
    if (s == NULL) {
        return NULL;
    }

    s->family = family;
    s->type = type;

    if (upy_socket_new(s) != UPY_OK) {
        goto failed;
    }

    if (upy_socket_nonblocking( s->fd) != UPY_OK) {
        goto failed;
    }

    return s;

failed:
    upy_socket_free(s);
    return NULL;
}


int
upy_socket_udp_send( struct sockaddr_in *addr, char *addr_text, char *data, int len, unsigned int thread_index)
{
    int  fd, n;

    addr->sin_family = AF_INET;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        LOG( "upy_socket_udp_send[%u]: socket() failed, %s", thread_index, strerror(errno));
        return UPY_ERROR;
    }

    n = sendto(fd, data, len, 0, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
    if (n != len) {
        LOG( "upy_socket_udp_send[%u]: sendto() failed, n:%d, len:%d, %s", thread_index, n, len, strerror(errno));
        return UPY_ERROR;
    }

    LOG( "upy_socket_udp_send[%u]: sendto %s success, len:%d", thread_index, addr_text, len);
    close(fd);

    return n;
}

#endif


upy_socket_t *
upy_socket_malloc()
{
    upy_socket_t  *s;

    s = (upy_socket_t *)malloc(sizeof(upy_socket_t));
    if (s == NULL) {
        return NULL;
    }
    memset(s, 0, sizeof(upy_socket_t));
    s->type = SOCK_STREAM;
    s->family = AF_INET;
    s->wildcard = 1;
    return s;
}


int
upy_socket_new(upy_socket_t *s)
{
    s->fd = socket(s->family, s->type, 0);
    if (s->fd == -1) {
        return UPY_ERROR;
    }

    return UPY_OK;
}
int
upy_socket_inet_addr(upy_socket_t *s)
{
    struct sockaddr_in  *in;

    if (s->url == NULL){
        return -1;
    }

    in = &s->addr;

    in->sin_family = s->family;
    in->sin_port = htons(s->port);
    if (s->wildcard == 1) {
        in->sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(s->family, s->url, &in->sin_addr) != 1) {
            LOG( "socket inet_pton() failed, %s", strerror(errno));
            return -2;
        }
    }
    s->addrlen = sizeof(s->addr);

    return UPY_OK;
}


int
upy_socket_bind(upy_socket_t *s)
{
    if (upy_socket_inet_addr(s) != UPY_OK) {
        return -1;
    }

    if (bind(s->fd, (struct sockaddr *)&s->addr, s->addrlen) == -1) {
        LOG( "socket bind() failed, %s \n", strerror(errno));
        return -2;
    }

    return UPY_OK;
}


int
upy_socket_listen( int fd, int backlog)
{
    if (listen(fd, backlog) != 0) {
        LOG( "socket listen() failed, %s\n", strerror(errno));
        return UPY_ERROR;
    }

    return UPY_OK;
}
int upy_socket_reuseport(int fd)
{
    int optval = 1;

    /* multi-threads listen at same time */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        LOG( "socket reuseport() failed, %s\n", strerror(errno));
        return UPY_ERROR;
    }

    return UPY_OK;
}
int upy_socket_nonblocking( int fd)
{
    int opt;

    opt = fcntl(fd, F_GETFL);
    if (opt < 0) {
        LOG( "socket nonblocking() failed, %s\n", strerror(errno));
        return UPY_ERROR;
    }

    opt |= O_NONBLOCK;

     if (fcntl(fd, F_SETFL, opt) < 0) {
         LOG( "socket nonblocking() failed, %s\n", strerror(errno));
        return UPY_ERROR;
    }

    return UPY_OK;
}
void
upy_socket_close( int fd)
{
    if (fd < 0) {
        return;
    }

    close(fd);
}
