#ifndef _WKS_XMPP_COMMON_
#define _WKS_XMPP_COMMON_

#include <pthread.h>

#include "strophe.h"
#include "wksxmpp.h"

#define XMLNS_PING "urn:xmpp:ping"

typedef struct _wksxmpp_t {
    xmpp_ctx_t  *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t  *log;
    pthread_t    pth;
    wksxmpp_conn_handler callback;
    void        *userdata;
} wksxmpp_t;

/**
 *
 * @param conn conn for libstrophe
 * @param to to target, if null or "", will not specific target.
 */
void wksxmpp_presence(xmpp_conn_t *conn, char *to);

void wksxmpp_ping(xmpp_conn_t* conn, char* const xmpp_server);

#endif//_WKS_XMPP_COMMON_
