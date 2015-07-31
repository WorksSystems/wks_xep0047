#include "strophe.h"

#define DMC_INTERVAL 10
#define MAX_URI_LEN 256
#define MAX_PROP_LEN 256
#define MAX_PAYLOAD_LEN 256
#define MAX_XMPPUN_LEN 128
#define MAX_XMPPPW_LEN 128
#define MAX_XMPPSV_LEN 128
#define MAX_ID_LEN 128
#define MAX_JID_LEN    255
#define MAX_SID_LEN 128
#define MAX_PIN_LEN 64
#define HASH_TABLE_SIZE 128
#define MAX_OBSERV_DEVICES 128

#define TAG PCF("raclient")
#define CTX_VAL 0x99

typedef struct _xmpp_cbctx_t
{

    xmpp_conn_t* conn;
    xmpp_ctx_t* ctx;
    xmpp_stanza_t *stanza;
    char szFrom[128];
    char szSid[128];
    char szIPAddr[16];
    char szPort[8];

} xmpp_cbctx_t;

char *get_target();
