#include <stdlib.h>
#include <pthread.h>
#include <xmpp.h>

#include "xmpp_common.h"

static int _ping_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
    char *to;
    char *id;
    printf("_ping_handler()\n");
    to = xmpp_stanza_get_attribute(stanza, "from");
    id = xmpp_stanza_get_attribute(stanza, "id");
    xmpp_ping(conn, id, to, "result");
    //xmpp_ping(conn, NULL, to, NULL);
    return 1;
}

static int _stanza_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza,
        void * const userdata)
{
    //fprintf(stderr, "    %s-%d: %s()\n", __FILE__, __LINE__, __FUNCTION__);
    return 1;
}

static void _conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status, 
                  const int error, xmpp_stream_error_t * const stream_error, 
                  void * const userdata) 
{
    xmpp_t      *xmpp;

    xmpp = (xmpp_t *) userdata;

    if (status == XMPP_CONN_CONNECT) {
        xmpp_presence(conn, "");
        xmpp_handler_add(conn, _ping_handler, XMLNS_PING, "iq", "get", xmpp);
        xmpp_handler_add(conn, _stanza_handler, NULL, NULL, NULL, xmpp);
    } else if (status == XMPP_CONN_DISCONNECT) {
        xmpp_handler_delete(conn, _ping_handler);
        xmpp_handler_delete(conn, _stanza_handler);
    } else {
        fprintf(stderr, "\n    unknown status(%d) \n\n", status);
    }

    if (xmpp->callback != NULL) {
        xmppconn_info_t conninfo;
        conninfo.connevent = (int) status;
        conninfo.error = error;
        if (conninfo.error != 0 && stream_error != NULL) {
            conninfo.errortype = (int) stream_error->type;
            conninfo.errortext = stream_error->text;
        } else {
            conninfo.errortext = NULL;
            conninfo.errortype = 0;
        }
        xmpp->callback(xmpp, &conninfo, xmpp->userdata);
    }
}

static void *pth_func(void *arg)
{
    xmpp_t *xmpp;
    xmpp = (xmpp_t *) arg;
    xmpp_run(xmpp->ctx);
    return NULL;
}

xmpp_t *xmpp_new(xmppconn_handler cb, void *userdata)
{
    xmpp_t *xmpp;
    xmpp = (xmpp_t *) malloc(sizeof(struct _xmpp_t));

    xmpp_initialize();

    xmpp->log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
    xmpp->ctx = xmpp_ctx_new(NULL, xmpp->log);
    xmpp->conn = xmpp_conn_new(xmpp->ctx);
    xmpp->callback = cb;
    xmpp->userdata = userdata;

    return xmpp;
}

void xmpp_connect(xmpp_t *xmpp, char *host, int port, char *jid, char *pass)
{
    xmpp_conn_set_jid(xmpp->conn, jid);
    xmpp_conn_set_pass(xmpp->conn, pass);
    xmpp_connect_client(xmpp->conn, NULL, 0, _conn_handler, xmpp);
}

void xmpp_run_thread(xmpp_t *xmpp)
{
    pthread_create(&xmpp->pth, NULL, pth_func, xmpp);
}

void xmpp_stop_thread(xmpp_t *xmpp)
{
    xmpp_disconnect(xmpp->conn);
    xmpp_stop(xmpp->ctx);
}

void xmpp_thread_join(xmpp_t *xmpp)
{
    pthread_join(xmpp->pth, NULL);
}

int xmpp_release(xmpp_t *xmpp)
{
    xmpp_conn_release(xmpp->conn);
    xmpp->conn = NULL;
    xmpp_ctx_free(xmpp->ctx);
    xmpp->ctx = NULL;
    xmpp_shutdown();
    free(xmpp);
    return 0;
}

xmpp_conn_t *xmpp_get_conn(xmpp_t *xmpp)
{
    return xmpp->conn;
}
