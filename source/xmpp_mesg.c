#include <string.h>
#include <xmpp_mesg.h>
#include "xmpp_common.h"

typedef struct _xmppmesg_userdata_t
{
    xmppmesg_handler handler;
    void *userdata;
} xmppmesg_userdata_t;

static xmppmesg_userdata_t s_mesg_udata;

int xmppmesg_send_message(xmpp_conn_t *conn, xmppdata_t *xdata)
{
    xmpp_stanza_t *szmsg, *szbody, *sztext;
    xmpp_ctx_t *ctx;

    ctx = xmpp_conn_get_context(conn);

    sztext = xmpp_stanza_new(ctx);
    xmpp_stanza_set_text(sztext, xdata->data);

    szbody = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(szbody, "body");
    xmpp_stanza_add_child(szbody, sztext);

    szmsg = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(szmsg, "message");
    xmpp_stanza_set_attribute(szmsg, "to", xdata->tojid);
    xmpp_stanza_add_child(szmsg, szbody);

    xmpp_send(conn, szmsg);
    xmpp_stanza_release(szmsg);

    return 0;
}

static int _mesg_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmppdata_t xdata;
    char *intext;

    xmppmesg_userdata_t *udata = (xmppmesg_userdata_t *) userdata;

    if (!xmpp_stanza_get_child_by_name(stanza, "body"))
        return 1;
    if (xmpp_stanza_get_attribute(stanza, "type") != NULL && !strcmp(xmpp_stanza_get_attribute(stanza, "type"), "error"))
        return 1;
    intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "body"));

    //printf("Incoming message from %s: %s\n", xmpp_stanza_get_attribute(stanza, "from"), intext);
    xdata.from = (char *) xmpp_stanza_get_attribute(stanza, "from");
    xdata.data = (void *) intext;
    if (udata != NULL && udata->handler != NULL)
        udata->handler(conn, &xdata, udata->userdata);
    return 1;
}

void xmppmesg_handler_add(xmpp_conn_t *conn, xmppmesg_handler handler, void *userdata)
{
    //xmppmesg_userdata_t *udata;
    //udata = (xmppmesg_userdata_t *) malloc(sizeof(xmppmesg_userdata_t));
    s_mesg_udata.handler = handler;
    s_mesg_udata.userdata = userdata;
    xmpp_handler_add(conn, _mesg_handler, NULL, "message", "", &s_mesg_udata);
}

void xmppmesg_handler_del(xmpp_conn_t *conn, xmppmesg_handler handler)
{
    xmpp_handler_delete(conn, _mesg_handler);
}
