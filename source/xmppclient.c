#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <strophe.h>

#include "xmppclient.h"
#include "xmpp_ibb.h"

//hash_t* gHash_Table;
time_t glast_ping_time;

int ping_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    printf("ping_handler\n");
    XMPP_Ping(conn, xmpp_stanza_get_attribute(stanza, "from"));
    time(&glast_ping_time);
    
    return 1;
}

int presence_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmpp_stanza_t *pres;
    xmpp_ctx_t *ctx = (xmpp_ctx_t*) userdata;

    printf("presence handler\n");
    time(&glast_ping_time);

    if (strcmp(xmpp_stanza_get_attribute(stanza, "from"), xmpp_conn_get_jid(conn)) == 0) {
        printf("Get Presence of myself, return\n");
        return 1;
    }

    pres = xmpp_stanza_new(ctx);

    xmpp_stanza_set_name(pres, "presence");

    xmpp_stanza_set_attribute(pres, "to", xmpp_stanza_get_attribute(stanza, "from"));

    xmpp_send(conn, pres);
    xmpp_stanza_release(pres);
    return 1;

}

int message_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t*) userdata;
    printf("message_handler\n");
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

        XMPP_Presence(conn);

        xmpp_handler_add(conn, ping_handler, XMLNS_PING, "iq", "get", ctx);
        xmpp_handler_add(conn, message_handler, NULL, "message", NULL, ctx);

    } else {
        fprintf(stderr, "DEBUG: disconnected\n");
    }

}

void XMPP_Echo_Test(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{

#if 0
    char to[128], from[128];
    char time_str[128];
    xmpp_stanza_t *stanza_2;

    stanza_2 = xmpp_stanza_copy(stanza);
    strcpy(from, xmpp_stanza_get_attribute(stanza, "from"));
    strcpy(to, xmpp_stanza_get_attribute(stanza, "to"));

    printf("to =%s, from=%s\n", to, from);
    time_t tt;
    xmpp_stanza_set_attribute(stanza_2, "to", from);
    xmpp_stanza_set_attribute(stanza_2, "from", to);
    tt = time(NULL);

    printf("time=[%s]\n", ctime(&tt));
    xmpp_stanza_set_id(stanza_2,ctime(&tt));
    xmpp_send(conn, stanza_2);
    xmpp_stanza_release(stanza_2);
#endif

}

void XMPP_Ping(xmpp_conn_t* conn, char* const xmpp_server)
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
void XMPP_Presence(xmpp_conn_t* conn)
{

    xmpp_stanza_t* pres;
    xmpp_ctx_t *ctx;

    ctx = xmpp_ctx_new(NULL, NULL);

    /* Send initial <presence/> so that we appear online to contacts */
    pres = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(pres, "presence");

    xmpp_send(conn, pres);
    xmpp_stanza_release(pres);

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

    xmpp_handler_delete(conn, message_handler);
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

    xmpp_handler_delete(conn, message_handler);
    xmpp_handler_delete(conn, iq_ibb_open_handler);

    xmpp_conn_release(conn);

    fprintf(stderr, "Conn release!");

    /* final shutdown of the library */

    if (ctx != NULL)
        xmpp_ctx_free(ctx);

}

/*
hash_t* Hash_Init(xmpp_ctx_t * const ctx, const int size, hash_free_func free)
{
    gHash_Table = hash_new(ctx, size, free);
    return gHash_Table;
}
hash_t* Get_Hash_Handle()
{
    return gHash_Table;
}
*/

