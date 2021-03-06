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
bool g_looping = true;

static int conn_handler(xmpp_t *xmpp, xmppconn_info_t *conninfo, void *udata)
{
    if (conninfo->connevent != 0) {
        fprintf(stderr, "  status(%d) error(%d) errorType(%d) errorText '%s'\n",
                conninfo->connevent, conninfo->error, conninfo->errortype,
                conninfo->errortext);
        g_looping = false;
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
//    xmpp_ibb_release(sess);
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
//    xmpp_ibb_release(sess);
    g_session = NULL;
    g_tojid[0] = '\0';
    return 0;
}

void print_usage()
{
    printf("Usage: command [-s host -p port -j jid -w password]\n");
    printf("      -s host, xmpp server hostname or ip address\n");
    printf("      -p port, xmpp service port\n");
    printf("      -j jid, login bare JID\n");
    printf("      -w password, login passowrd\n");
    printf("      -v , show stophe debug messages\n");
    printf("      -f , force TLS\n");
}

static bool doCmd(xmpp_t *xmpp, char cmd)
{
    int looping = true;
    xmpp_conn_t *conn = xmpphelper_get_conn(xmpp);
    switch (cmd)
    {
        case 'r':
//            xmpp_ibb_release(g_session);
            break;
        case 'q':
            xmpphelper_stop(xmpp);
            looping = false;
            break;
        case 'e':
            printf("input target jid to establish session: ");
            fgets(g_tojid, sizeof(g_tojid), stdin);
            fprintf(stderr, "tojid '%s' size(%ld)", g_tojid, strlen(g_tojid));
            g_tojid[strlen(g_tojid) - 1] = '\0';
            g_session = xmpp_ibb_open(conn, g_tojid, NULL);
            break;
        case 'c':
            if (g_session == NULL || strlen(g_tojid) == 0) {
                printf("session is not setup. session<%p> target'%s'.", g_session, g_tojid);
                return looping;
            }
            xmpp_ibb_close(g_session);
            g_session = NULL;
            g_tojid[0] = '\0';
            break;
        case 's':
        {
            xmppdata_t xdata;
            char msg[4096] = "";
            if (g_session == NULL || strlen(g_tojid) == 0) {
                printf("session is not setup. session<%p> target'%s'.", g_session, g_tojid);
                return looping;
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
    return looping;
}

int main(int argc, char *argv[])
{
    bool looping = true, verbose = false;
    int opt;
    int force_tls = 0;
    char msg[1024] = "";
    char *host = "localhost", *jid = "user1@localhost", *pass = "1234";
    int port = 5222;
    xmpp_log_t *log = NULL;
    xmpp_t *xmpp;

    while ((opt = getopt(argc, argv, "s:p:w:j:t:hfv")) != -1) {
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
            case 'f':
                force_tls = 1;
                break;
            case 'j':
                jid = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
            default:
                print_usage();
                return -1;
        }
    }
    if (verbose)
        log = xmpp_get_default_logger(XMPP_LEVEL_DEBUG);
    xmpp = xmpphelper_new(conn_handler, NULL, log, NULL);
    if (force_tls == 1) {
        xmpphelper_force_tls(xmpp);
    }
    xmpphelper_connect(xmpp, host, port, jid, pass);
    xmpp_ibb_reg_funcs_t regfuncs;
    regfuncs.open_cb = open_cb;
    regfuncs.close_cb = close_cb;
    regfuncs.recv_cb = recv_cb;
    regfuncs.error_cb = error_cb;
    xmpp_ibb_register(xmpphelper_get_conn(xmpp), &regfuncs);
    xmpphelper_run(xmpp);

    while (looping && g_looping) {
        looping = doCmd(xmpp, msg[0]);
        fgets(msg, sizeof(msg), stdin);
    }
    xmpphelper_join(xmpp);

    xmpp_ibb_unregister(xmpphelper_get_conn(xmpp));

    xmpphelper_release(xmpp);
    return 0;
}

