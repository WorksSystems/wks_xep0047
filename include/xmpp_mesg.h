/*
 * xmpp_mesg.h
 *
 *  Created on: Oct 27, 2016
 *      Author: longer
 */

#ifndef INCLUDE_XMPP_MESG_H_
#define INCLUDE_XMPP_MESG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "strophe.h"
#include "xmpp_types.h"

typedef int (*xmppmesg_handler)(xmpp_conn_t *conn, xmppdata_t *msgdata, void *udata);

int xmppmesg_send_message(xmpp_conn_t *conn, xmppdata_t *msgdata);

void xmppmesg_handler_add(xmpp_conn_t *conn, xmppmesg_handler handler, void *udata);

void xmppmesg_handler_del(xmpp_conn_t *conn, xmppmesg_handler handler);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XMPP_MESG_H_ */
