#ifndef __XMPP_CLIENT_H__
#define __XMPP_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "strophe.h"

//#define XMLNS_IBB "http://jabber.org/protocol/ibb"

#define MAX_COAP_URI_LEN 100
#define MAX_METHOD_LEN 10
#define MAX_JSON_PAYLOAD_LEN 200
#define MAX_HEAD_LEN	128
#define MAX_LEN 50

extern time_t glast_ping_time;

typedef int (*XMPP_XEP_Handler)(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

typedef struct _XMPP_XEP_Ops_t
{
    XMPP_XEP_Handler xmpp_xep_handler_ops;
} XMPP_XEP_Ops_t;

xmpp_conn_t* XMPP_Init(char* jid, char* pass, char* host, xmpp_ctx_t **pctx);

void XMPP_Echo_Test(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

void XMPP_XEP0047_Init(xmpp_conn_t* conn, xmpp_ctx_t* ctx);

int XMPP_xep0047_data_process(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

void XMPP_Close(xmpp_conn_t *conn, xmpp_ctx_t *ctx);

void XMPP_Close_Noshutdown(xmpp_conn_t *conn, xmpp_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif//__XMPP_CLIENT_H__
