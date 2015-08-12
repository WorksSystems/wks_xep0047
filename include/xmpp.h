#ifndef _STROPHE_HELPER_
#define _STROPHE_HELPER_

#ifdef __cplusplus
extern "C" {
#endif

#include "strophe.h"
#include "xmpp_types.h"

typedef int (*xmppconn_handler)(void *ins, xmppconn_info_t *conninfo,
                void *userdata);

xmpp_t *xmpp_new(xmppconn_handler callback, void *userdata);

void xmpp_connect(xmpp_t *xmpp, char *host, int port, char *jid, char *pass);

void xmpp_run_thread(xmpp_t *xmpp);

void xmpp_stop_thread(xmpp_t *xmpp);

void xmpp_thread_join(xmpp_t *xmpp);

int xmpp_release(xmpp_t *xmpp);

xmpp_conn_t *xmpp_get_conn(xmpp_t *xmpp);

#ifdef __cplusplus
}
#endif

#endif//_STROPHE_HELPER_
