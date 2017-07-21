#include <string.h>
#include <xmpp_chat.h>
#include "xmpp_common.h"

typedef struct _xmppchat_userdata_t
{
    xmppchat_handler handler;
    void *userdata;
} xmppchat_userdata_t;

static xmppchat_userdata_t s_chat_udata;

int xmppchat_send_message(xmpp_conn_t *conn, xmppchat_t *xdata)
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
    xmpp_stanza_set_type(szmsg, "chat");
    xmpp_stanza_set_attribute(szmsg, "to", xdata->tojid);
    xmpp_stanza_add_child(szmsg, szbody);

    if (xdata->subject != NULL && strlen(xdata->subject) > 0) {
        xmpp_stanza_t *szsub, *szstext;
        szstext = xmpp_stanza_new(ctx);
        xmpp_stanza_set_text(szstext, xdata->subject);

        szsub = xmpp_stanza_new(ctx);
        xmpp_stanza_set_name(szsub, "subject");
        xmpp_stanza_add_child(szsub, szstext);
        xmpp_stanza_add_child(szmsg, szsub);
    }

    xmpp_send(conn, szmsg);
    xmpp_stanza_release(szmsg);

    return 0;
}

static int _chat_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmppchat_t xdata;
    char *intext;
    char *subject;
    //char *thread;

    xmppchat_userdata_t *udata = (xmppchat_userdata_t *) userdata;

    if (!xmpp_stanza_get_child_by_name(stanza, "body"))
        return 1;
    if (xmpp_stanza_get_attribute(stanza, "type") != NULL && !strcmp(xmpp_stanza_get_attribute(stanza, "type"), "error"))
        return 1;
    intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "body"));
    subject = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "subject"));

    //printf("Incoming message from %s: %s\n", xmpp_stanza_get_attribute(stanza, "from"), intext);
    xdata.from = (char *) xmpp_stanza_get_attribute(stanza, "from");
    xdata.data = (void *) intext;
    xdata.subject = (void *) subject;
    if (udata != NULL && udata->handler != NULL)
        udata->handler(conn, &xdata, udata->userdata);
    return 1;
}

void xmppchat_handler_add(xmpp_conn_t *conn, xmppchat_handler handler, void *userdata)
{
    //xmppchat_userdata_t *udata;
    //udata = (xmppchat_userdata_t *) malloc(sizeof(xmppchat_userdata_t));
    s_chat_udata.handler = handler;
    s_chat_udata.userdata = userdata;
    xmpp_handler_add(conn, _chat_handler, NULL, "message", "chat", &s_chat_udata);
}

void xmppchat_handler_del(xmpp_conn_t *conn, xmppchat_handler handler)
{
    xmpp_handler_delete(conn, _chat_handler);
}
