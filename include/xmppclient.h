#include "strophe.h"
#include "hash.h"

#define XMLNS_PING "urn:xmpp:ping"
//#define XMLNS_IBB "http://jabber.org/protocol/ibb"

#define MAX_COAP_URI_LEN 100
#define MAX_METHOD_LEN 10
#define MAX_JSON_PAYLOAD_LEN 200
#define MAX_HEAD_LEN	128
#define MAX_LEN 50

char *base64_encode(xmpp_ctx_t *ctx, const unsigned char * const buffer, const unsigned len);

unsigned char *base64_decode(xmpp_ctx_t *ctx, const char * const buffer, const unsigned len);

typedef int (*XMPP_XEP_Handler)(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

typedef struct _XMPP_XEP_Ops_t
{
    XMPP_XEP_Handler xmpp_xep_handler_ops;
} XMPP_XEP_Ops_t;

xmpp_conn_t* XMPP_Init(char* jid, char* pass, char* host, xmpp_ctx_t **pctx);

void XMPP_Presence(xmpp_conn_t* conn);

//void XMPP_IBB_SendPayload(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata, OCClientResponse *);

void XMPP_Echo_Test(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

//void XMPP_IBB_Ack_Send( xmpp_ibb_session_t *handle );

void XMPP_XEP0047_Init(xmpp_conn_t* conn, xmpp_ctx_t* ctx);

int XMPP_xep0047_data_process(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata);

void XMPP_Ping(xmpp_conn_t* conn, char* const xmpp_server);

void XMPP_Close(xmpp_conn_t *conn, xmpp_ctx_t *ctx);

void XMPP_Close_Noshutdown(xmpp_conn_t *conn, xmpp_ctx_t *ctx);

hash_t* Hash_Init(xmpp_ctx_t * const ctx, const int size, hash_free_func free);

hash_t* Get_Hash_Handle();

