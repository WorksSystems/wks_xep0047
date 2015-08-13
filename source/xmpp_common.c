/*
 * xmpp_common.c
 *
 *  Created on: Aug 7, 2015
 *      Author: user
 */
#include <string.h>

#include "xmpp_common.h"

void xmpp_presence(xmpp_conn_t *conn, char *to)
{
    xmpp_ctx_t *ctx;
    xmpp_stanza_t *szpres;

    ctx = xmpp_conn_get_context(conn);
    szpres = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(szpres, "presence");
    if (to != NULL && strlen(to) > 0) {
        xmpp_stanza_set_attribute(szpres, "to", to);
    }
    xmpp_send(conn, szpres);
    xmpp_stanza_release(szpres);
}

void xmpp_ping(xmpp_conn_t* conn, char* const id, char * const to, char * const type)
{
    if (type != NULL && strcmp(type, "result") == 0) {
        xmpp_iq_ack_result(conn, id, to);
    } else {
        xmpp_stanza_t *iq = NULL, *ping = NULL;
        xmpp_ctx_t *ctx;
        const char *jid = xmpp_conn_get_bound_jid(conn);

        ctx = xmpp_conn_get_context(conn);
        iq = xmpp_stanza_new(ctx);

        xmpp_stanza_set_name(iq, "iq");
        if (type != NULL && strlen(type) > 0) {
            xmpp_stanza_set_type(iq, type);
        } else {
            xmpp_stanza_set_type(iq, "get");
        }
        if (id != NULL && strlen(id) > 0) {
            xmpp_stanza_set_id(iq, id);
        } else {
            xmpp_stanza_set_id(iq, jid);
        }
        xmpp_stanza_set_attribute(iq, "from", jid);
        xmpp_stanza_set_attribute(iq, "to", to);

        ping = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(ping, "ping");
        xmpp_stanza_set_ns(ping, XMLNS_PING);
        xmpp_stanza_add_child(iq, ping);

        xmpp_send(conn, iq);
        if (ping != NULL)
            xmpp_stanza_release(ping);
        if (iq != NULL)
            xmpp_stanza_release(iq);
    }
}

void xmpp_iq_ack_result(xmpp_conn_t * const conn, char * const id, char * const to)
{
    xmpp_stanza_t *iq = NULL;
    xmpp_ctx_t *ctx;
    //fprintf(stderr, "%s-%d %s(%p, %s, %s)", __FILE__, __LINE__, __FUNCTION__, conn, id, to);
    ctx = xmpp_conn_get_context(conn);
    iq = xmpp_stanza_new(ctx);

    if (iq == NULL || to == NULL) {
        fprintf(stderr, "%s() failed.", __FUNCTION__);
        return;
    }

    if (iq != NULL) {
        xmpp_stanza_set_name(iq, "iq");
        xmpp_stanza_set_type(iq, "result");
        xmpp_stanza_set_id(iq, id);
        xmpp_stanza_set_attribute(iq, "from", xmpp_conn_get_bound_jid(conn));
        xmpp_stanza_set_attribute(iq, "to", to);
        xmpp_send(conn, iq);
        xmpp_stanza_release(iq);
    }
}
