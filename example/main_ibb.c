#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <xmpp_helper.h>

#include "xmpp_chat.h"
#include "xmpp_utils.h"
#include "xmpp_ibb.h"

char g_tojid[256] = "";
xmpp_ibb_session_t *g_session;
#if 0
static int chat_recv_handler(xmpp_conn_t *xmpp, xmppdata_t *xdata, void *udata)
{
    char *decdata;
    size_t decsize;
    fprintf(stderr, "\n  chat_recv_handler(conn<%p>, from'%s', msg'%s'\n\n", xmpp, xdata->from, (char *) xdata->data);
    xmpp_b64decode((char *) xdata->data, &decdata, &decsize);
    fprintf(stderr, "\n    try decode(decdata'%s', decsize(%ld))\n", decdata, decsize);
    strcpy(g_rejid, xdata->from);
    return 0;
}
#endif
static int conn_handler(xmpp_t *xmpp, xmppconn_info_t *conninfo, void *udata)
{
    if (conninfo->connevent != 0) {
        fprintf(stderr, "  status(%d) error(%d) errorType(%d) errorText '%s'\n",
                conninfo->connevent, conninfo->error, conninfo->errortype,
                conninfo->errortext);
        return -1;
    }
    printf( "\n\n       login full JID: %s\n\n\n", xmpphelper_get_bound_jid(xmpp));
    return 0;
}

static int open_cb(xmpp_ibb_session_t *sess, char *type)
{
    printf("\n  %s() type '%s'\n", __FUNCTION__, type);
    strncpy(g_tojid, xmpp_ibb_get_remote_jid(sess), sizeof(g_tojid));
    g_session = sess;
    if (strncmp("result", type, 6) == 0)
        printf("%s() result\n", __FUNCTION__);
    return 0;
}

static int close_cb(xmpp_ibb_session_t *sess, char *type)
{
    printf("\n  %s() type '%s'\n", __FUNCTION__, type);
    xmpp_ibb_release(sess);
    g_session = NULL;
    g_tojid[0] = '\0';
    if (strncmp("result", type, 6) == 0)
        printf("%s() result\n", __FUNCTION__);
    return 0;
}

static int recv_cb(xmpp_ibb_session_t *sess, xmppdata_t *xdata)
{
    printf("\n  %s()\n", __FUNCTION__);
    if (xdata != NULL)
        printf("    data'%s' size(%d)\n", (char *) xdata->data, xdata->size);
    else
        printf("  %s() result\n", __FUNCTION__);
    return 0;
}

static int error_cb(xmpp_ibb_session_t *sess, xmpperror_t *xerr)
{
    printf("\n  %s() code(%d) type '%s' msg '%s'\n", __FUNCTION__, xerr->code, xerr->type, xerr->mesg);
    xmpp_ibb_release(sess);
    g_session = NULL;
    g_tojid[0] = '\0';
    return 0;
}

void print_usage()
{
    printf("Usage: command [-s host -p port -j jid -w password]\n");
}

int main(int argc, char *argv[])
{
    bool looping = true;
    int opt;
    xmpp_t *xmpp;
    xmpp_conn_t *conn;
    char msg[1024] = "";
    char *host = "localhost", *jid = "user1@localhost", *pass = "1234";
    int port = 5222;

    while ((opt = getopt(argc, argv, "s:p:w:j:t:h")) != -1) {
        switch (opt)
        {
            case 's':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'w':
                pass = optarg;
                break;
            case 'j':
                jid = optarg;
                break;
            case 'h':
            default:
                print_usage();
                return -1;
        }
    }

    xmpp = xmpphelper_new(conn_handler, NULL);
    xmpphelper_connect(xmpp, host, port, jid, pass);
    conn = xmpphelper_get_conn(xmpp);
    xmpp_ibb_reg_funcs_t regfuncs;
    regfuncs.open_cb = open_cb;
    regfuncs.close_cb = close_cb;
    regfuncs.recv_cb = recv_cb;
    regfuncs.error_cb = error_cb;
    xmpp_ibb_register(conn, &regfuncs);
    xmpphelper_run(xmpp);

    while (looping) {
        fgets(msg, sizeof(msg), stdin);
        switch (msg[0])
        {
            case 'q':
                xmpphelper_stop(xmpp);
                looping = false;
                break;
            case 'e':
                printf("input target jid to establish session: ");
                fgets(g_tojid, sizeof(g_tojid), stdin);
                fprintf(stderr, "tojid '%s' size(%ld)", g_tojid, strlen(g_tojid));
                g_tojid[strlen(g_tojid) - 1] = '\0';
                g_session = xmpp_ibb_establish(conn, g_tojid, NULL);
                break;
            case 'c':
                if (g_session == NULL || strlen(g_tojid) == 0) {
                    printf("session is not setup. session<%p> target'%s'.", g_session, g_tojid);
                    continue;
                }
                xmpp_ibb_disconnect(g_session);
                g_session = NULL;
                g_tojid[0] = '\0';
                break;
            case 's':
            {
                xmppdata_t xdata;
                if (g_session == NULL || strlen(g_tojid) == 0) {
                    printf("session is not setup. session<%p> target'%s'.", g_session, g_tojid);
                    continue;
                }
                printf("input messages to target jid '%s': ", g_tojid);
                fgets(msg, sizeof(msg), stdin);
                xdata.data = msg;
                xdata.size = strlen(msg);
                xmpp_ibb_send_data(g_session, &xdata);
                break;
            }
            default:
                printf("\n 'q' to quit, 'e' establish ibb session, 's' send message to '%s', 'c' close ibb session: ", g_tojid);
                break;
        }
    }
    xmpphelper_join(xmpp);

    xmpp_ibb_unregister(xmpphelper_get_conn(xmpp));

    xmpphelper_release(xmpp);
    return 0;
}

