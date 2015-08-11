#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <strophe.h>

#include "xmppclient.h"
#include "xmpp_ibb.h"

#include "xmpp_common.h"

time_t glast_ping_time;

static int _ping_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    char *to;
    char *id;
    printf("_ping_handler()\n");
    to = xmpp_stanza_get_attribute(stanza, "from");
    id = xmpp_stanza_get_attribute(stanza, "id");
    xmpp_ping(conn, id, to, "result");
    //xmpp_ping(conn, NULL, xmpp_stanza_get_attribute(stanza, "from"), NULL);
    time(&glast_ping_time);
    
    return 1;
}
#if 0
static int _presence_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    char *topres;

    printf("presence handler\n");
    time(&glast_ping_time);
    topres = xmpp_stanza_get_attribute(stanza, "from");

    if (strcmp(topres, xmpp_conn_get_jid(conn)) == 0) {
        printf("Get Presence of myself, return\n");
        return 1;
    }

    xmpp_presence(conn, topres);

    return 1;
}
#endif
static int _message_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t*) userdata;
    printf("_message_handler\n");
    char *intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "body"));

    printf("Get message body=\n%s\n", intext);

    xmpp_free(ctx, intext);

    return 1;

}

/* define a handler for connection events */
void conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status, const int error, xmpp_stream_error_t * const stream_error, void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t *) userdata;
//    XMPP_XEP_Ops_t *xmpp_xep_ops_p = (XMPP_XEP_Ops_t *)userdata;    

    if (status == XMPP_CONN_CONNECT) {
        fprintf(stderr, "connected Received : [%s]\n", (char *) userdata);

        xmpp_presence(conn, NULL);

        xmpp_handler_add(conn, _ping_handler, XMLNS_PING, "iq", "get", ctx);
        xmpp_handler_add(conn, _message_handler, NULL, "message", NULL, ctx);

    } else {
        fprintf(stderr, "DEBUG: disconnected\n");
    }

}

xmpp_conn_t* XMPP_Init(char* jid, char* pass, char* host, xmpp_ctx_t **pctx)
{
    xmpp_conn_t *conn;
    xmpp_log_t *log;

    //printf("jid=[%s] pass=[%s] host=[%s]", jid, pass, host);
    xmpp_initialize();

    /* create a context */
    log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG); /* pass NULL instead to silence output */
    *pctx = xmpp_ctx_new(NULL, log);

    /* create a connection */
    conn = xmpp_conn_new(*pctx);
    /* setup authentication information */
    xmpp_conn_set_jid(conn, jid);
    xmpp_conn_set_pass(conn, pass);

    /* initiate connection */
    xmpp_connect_client(conn, host, 0, conn_handler, *pctx);

    return conn;
}

void XMPP_Close(xmpp_conn_t *conn, xmpp_ctx_t *ctx)
{

    if (conn != NULL)
        xmpp_disconnect(conn);

    xmpp_handler_delete(conn, _message_handler);
    xmpp_handler_delete(conn, iq_ibb_open_handler);

    xmpp_conn_release(conn);

    fprintf(stderr, "Conn release!");

    /* final shutdown of the library */

    if (ctx != NULL)
        xmpp_ctx_free(ctx);
    xmpp_shutdown();
}

void XMPP_Close_Noshutdown(xmpp_conn_t *conn, xmpp_ctx_t *ctx)
{

    if (conn != NULL)
        xmpp_disconnect(conn);

    xmpp_handler_delete(conn, _message_handler);
    xmpp_handler_delete(conn, iq_ibb_open_handler);

    xmpp_conn_release(conn);

    fprintf(stderr, "Conn release!");

    /* final shutdown of the library */

    if (ctx != NULL)
        xmpp_ctx_free(ctx);

}

