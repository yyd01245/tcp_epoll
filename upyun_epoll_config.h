
#pragma once

#include "upyun_epoll_common.h"


enum {
    UTUN_CONFIG_SECTION_REQUIRED,
    UTUN_CONFIG_SECTION_OPTIONAL
};


typedef struct upy_config_section_s {
    const char                     *name;
    int                           (*handler)(json_t *setting, utun_config_t *cfg);
    int                             option;
} upy_config_section_t;


typedef struct utun_config_core_s {
    int                             daemon;
    int                             nthreads;
    int                             epoll_events;
    int                             max_connections;   /* max streams */

    int                             thread_connections;
} utun_config_core_t;



