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
    STATE_CLOSING,
    STATE_FAILED
} xmpp_ibb_state_t;

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
    xmpp_ibb_state_t state;

    //for data
    int send_seq;
    int recv_seq;
    int data_ack_count; //how many data ack has been received
    unsigned char *recv_data;
    void *userdata;

    struct _xmpp_ibb_session_t *next;
    struct _xmpp_ibb_session_t *internal_next;

} xmpp_ibb_session_t;

/**
 *
 * @param sess session of IBB
 * @param type type of stanza, set or result
 * @return
 */
typedef int (*xmpp_ibb_open_cb)(xmpp_ibb_session_t *sess, char *type);

/**
 *
 * @param sess session of IBB
 * @param type type of stanza set or result
 * @return 0 is OK, others error.
 */
typedef int (*xmpp_ibb_close_cb)(xmpp_ibb_session_t *sess, char *type);

/**
 *
 * @param sess session of IBB
 * @param xdata xdata if NULL, means stanza type is 'result',
 *          otherwise stanza type is set, and message data in xdata
 * @return 0 is OK, others error.
 */
typedef int (*xmpp_ibb_data_cb)(xmpp_ibb_session_t *sess, xmppdata_t *xdata);

/**
 *
 * @param sess session of IBB
 * @param xerror error information
 * @return 0 is OK, others error.
 */
typedef int (*xmpp_ibb_error_cb)(xmpp_ibb_session_t *sess, xmpperror_t *xerror);

typedef struct _xmpp_ibb_reg_funcs_t {
    xmpp_ibb_open_cb open_cb;
    xmpp_ibb_close_cb close_cb;
    xmpp_ibb_data_cb recv_cb;
    xmpp_ibb_error_cb error_cb;
} xmpp_ibb_reg_funcs_t;

/**
 *
 * @param conn conn of libstrophe.
 * @param reg_funcs register functions for IBB
 */
void xmpp_ibb_register(xmpp_conn_t * const conn, xmpp_ibb_reg_funcs_t *reg_funcs);

/**
 *
 * @param conn conn of libstrophe.
 */
void xmpp_ibb_unregister(xmpp_conn_t * const conn);

/**
 *
 * @param conn conn of libstrophe
 * @param jid target jid to establish
 * @param sid if set, specific session id, otherwise use random generate.
 * @return session of IBB
 */
xmpp_ibb_session_t *xmpp_ibb_establish(xmpp_conn_t * const conn, char * const jid, char * const sid);

/**
 *
 * @param sess session of IBB
 * @return 0 is OK, others error.
 */
int xmpp_ibb_disconnect(xmpp_ibb_session_t *sess);

/**
 *
 * @param sess session of IBB
 */
void xmpp_ibb_release(xmpp_ibb_session_t *sess);

/**
 *
 * @param sess session of IBB
 * @param xdata message data to send
 * @return
 */
int xmpp_ibb_send_data(xmpp_ibb_session_t *sess, xmppdata_t *xdata);

/**
 *
 * @param sess session of IBB
 * @return conn of libstrophe
 */
xmpp_conn_t * xmpp_ibb_get_conn(xmpp_ibb_session_t *sess);

/**
 *
 * @param sess session of IBB
 * @return session id
 */
char * xmpp_ibb_get_sid(xmpp_ibb_session_t *sess);

/**
 *
 * @param sess session of IBB
 * @return remote jid
 */
char * xmpp_ibb_get_remote_jid(xmpp_ibb_session_t *sess);

/**
 *
 * @param sid session id
 * @return session of IBB, if not found, return NULL
 */
xmpp_ibb_session_t *xmpp_ibb_get_session_by_sid(char *sid);

#ifdef __cplusplus
}
#endif

#endif//__XMPP_IBB_H__
