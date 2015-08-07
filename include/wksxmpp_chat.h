#ifndef _STROPHE_CHAT_
#define _STROPHE_CHAT_

#ifdef __cplusplus
extern "C" {
#endif

#include "strophe.h"
#include "wksxmpp_types.h"

typedef int (*wksxmpp_chat_handler)(xmpp_conn_t *conn,
            wksxmpp_data_t *msgdata, void *udata);

int wksxmpp_chat_send_message(xmpp_conn_t *conn, wksxmpp_data_t *msgdata);

void wksxmpp_chat_handler_add(xmpp_conn_t *conn, wksxmpp_chat_handler handler,
        void *udata);

void wksxmpp_chat_handler_del(xmpp_conn_t *conn, wksxmpp_chat_handler handler);

#ifdef __cplusplus
}
#endif

#endif//_STROPHE_CHAT_
