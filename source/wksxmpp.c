#include <stdlib.h>
#include <pthread.h>

#include "wksxmpp.h"
#include "wksxmpp_common.h"

static void _send_presense(xmpp_conn_t *conn, char *to)
{
    xmpp_ctx_t      *ctx;
    xmpp_stanza_t   *szpres;

    ctx = xmpp_conn_get_context(conn);
    szpres = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(szpres, "presence");
    xmpp_send(conn, szpres);
    xmpp_stanza_release(szpres);
}

static void _conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status, 
                  const int error, xmpp_stream_error_t * const stream_error, 
                  void * const userdata) 
{
    wksxmpp_t      *xmpp;

    xmpp = (wksxmpp_t *) userdata;

    if (status == XMPP_CONN_CONNECT) {
        _send_presense(conn, "");
    } else {
        fprintf(stderr, "\n    unknown status(%d) \n\n", status);
    }

    if (xmpp->callback != NULL) {
        wksxmpp_conninfo_t conninfo;
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

static int _stanza_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza,
        void * const userdata)
{
    return 1;
}

static void *pth_func(void *arg)
{
    wksxmpp_t *xmpp;
    xmpp = (wksxmpp_t *) arg;
    xmpp_run(xmpp->ctx);
    return NULL;
}

void *wksxmpp_new(wksxmpp_conn_handler cb, void *userdata)
{
    wksxmpp_t *xmpp;
    xmpp = (wksxmpp_t *) malloc(sizeof(struct _wksxmpp_t));

    xmpp_initialize();

    xmpp->log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
    xmpp->ctx = xmpp_ctx_new(NULL, xmpp->log);
    xmpp->conn = xmpp_conn_new(xmpp->ctx);
    xmpp->callback = cb;
    xmpp->userdata = userdata;

    return xmpp;
}

void wksxmpp_connect(void *ins, char *host, int port, char *jid, char *pass)
{
    wksxmpp_t *xmpp = (wksxmpp_t *) ins;
    xmpp_conn_set_jid(xmpp->conn, jid);
    xmpp_conn_set_pass(xmpp->conn, pass);
    xmpp_handler_add(xmpp->conn, _stanza_handler, NULL, NULL, NULL, xmpp);
    xmpp_connect_client(xmpp->conn, NULL, 0, _conn_handler, xmpp);
}

void wksxmpp_run_thread(void *ins)
{
    wksxmpp_t *xmpp = (wksxmpp_t *) ins;
    pthread_create(&xmpp->pth, NULL, pth_func, xmpp);
}

void wksxmpp_stop_thread(void *ins)
{
    wksxmpp_t *xmpp = (wksxmpp_t *) ins;
    xmpp_disconnect(xmpp->conn);
    xmpp_stop(xmpp->ctx);
}

void wksxmpp_thread_join(void *ins)
{
    wksxmpp_t *xmpp = (wksxmpp_t *) ins;
    pthread_join(xmpp->pth, NULL);
}

int wksxmpp_release(void *ins)
{
    wksxmpp_t *xmpp = (wksxmpp_t *) ins;
    xmpp_conn_release(xmpp->conn);
    xmpp->conn = NULL;
    xmpp_ctx_free(xmpp->ctx);
    xmpp->ctx = NULL;
    xmpp_shutdown();
    free(xmpp);
    return 0;
}

xmpp_conn_t *wksxmpp_get_conn(void *ins)
{
    wksxmpp_t *xmpp = (wksxmpp_t *) ins;
    return xmpp->conn;
}

