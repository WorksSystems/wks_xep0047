#include <string.h>

#include "wksxmpp_common.h"
#include "wksxmpp_chat.h"

typedef struct _wksxmpp_chat_userdata_t {
    wksxmpp_chat_handler handler;
    void *userdata;
} wksxmpp_chat_userdata_t;

static wksxmpp_chat_userdata_t s_chat_udata;

int wksxmpp_chat_send_message(xmpp_conn_t *conn, wksxmpp_data_t *wksdata)
{
    xmpp_stanza_t *szmsg, *szbody, *sztext;
    xmpp_ctx_t *ctx;

    ctx = xmpp_conn_get_context(conn);

    sztext = xmpp_stanza_new(ctx);
    xmpp_stanza_set_text(sztext, wksdata->data);

    szbody = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(szbody, "body");
    xmpp_stanza_add_child(szbody, sztext);

    szmsg = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(szmsg, "message");
    xmpp_stanza_set_type(szmsg, "chat");
    xmpp_stanza_set_attribute(szmsg, "to", wksdata->tojid);
    xmpp_stanza_add_child(szmsg, szbody);

    xmpp_send(conn, szmsg);
    xmpp_stanza_release(szmsg);

    return 0;
}

static int _chat_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza,
        void * const userdata)
{
    wksxmpp_data_t wksdata;
    char *intext;

    wksxmpp_chat_userdata_t *udata = (wksxmpp_chat_userdata_t *) userdata;

    if(!xmpp_stanza_get_child_by_name(stanza, "body")) return 1;
    if(xmpp_stanza_get_attribute(stanza, "type") !=NULL &&
            !strcmp(xmpp_stanza_get_attribute(stanza, "type"), "error"))
        return 1;
    intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "body"));

    printf("Incoming message from %s: %s\n", xmpp_stanza_get_attribute(stanza, "from"), intext);
    wksdata.from = xmpp_stanza_get_attribute(stanza, "from");
    wksdata.data = (void *) intext;
    udata->handler(conn, &wksdata, udata->userdata);
    return 1;
}

void wksxmpp_chat_handler_add(xmpp_conn_t *conn, wksxmpp_chat_handler handler,
        void *userdata)
{
    //wksxmpp_chat_userdata_t *udata;
    //udata = (wksxmpp_chat_userdata_t *) malloc(sizeof(wksxmpp_chat_userdata_t));
    s_chat_udata.handler = handler;
    s_chat_udata.userdata = userdata;
    xmpp_handler_add(conn, _chat_handler, NULL, "message", "chat", &s_chat_udata);
}

void wksxmpp_chat_handler_del(xmpp_conn_t *conn, wksxmpp_chat_handler handler)
{
    xmpp_handler_delete(conn, _chat_handler);
}
