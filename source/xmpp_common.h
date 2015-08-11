#ifndef __XMPP_COMMON__
#define __XMPP_COMMON__

#include <pthread.h>

#include "../include/xmpp.h"
#include "strophe.h"

#define XMLNS_PING "urn:xmpp:ping"

typedef struct _xmpp_t {
    xmpp_ctx_t  *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t  *log;
    pthread_t    pth;
    xmppconn_handler callback;
    void        *userdata;
} xmpp_t;

/**
 *
 * @param conn conn for libstrophe
 * @param to to target, if null or "", will not specific target.
 */
void xmpp_presence(xmpp_conn_t *conn, char *to);

void xmpp_ping(xmpp_conn_t* conn, char* const id, char * const to, char * const type);

#endif//__XMPP_COMMON__
