#ifndef __XMPP_IBB_H__
#define __XMPP_IBB_H__

#include "strophe.h"
#include "xmpp_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_XMPPUN_LEN  128
#define MAX_XMPPPW_LEN  128
#define MAX_XMPPSV_LEN  128
#define MAX_ID_LEN      128
#define MAX_JID_LEN     255
#define MAX_SID_LEN     128

#define XMLNS_IBB "http://jabber.org/protocol/ibb"
typedef enum
{
    STATE_NONE,
    STATE_OPENING,
    STATE_READY,
    STATE_SENDING,
    STATE_CLOSED,
    STATE_FAILED
} SEND_STATE;

/* xmpp_ibb_data_t and xmpp_ibb_session_t is not used in version 1.0 now.
 It is used for multi IBB session handle
 */
typedef struct _xmpp_ibb_session_t
{
    //for session
    xmpp_conn_t* conn;
    char id[MAX_ID_LEN];
    char sid[MAX_SID_LEN];
    int block_size;
    char peer[MAX_JID_LEN];
    SEND_STATE state;

    //for data
    int send_seq;
    int recv_seq;
    int data_ack_count; //how many data ack has been received
    unsigned char *recv_data;

    struct _xmpp_ibb_session_t *next;
    struct _xmpp_ibb_session_t *internal_next;

} xmpp_ibb_session_t;

typedef int (*XMPP_IBB_Open_CB)(xmpp_ibb_session_t*);
typedef void (*XMPP_IBB_Recv_CB)(xmpp_ibb_session_t*);
typedef void (*XMPP_IBB_Send_CB)(xmpp_ibb_session_t*); //result 0 for success, others for failure
typedef void (*XMPP_IBB_Close_CB)(xmpp_ibb_session_t*);

typedef struct _XMPP_IBB_Ops_t
{

    XMPP_IBB_Open_CB ibb_open_fp;
    XMPP_IBB_Recv_CB ibb_recv_fp;
    XMPP_IBB_Send_CB ibb_send_fp;
    XMPP_IBB_Close_CB ibb_close_fp;

} XMPP_IBB_Ops_t;

int XMPP_IBB_Send(xmpp_ibb_session_t *session_handle, char *message);

int XMPP_IBB_Establish(xmpp_conn_t * const conn, char *destination, xmpp_ibb_session_t *session_handle);

void XMPP_IBB_Ack_Send(xmpp_ibb_session_t *handle);

void XMPP_IBB_Close(xmpp_ibb_session_t *handle);

void XMPP_IBB_Init(xmpp_conn_t * const conn, XMPP_IBB_Ops_t* ibb_ops);

void XMPP_IBB_Release(xmpp_conn_t * const conn);

char* XMPP_IBB_Get_Sid(xmpp_stanza_t * const stanza);
/* Get IBB handle in order to get Session ID */
xmpp_ibb_session_t* XMPP_Get_IBB_Handle(void);
/* Get IBB Session Handle by Sid in order to get blocksize, data payload */
xmpp_ibb_session_t* XMPP_Get_IBB_Session_Handle(char* szSid);

/*Add a session to the Queue */
void XMPP_IBB_Add_Session_Queue(xmpp_ibb_session_t* ibb_ssn_new);

typedef int (*xmpp_ibb_open_cb)(xmpp_ibb_session_t *sess);
typedef int (*xmpp_ibb_close_cb)(xmpp_ibb_session_t *sess);
typedef int (*xmpp_ibb_data_cb)(xmpp_ibb_session_t *sess, xmppdata_t *xdata);

void xmpp_ibb_register(xmpp_conn_t * const conn, xmpp_ibb_open_cb open_cb, xmpp_ibb_close_cb close_cb, xmpp_ibb_data_cb recv_cb);

void xmpp_ibb_unregister(xmpp_conn_t * const conn);

xmpp_ibb_session_t *xmpp_ibb_establish(xmpp_conn_t * const conn, char * const peer, char * const sid);

void xmpp_ibb_disconnect(xmpp_ibb_session_t *sess);

void xmpp_ibb_release(xmpp_ibb_session_t *sess);

int xmpp_ibb_send_data(xmpp_ibb_session_t *sess, xmppdata_t *xdata);

xmpp_conn_t * xmpp_ibb_get_conn(xmpp_ibb_session_t *sess);

char * xmpp_ibb_get_sid(xmpp_ibb_session_t *sess);

char * xmpp_ibb_get_peer(xmpp_ibb_session_t *sess);

#ifdef __cplusplus
}
#endif

#endif//__XMPP_IBB_H__
