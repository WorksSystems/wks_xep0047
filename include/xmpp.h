#ifndef _STROPHE_HELPER_
#define _STROPHE_HELPER_

#ifdef __cplusplus
extern "C" {
#endif

#include "strophe.h"
#include "xmpp_types.h"

typedef int (*xmppconn_handler)(void *ins, xmppconn_info_t *conninfo,
                void *userdata);

void *xmpp_new(xmppconn_handler callback, void *userdata);

void xmpp_connect(void *ins, char *host, int port, char *jid, char *pass);

void xmpp_run_thread(void *ins);

void xmpp_stop_thread(void *ins);

void xmpp_thread_join(void *ins);

int xmpp_release(void *ins);

xmpp_conn_t *xmpp_get_conn(void *ins);

#ifdef __cplusplus
}
#endif

#endif//_STROPHE_HELPER_
