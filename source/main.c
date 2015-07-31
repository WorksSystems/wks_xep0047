#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <strophe.h>
#include "common.h"

#include "xmppclient.h"
#include "xmpp_ibb.h"

#define CTX_VAL 0x99
char xmpp_un[MAX_XMPPUN_LEN] = "";
char xmpp_pw[MAX_XMPPPW_LEN] = "";
char xmpp_server[MAX_XMPPSV_LEN] = "cloud01.workssys.com";

int bQuit = 1;

time_t glast_ping_time;
int gQuitFlag = 0;

char target[128]="";
char *get_target()
{
    return target;
}


/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{

    if (signum == SIGINT) {
        gQuitFlag = 1;
    }
}

int xmpp_ibb_open_cb( xmpp_ibb_session_t *session_handle )
{
    printf("====Open CB====\n");
    printf("id: %s\n", session_handle->id );
    printf("sid: %s\n", session_handle->sid );
    printf("peer: %s\n", session_handle->peer );
    printf("state: %d\n", session_handle->state );
    printf("===============\n");


    if( session_handle->state == STATE_READY && strcmp( session_handle->conn->jid, "andreac\\40workssys.com@cloud01.workssys.com/resource1" ) == 0 )
        printf("send to session_handle->peer returns: %d\n", XMPP_IBB_Send( session_handle, "If the responder wishes to proceed with the IBB session, it returns an IQ-result to the initiator." ) );

    return 0;
}

void xmpp_ibb_send_cb( xmpp_ibb_session_t *session_handle )
{
    printf("====Send CB====\n");
    printf("id: %s\n", session_handle->id );
    printf("sid: %s\n", session_handle->sid );
    printf("peer: %s\n", session_handle->peer );
    printf("recv_seq: %d\n", session_handle->recv_seq );
    printf("send_seq: %d\n", session_handle->send_seq );
    printf("data_ack_count: %d\n", session_handle->data_ack_count );
    printf("state: %d\n", session_handle->state );
    printf("===============\n");

    if( session_handle->data_ack_count == 0 )
        XMPP_IBB_Close( session_handle );
}

void xmpp_ibb_recv_cb( xmpp_ibb_session_t *session_handle )
{
    printf("====Recv CB====\n");
    printf("id: %s\n", session_handle->id );
    printf("sid: %s\n", session_handle->sid );
    printf("peer: %s\n", session_handle->peer );
    printf("state: %d\n", session_handle->state );
    printf("seq: %d\n", session_handle->recv_seq );
    printf("msg: %s\n", session_handle->recv_data );
    printf("===============\n");

    /*
    if( strcmp( session_handle->peer, "andreac\\40workssys.com@cloud01.workssys.com/resource1" ) == 0 )
    {
        printf("reply to resource1 returns: %d\n", XMPP_IBB_Send( session_handle, "WTF WTF WTF WTF WTF return retrun return....." ));
    }
    */

}
void xmpp_ibb_close_cb( xmpp_ibb_session_t *session_handle )
{
    printf("====Close CB====\n");
    printf("id: %s\n", session_handle->id );
    printf("sid: %s\n", session_handle->sid );
    printf("peer: %s\n", session_handle->peer );
    printf("state: %d\n", session_handle->state );
    printf("===============\n");
}
void init_argument(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    xmpp_ctx_t *ctx = NULL;
    xmpp_conn_t* conn = NULL;
    XMPP_IBB_Ops_t xmpp_ibb_ops;

    xmpp_ibb_ops.ibb_open_fp = xmpp_ibb_open_cb;
    xmpp_ibb_ops.ibb_recv_fp = xmpp_ibb_recv_cb;
    xmpp_ibb_ops.ibb_send_fp = xmpp_ibb_send_cb;
    xmpp_ibb_ops.ibb_close_fp = xmpp_ibb_close_cb;

    init_argument(argc, argv);

    while ((!gQuitFlag) || (bQuit == 0)) {

        if (conn == NULL) {
            conn = XMPP_Init(xmpp_un, xmpp_pw, xmpp_server, &ctx);
            XMPP_IBB_Init(conn, &xmpp_ibb_ops);
            Hash_Init(ctx, HASH_TABLE_SIZE, xmpp_free);
            time(&glast_ping_time);

        }
        else
            xmpp_run_once( ctx, 500 );
    }

    printf("Quit\n");

    xmpp_stop(ctx);
    sleep(1);
    XMPP_Close(conn, ctx);

    return 0;
}

void PrintUsage()
{

    printf("Usage: wks_raclient-arm <Interface> [-p PIN -q 0|1 ]\n\
        -q : Enable Quit by Ctrl+C or SIGINT\n\
        -h : This Help\n");

}
void init_argument(int argc, char* argv[])
{
    char c;

    while ((c = getopt(argc, argv, "u:t:q:h")) != -1) {
        switch (c)
        {
            case 'u':
                if( strcmp( optarg, "1" ) == 0 )
                {
                    snprintf( xmpp_un, sizeof(xmpp_un), "andreac\\40workssys.com@cloud01.workssys.com/resource1" );
                    snprintf( xmpp_pw, sizeof(xmpp_pw), "947c2c96o13cnjmujfh4l5i3is" );
                }
                else if( strcmp( optarg, "2" ) == 0 )
                {
                    snprintf( xmpp_un, sizeof(xmpp_un), "andreac\\40workssys.com@cloud01.workssys.com/resource2" );
                    snprintf( xmpp_pw, sizeof(xmpp_pw), "947c2c96o13cnjmujfh4l5i3is" );
                }
                else if( strcmp( optarg, "3" ) == 0 )
                {
                    snprintf( xmpp_un, sizeof(xmpp_un), "andreac\\40workssys.com@cloud01.workssys.com/resource3" );
                    snprintf( xmpp_pw, sizeof(xmpp_pw), "947c2c96o13cnjmujfh4l5i3is" );
                }

            case 't':
                if( strlen(optarg) > 4 )
                    snprintf( target, sizeof(target), "%s", optarg );

            case 'q':

                bQuit = atoi(optarg);
                if (bQuit == 0)
                    printf("No Ctrl+C used\n");

                break;

            case 'h':
                PrintUsage();
                exit(-1);
                break;
            default:

                return;
        }

    }
}

