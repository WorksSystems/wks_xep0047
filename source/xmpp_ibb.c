#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "strophe.h"
#include "xmpp_ibb.h"
#include "xmpp_utils.h"
#include "xmpp_common.h"

extern time_t glast_ping_time;
xmpp_ibb_session_t *gXMPP_IBB_handle_head = NULL, *gXMPP_IBB_handle_tail = NULL;
ilist_t *g_list = NULL;

typedef struct _xmpp_ibb_userdata_t
{
    xmpp_ibb_open_cb open_cb;
    xmpp_ibb_close_cb close_cb;
    xmpp_ibb_data_cb recv_cb;
    ilist_t *ilist;
} xmpp_ibb_userdata_t;

//return 0 for available
int ibb_check_handle(xmpp_ibb_session_t *handle)
{
    if (ilist_foundinlist(g_list, handle))
        return 0;
    return -1;
}

static bool _find_id(void *item, void *key)
{
    xmpp_ibb_session_t *sess;
    sess = (xmpp_ibb_session_t *) item;
    if (strncmp(sess->id, (char *) key, strlen(key)) == 0)
        return true;
    return false;
}

static bool _find_sid(void *item, void *key)
{
    xmpp_ibb_session_t *sess;
    sess = (xmpp_ibb_session_t *) item;
    if (strncmp(sess->sid, (char *) key, strlen(key)) == 0)
        return true;
    return false;
}

xmpp_ibb_session_t *ibb_get_handle_from_queue(char *id, char *sid)
{
    xmpp_ibb_session_t *sess = NULL;
    if ((sess = ilist_finditem_func(g_list, _find_sid, sid)) != NULL)
        return sess;
    else if ((sess = ilist_finditem_func(g_list, _find_id, id)) != NULL)
        return sess;
    return NULL;
}

void ibb_add_handle_to_queue(xmpp_ibb_session_t *handle)
{
    ilist_add(g_list, handle);
}

void ibb_delete_handle_from_queue(xmpp_ibb_session_t *handle)
{
    ilist_remove(g_list, handle);
}

char *generate_random_id()
{
    int i = 0, random;
    static char rid[9] = "";

    for (; i < 8; i++) {
        random = rand() % (26 + 26 + 10);
        if (random < 26)
            rid[i] = 'a' + random;
        else if (random < 26 + 26)
            rid[i] = 'A' + random - 26;
        else
            rid[i] = '0' + random - 26 - 26;
    }
    return rid;
}

static int _error_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    XMPP_IBB_Ops_t *ibb_ops_p = (XMPP_IBB_Ops_t*) userdata;
    xmpp_ibb_session_t *handle = ibb_get_handle_from_queue(xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza));
    if (handle != NULL) {
        handle->state = STATE_FAILED;
        XMPP_IBB_Open_CB open_cb = ibb_ops_p->ibb_open_fp;
        (*open_cb)(handle);
        ibb_delete_handle_from_queue(handle);
    }
    return 1;
}

static int _resule_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    XMPP_IBB_Ops_t *ibb_ops_p = (XMPP_IBB_Ops_t*) userdata;
    xmpp_ibb_session_t *handle = ibb_get_handle_from_queue(xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza));
    if (handle != NULL) {
        if (handle->state == STATE_OPENING) {
            //session created ack
            handle->state = STATE_READY;
            XMPP_IBB_Open_CB open_cb = ibb_ops_p->ibb_open_fp;
            (*open_cb)(handle);
        } else if (handle->state == STATE_READY) {
            //data sent ack
            XMPP_IBB_Send_CB send_cb = ibb_ops_p->ibb_send_fp;
            handle->data_ack_count++;
            (*send_cb)(handle);
        }
    }

    return 1;
}

static int _set_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    XMPP_IBB_Ops_t* ibb_ops_p = (XMPP_IBB_Ops_t*) userdata;
    xmpp_ibb_session_t* session_p;

    if (xmpp_stanza_get_child_by_name(stanza, "open") != NULL) {
        session_p = malloc(sizeof(xmpp_ibb_session_t));
        session_p->conn = conn;
        snprintf(session_p->id, sizeof(session_p->id), "%s", xmpp_stanza_get_id(stanza));
        snprintf(session_p->sid, sizeof(session_p->sid), "%s", XMPP_IBB_Get_Sid(stanza));
        session_p->block_size = atoi(xmpp_stanza_get_attribute(xmpp_stanza_get_child_by_name(stanza, "open"), "block-size"));
        snprintf(session_p->peer, sizeof(session_p->peer), "%s", xmpp_stanza_get_attribute(stanza, "from"));
        session_p->state = STATE_READY;
        session_p->send_seq = -1;
        session_p->recv_seq = -1;
        session_p->data_ack_count = -1;
        session_p->recv_data = NULL;
        session_p->next = NULL;
        session_p->internal_next = NULL;

        ibb_add_handle_to_queue(session_p);

        XMPP_IBB_Open_CB open_cb;
        open_cb = ibb_ops_p->ibb_open_fp;
        (*open_cb)(session_p);
        XMPP_IBB_Ack_Send(session_p);

    } else if (xmpp_stanza_get_child_by_name(stanza, "data") != NULL) {
        session_p = ibb_get_handle_from_queue(xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza));
        if (session_p != NULL) {
            char *intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "data"));
            size_t decode_len;
            session_p->recv_seq = atoi(xmpp_stanza_get_attribute(xmpp_stanza_get_child_by_name(stanza, "data"), "seq"));
            xmpp_b64decode(intext, (char **) &session_p->recv_data, &decode_len);
            XMPP_IBB_Recv_CB recv_fp = ibb_ops_p->ibb_recv_fp;
            (*recv_fp)(session_p);
            xmpp_b64free(session_p->recv_data);
            session_p->recv_data = NULL;
            XMPP_IBB_Ack_Send(session_p);
        }

    } else if (xmpp_stanza_get_child_by_name(stanza, "close") != NULL) {
        session_p = ibb_get_handle_from_queue(xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza));
        if (session_p != NULL) {
            XMPP_IBB_Ack_Send(session_p);
            ibb_delete_handle_from_queue(session_p);
            session_p->state = STATE_CLOSED;
            XMPP_IBB_Close_CB close_fp = ibb_ops_p->ibb_close_fp;
            (*close_fp)(session_p);
        }
    }

    time(&glast_ping_time);

    return 1;
}

//return seq number
int XMPP_IBB_Send(xmpp_ibb_session_t *handle, char *message)
{
    if (ibb_check_handle(handle) == 0) {
        xmpp_stanza_t *iq, *data, *text;
        char *encode, seqchar[16] = "";
        xmpp_ctx_t *ctx;
        const char *jid = xmpp_conn_get_jid(handle->conn);

        ctx = xmpp_conn_get_context(handle->conn);

        iq = xmpp_stanza_new(ctx);
        data = xmpp_stanza_new(ctx);
        text = xmpp_stanza_new(ctx);

        xmpp_stanza_set_name(iq, "iq");
        xmpp_stanza_set_type(iq, "set");
        xmpp_stanza_set_id(iq, handle->id);
        xmpp_stanza_set_attribute(iq, "to", handle->peer);
        xmpp_stanza_set_attribute(iq, "from", jid);

        xmpp_stanza_set_name(data, "data");
        xmpp_stanza_set_ns(data, XMLNS_IBB);
        xmpp_stanza_set_attribute(data, "sid", handle->sid);
        snprintf(seqchar, sizeof(seqchar), "%d", ++handle->send_seq);
        xmpp_stanza_set_attribute(data, "seq", seqchar);

        xmpp_b64encode(message, strlen(message), &encode);
        xmpp_stanza_set_text_with_size(text, encode, strlen(encode));

        xmpp_stanza_add_child(data, text);
        xmpp_stanza_add_child(iq, data);
        xmpp_send(handle->conn, iq);

        //handle->send_seq++;
        xmpp_stanza_release(text);
        xmpp_stanza_release(data);
        xmpp_stanza_release(iq);
        xmpp_b64free(encode);

        return handle->send_seq;
    }
    return -1;
}

//return 0 for success -1 for failure
int XMPP_IBB_Establish(xmpp_conn_t * const conn, char *destination, xmpp_ibb_session_t *session_handle)
{
    xmpp_stanza_t *iq, *open;
    xmpp_ctx_t *ctx;
    const char *jid = xmpp_conn_get_jid(conn);
    char *sizetemp = "4096";
    char idtemp[9] = "", sidtemp[9] = "";

    snprintf(idtemp, sizeof(idtemp), "%s", generate_random_id());
    snprintf(sidtemp, sizeof(sidtemp), "%s", generate_random_id());

    ctx = xmpp_conn_get_context(conn);

    iq = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_id(iq, idtemp);
    xmpp_stanza_set_attribute(iq, "from", jid);
    xmpp_stanza_set_attribute(iq, "to", destination);

    open = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(open, "open");
    xmpp_stanza_set_ns(open, XMLNS_IBB);
    xmpp_stanza_set_attribute(open, "block-size", sizetemp);
    xmpp_stanza_set_attribute(open, "sid", sidtemp);
    xmpp_stanza_set_attribute(open, "stanza", "iq");

    xmpp_stanza_add_child(iq, open);
    xmpp_stanza_release(open);
    xmpp_send(conn, iq);
    xmpp_stanza_release(iq);

    if (session_handle != NULL) {
        session_handle->conn = conn;
        snprintf(session_handle->id, sizeof(session_handle->id), "%s", idtemp);
        snprintf(session_handle->sid, sizeof(session_handle->sid), "%s", sidtemp);
        session_handle->block_size = atoi(sizetemp);
        session_handle->conn = conn;
        snprintf(session_handle->peer, sizeof(session_handle->peer), "%s", destination);
        session_handle->state = STATE_OPENING;
        session_handle->send_seq = -1;
        session_handle->recv_seq = -1;
        session_handle->data_ack_count = -1;
        session_handle->recv_data = NULL;
        session_handle->next = NULL;
        session_handle->internal_next = NULL;

        ibb_add_handle_to_queue(session_handle);
        return 0;
    }
    return -1;
}

void XMPP_IBB_Ack_Send(xmpp_ibb_session_t *handle)
{
    xmpp_iq_ack_result(handle->conn, handle->id, handle->peer);
}

void XMPP_IBB_Close(xmpp_ibb_session_t *handle)
{
    if (ibb_check_handle(handle) == 0) {
        xmpp_stanza_t *iq, *close;
        xmpp_ctx_t *ctx;
        const char *jid = xmpp_conn_get_jid(handle->conn);

        ctx = xmpp_conn_get_context(handle->conn);
        iq = xmpp_stanza_new(ctx);
        close = xmpp_stanza_new(ctx);

        xmpp_stanza_set_name(iq, "iq");
        xmpp_stanza_set_type(iq, "set");
        xmpp_stanza_set_id(iq, handle->id);
        xmpp_stanza_set_attribute(iq, "to", handle->peer);
        xmpp_stanza_set_attribute(iq, "from", jid);

        xmpp_stanza_set_name(close, "close");
        xmpp_stanza_set_ns(close, XMLNS_IBB);
        xmpp_stanza_set_attribute(close, "sid", handle->sid);

        xmpp_stanza_add_child(iq, close);
        xmpp_send(handle->conn, iq);
        xmpp_stanza_release(close);
        xmpp_stanza_release(iq);

        handle->state = STATE_CLOSED;
        ibb_delete_handle_from_queue(handle);
    }
}

char* XMPP_IBB_Get_Sid(xmpp_stanza_t * const stanza)
{
    xmpp_stanza_t *child;
    if ((child = xmpp_stanza_get_child_by_name(stanza, "open")) != NULL)
        return xmpp_stanza_get_attribute(child, "sid");
    else if ((child = xmpp_stanza_get_child_by_name(stanza, "data")) != NULL)
        return xmpp_stanza_get_attribute(child, "sid");
    else
        return NULL;

}

/* Initialize IBB handle. */
void XMPP_IBB_Init(xmpp_conn_t * const conn, XMPP_IBB_Ops_t* ibb_ops)
{
    srand(time(NULL)); //for generate random string
    xmpp_handler_add(conn, _set_handler, XMLNS_IBB, "iq", "set", ibb_ops);
    xmpp_handler_add(conn, _resule_handler, NULL, "iq", "result", ibb_ops);
    xmpp_handler_add(conn, _error_handler, NULL, "iq", "error", ibb_ops);
    if (g_list != NULL) {
        ilist_destroy(g_list);
    }
    g_list = ilist_new();
}

void XMPP_IBB_Release(xmpp_conn_t * const conn)
{
    ilist_destroy(g_list);
    g_list = NULL;
    xmpp_handler_delete(conn, _set_handler);
    xmpp_handler_delete(conn, _resule_handler);
    xmpp_handler_delete(conn, _error_handler);
}

#define IBB_DEFAULT_BLOCK_SIZE 4096
static xmpp_ibb_session_t * _ibb_session_init(xmpp_conn_t * const conn, char * const peer, char * const sid)
{
    xmpp_ibb_session_t *sess = NULL;
    if (conn == NULL || peer == NULL) {
        return NULL;
    }
    sess = malloc(sizeof(struct _xmpp_ibb_session_t));
    if (sess == NULL) {
        return NULL;
    }
    sess->conn = conn;
    strncpy(sess->id, generate_random_id(), sizeof(sess->id));
    if (sid != NULL && strlen(sid) > 0) {
        strncpy(sess->sid, sid, sizeof(sess->sid));
    } else {
        strncpy(sess->sid, generate_random_id(), sizeof(sess->sid));
    }
    sess->block_size = IBB_DEFAULT_BLOCK_SIZE;
    sess->conn = conn;
    strncpy(sess->peer, peer, sizeof(sess->peer));
    sess->state = STATE_OPENING;
    sess->send_seq = -1;
    sess->recv_seq = -1;
    sess->data_ack_count = -1;
    sess->recv_data = NULL;
    sess->next = NULL;
    sess->internal_next = NULL;
    return sess;
}

static int _ibb_set_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmpp_stanza_t *child;
    xmpp_ibb_userdata_t * udata = (xmpp_ibb_userdata_t *) userdata;
    xmpp_ibb_session_t * sess;
    char *from, *id;

    id = xmpp_stanza_get_id(stanza);
    from = xmpp_stanza_get_attribute(stanza, "from");
    if ((child = xmpp_stanza_get_child_by_name(stanza, "open")) != NULL) {
        char *sid = xmpp_stanza_get_attribute(child, "sid");
        sess = _ibb_session_init(conn, from, sid);
        strncpy(sess->id, id, sizeof(sess->id));
        strncpy(sess->peer, from, sizeof(sess->peer));
        sess->state = STATE_READY;
        udata->open_cb(sess);
        ilist_add(g_list, sess);
        xmpp_iq_ack_result(sess->conn, sess->id, sess->peer);
    } else if ((child = xmpp_stanza_get_child_by_name(stanza, "data")) != NULL) {
        char *sid = xmpp_stanza_get_attribute(child, "sid");
        sess = ilist_finditem_func(g_list, _find_sid, sid);
        if (sess != NULL) {
            xmppdata_t xdata;
            char *intext = xmpp_stanza_get_text(child);
            sess->recv_seq = atoi(xmpp_stanza_get_attribute(child, "seq"));
            xmpp_b64decode(intext, (char **) &xdata.data, (size_t *) &xdata.size);
            udata->recv_cb(sess, &xdata);
            xmpp_b64free(sess->recv_data);
            sess->recv_data = NULL;
            xmpp_iq_ack_result(sess->conn, sess->id, sess->peer);
        }
    } else if ((child = xmpp_stanza_get_child_by_name(stanza, "close")) != NULL) {
        char *sid = xmpp_stanza_get_attribute(child, "sid");
        sess = ilist_finditem_func(g_list, _find_sid, sid);
        if (sess != NULL) {
            ilist_remove(g_list, sess);
            xmpp_iq_ack_result(sess->conn, sess->id, sess->peer);
            sess->state = STATE_CLOSED;
            udata->close_cb(sess);
        }
    }

    time(&glast_ping_time);

    return 1;
}

static int _ibb_result_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    xmpp_ibb_session_t * sess;
    char *id = xmpp_stanza_get_id(stanza);

    sess = ilist_finditem_func(g_list, _find_id, id);
    if (sess != NULL) {
        if (sess->state == STATE_OPENING) {
            //session created ack
            sess->state = STATE_READY;
        } else if (sess->state == STATE_SENDING) {
            sess->state = STATE_READY;
            //data sent ack
        }
    }

    time(&glast_ping_time);
    return 1;
}

void xmpp_ibb_register(xmpp_conn_t * const conn, xmpp_ibb_open_cb open_cb, xmpp_ibb_close_cb close_cb, xmpp_ibb_data_cb recv_cb)
{
    static xmpp_ibb_userdata_t s_ibb_udata;
    s_ibb_udata.open_cb = open_cb;
    s_ibb_udata.close_cb = close_cb;
    s_ibb_udata.recv_cb = recv_cb;
    if (g_list != NULL) {
        ilist_destroy(g_list);
    }
    g_list = ilist_new();
    xmpp_handler_add(conn, _ibb_set_handler, XMLNS_IBB, "iq", "set", &s_ibb_udata);
    xmpp_handler_add(conn, _ibb_result_handler, NULL, "iq", "result", NULL);
//    xmpp_handler_add(conn, _error_handler, NULL, "iq", "error", ibb_ops);
}

void xmpp_ibb_unregister(xmpp_conn_t * const conn)
{
    xmpp_handler_delete(conn, _ibb_set_handler);
    xmpp_handler_delete(conn, _ibb_result_handler);
    ilist_destroy(g_list);
}

int xmpp_ibb_send_data(xmpp_ibb_session_t *sess, xmppdata_t *xdata)
{
    xmpp_stanza_t *iq, *data, *text;
    char *encode, seqchar[16] = "";
    xmpp_ctx_t *ctx;
    const char *jid = xmpp_conn_get_jid(sess->conn);

    if (sess->state != STATE_READY) {
        fprintf(stderr, "xmpp_ibb_send_data() failed. state(%d) not ready.", sess->state);
        return -1;
    }

    if (!ilist_foundinlist(g_list, sess)) {
        fprintf(stderr, "session not in handle, may be closed.");
        return -1;
    }

    ctx = xmpp_conn_get_context(sess->conn);

    iq = xmpp_stanza_new(ctx);
    data = xmpp_stanza_new(ctx);
    text = xmpp_stanza_new(ctx);

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_id(iq, sess->id);
    xmpp_stanza_set_attribute(iq, "to", sess->peer);
    xmpp_stanza_set_attribute(iq, "from", jid);

    xmpp_stanza_set_name(data, "data");
    xmpp_stanza_set_ns(data, XMLNS_IBB);
    xmpp_stanza_set_attribute(data, "sid", sess->sid);
    snprintf(seqchar, sizeof(seqchar), "%d", ++(sess->send_seq));
    xmpp_stanza_set_attribute(data, "seq", seqchar);

    xmpp_b64encode(xdata->data, xdata->size, &encode);
    xmpp_stanza_set_text_with_size(text, encode, strlen(encode));

    xmpp_stanza_add_child(data, text);
    xmpp_stanza_add_child(iq, data);
    sess->state = STATE_SENDING;
    xmpp_send(sess->conn, iq);

    xmpp_stanza_release(text);
    xmpp_stanza_release(data);
    xmpp_stanza_release(iq);
    xmpp_b64free(encode);

    return sess->send_seq;
}

xmpp_conn_t * xmpp_ibb_get_conn(xmpp_ibb_session_t *sess)
{
    if (sess == NULL)
        return NULL;

    return sess->conn;
}

char * xmpp_ibb_get_sid(xmpp_ibb_session_t *sess)
{
    if (sess == NULL)
        return NULL;

    return sess->sid;
}

char * xmpp_ibb_get_peer(xmpp_ibb_session_t *sess)
{
    if (sess == NULL)
        return NULL;

    return sess->peer;
}

xmpp_ibb_session_t *xmpp_ibb_establish(xmpp_conn_t * const conn, char * const peer, char * const sid)
{
    xmpp_ibb_session_t *sess;
    xmpp_stanza_t *iq, *open;
    xmpp_ctx_t *ctx;
    const char *jid = xmpp_conn_get_jid(conn);
    char sizetemp[6] = "";
    int size;

    sess = _ibb_session_init(conn, peer, sid);
    if (sess == NULL) {
        return NULL;
    }
    size = snprintf(sizetemp, sizeof(sizetemp), "%d", sess->block_size);
    if (size < sizeof(sizetemp)) {
        sizetemp[size] = '\0';
    }

    ctx = xmpp_conn_get_context(conn);
    iq = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_id(iq, sess->id);
    xmpp_stanza_set_attribute(iq, "from", jid);
    xmpp_stanza_set_attribute(iq, "to", sess->peer);

    open = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(open, "open");
    xmpp_stanza_set_ns(open, XMLNS_IBB);
    xmpp_stanza_set_attribute(open, "block-size", sizetemp);
    xmpp_stanza_set_attribute(open, "sid", sess->sid);
    xmpp_stanza_set_attribute(open, "stanza", "iq");

    xmpp_stanza_add_child(iq, open);
    xmpp_send(conn, iq);
    xmpp_stanza_release(open);
    xmpp_stanza_release(iq);

    ilist_add(g_list, sess);

    return sess;
}

void xmpp_ibb_disconnect(xmpp_ibb_session_t *sess)
{
    xmpp_stanza_t *iq, *close;
    xmpp_ctx_t *ctx;
    const char *jid = xmpp_conn_get_jid(sess->conn);

    ctx = xmpp_conn_get_context(sess->conn);
    iq = xmpp_stanza_new(ctx);
    close = xmpp_stanza_new(ctx);

    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_id(iq, sess->id);
    xmpp_stanza_set_attribute(iq, "to", sess->peer);
    xmpp_stanza_set_attribute(iq, "from", jid);

    xmpp_stanza_set_name(close, "close");
    xmpp_stanza_set_ns(close, XMLNS_IBB);
    xmpp_stanza_set_attribute(close, "sid", sess->sid);

    xmpp_stanza_add_child(iq, close);
    xmpp_send(sess->conn, iq);
    xmpp_stanza_release(close);
    xmpp_stanza_release(iq);

    sess->state = STATE_CLOSED;

    ilist_remove(g_list, sess);
}

void xmpp_ibb_release(xmpp_ibb_session_t *sess)
{
    free(sess);
}
