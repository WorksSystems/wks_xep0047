/*
 * wksxmpp_common.c
 *
 *  Created on: Aug 7, 2015
 *      Author: user
 */
#include <string.h>
#include "wksxmpp_common.h"

void wksxmpp_presence(xmpp_conn_t *conn, char *to)
{
    xmpp_ctx_t      *ctx;
    xmpp_stanza_t   *szpres;

    ctx = xmpp_conn_get_context(conn);
    szpres = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(szpres, "presence");
    if (to != NULL && strlen(to) > 0) {
        xmpp_stanza_set_attribute(szpres, "to", to);
    }
    xmpp_send(conn, szpres);
    xmpp_stanza_release(szpres);
}

void wksxmpp_ping(xmpp_conn_t* conn, char* const xmpp_server)
{

    xmpp_stanza_t *iq, *ping;
    xmpp_ctx_t *ctx;

    ctx = xmpp_conn_get_context(conn);

    iq = xmpp_stanza_new(ctx);
    ping = xmpp_stanza_new(ctx);

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "get");
    xmpp_stanza_set_id(iq, xmpp_conn_get_jid(conn));

    xmpp_stanza_set_name(ping, "ping");
    xmpp_stanza_set_ns(ping, XMLNS_PING);
    xmpp_stanza_set_attribute(ping, "from", xmpp_conn_get_jid(conn));
    xmpp_stanza_set_attribute(ping, "to", xmpp_server);

    xmpp_stanza_add_child(iq, ping);

    xmpp_send(conn, iq);
    xmpp_stanza_release(ping);
    xmpp_stanza_release(iq);

}

