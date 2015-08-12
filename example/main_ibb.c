#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "xmpp.h"
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
static int conn_handler(void *ins, xmppconn_info_t *conninfo, void *udata)
{
    fprintf(stderr, "\n    conn_handler(ins<%p>, conninfo<%p>, udata<%p>)\n", ins, conninfo, udata);
    fprintf(stderr, "      status(%d) error(%d) errorType(%d) errorText '%s'\n", conninfo->connevent, conninfo->error, conninfo->errortype, conninfo->errortext);
    return 0;
}

static int open_cb(xmpp_ibb_session_t *sess)
{
    fprintf(stderr, "\n    %s()\n", __FUNCTION__);
    strncpy(g_tojid, xmpp_ibb_get_peer(sess), sizeof(g_tojid));
    g_session = sess;
    putchar(' ');
    return 0;
}

static int close_cb(xmpp_ibb_session_t *sess)
{
    fprintf(stderr, "\n    %s()\n", __FUNCTION__);
    xmpp_ibb_release(g_session);
    g_session = NULL;
    g_tojid[0] = '\0';
    putchar(' ');
    return 0;
}

static int recv_cb(xmpp_ibb_session_t *sess, xmppdata_t *xdata)
{
    fprintf(stderr, "\n    %s()\n", __FUNCTION__);
    fprintf(stderr, "    data'%s' size(%d)\n", (char *) xdata->data, xdata->size);
    putchar(' ');
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
    char *host = "localhost", *jid = "user1@localhost/res1", *pass = "1234";
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

    xmpp = xmpp_new(conn_handler, NULL);
    xmpp_connect(xmpp, host, port, jid, pass);
    conn = xmpp_get_conn(xmpp);
    xmpp_ibb_register(conn, open_cb, close_cb, recv_cb);
    xmpp_run_thread(xmpp);

    while (looping) {
        printf("\n 'q' to quit, 'e' establish ibb session, 's' send message to '%s', 'c' close ibb session: ", g_tojid);
        fgets(msg, sizeof(msg), stdin);
        switch (msg[0])
        {
            case 'q':
                xmpp_stop_thread(xmpp);
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
                xmpp_ibb_release(g_session);
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
                break;
        }
    }
    xmpp_thread_join(xmpp);

    xmpp_ibb_unregister(xmpp_get_conn(xmpp));

    xmpp_release(xmpp);
    return 0;
}

