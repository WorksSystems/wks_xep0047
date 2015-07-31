#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "xmppclient.h"
#include "xmpp_ibb.h"

#include <strophe.h>
#include "common.h"

extern time_t glast_ping_time;
xmpp_ibb_session_t *gXMPP_IBB_handle_head = NULL, *gXMPP_IBB_handle_tail = NULL;

//return 0 for available
int ibb_check_handle( xmpp_ibb_session_t *handle )
{
    xmpp_ibb_session_t *temp = gXMPP_IBB_handle_head;
    while( temp != NULL )
    {
        if( temp == handle )
            return 0;
        else
            temp = temp->next;
    }
    return -1;
}

xmpp_ibb_session_t *ibb_get_handle_from_queue( char *id, char *sid )
{
    xmpp_ibb_session_t *temp = gXMPP_IBB_handle_head;
    while( temp != NULL )
    {
        if( sid != NULL )
        {
            //printf("id: %s, temp->id: %s\n", id, temp->id );
            //printf("sid: %s, temp->sid: %s\n", sid, temp->sid );
            if( strcmp( id, temp->id ) == 0 && strcmp( sid, temp->sid ) == 0 )
            {
                return temp;
            }
            else
            {
                temp = temp->next;
            }
        }
        else
        {
            //printf("id: %s, temp->id: %s\n", id, temp->id );
            if( strcmp( id, temp->id ) == 0 )
                return temp;
            else
                temp = temp->next;
        }
    }
    return NULL; 
}

void ibb_add_handle_to_queue( xmpp_ibb_session_t *handle )
{
    /*
    printf("before add, ");
    if( handle == handle->next )
        printf("temp is temp->next\n");
    else
        printf("temp is not temp->next\n");
    */

    if( gXMPP_IBB_handle_head == NULL )
    {
        gXMPP_IBB_handle_head = handle;
    }
    else
        gXMPP_IBB_handle_tail->next = handle;

    gXMPP_IBB_handle_tail = handle;
    gXMPP_IBB_handle_tail->next = NULL;

    //printf("gXMPP_IBB_handle_head: %s\n", gXMPP_IBB_handle_head->id );
    
    /*
    printf("after add, ");
    if( handle == handle->next )
        printf("temp is tmep->next\n");
    else
        printf("temp is not temp->next\n");
    */
}

void ibb_delete_handle_from_queue( xmpp_ibb_session_t *handle )
{
    xmpp_ibb_session_t *temp, *last;
    if( gXMPP_IBB_handle_head != NULL )
    {
        temp = gXMPP_IBB_handle_head;
        last = temp;
        while( temp != NULL )
        {
            if( temp == handle )
            {
                if( gXMPP_IBB_handle_head == temp )
                {
                    gXMPP_IBB_handle_head = gXMPP_IBB_handle_head->next;
                }
                else if( gXMPP_IBB_handle_tail == temp )
                {
                    gXMPP_IBB_handle_tail = last;
                }
                else
                {
                    last->next = temp->next;
                    temp->next = NULL;
                }
                return;
            }
            else
            {
                last = temp;
                temp = temp->next;
            }
        }
    }
}

char *generate_random_id()
{
    int i = 0, random;
    static char rid[9]="";

    for( ; i<8; i++ )
    {
        random = rand()%( 26+26+10 );
        if( random < 26 )
            rid[i] = 'a' + random;
        else if( random < 26+26 )
            rid[i] = 'A' + random - 26;
        else
            rid[i] = '0' + random - 26 - 26;
    }
    return rid;
}

int ibb_error_handler( xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata )
{
    XMPP_IBB_Ops_t *ibb_ops_p = (XMPP_IBB_Ops_t*) userdata;
    xmpp_ibb_session_t *handle = ibb_get_handle_from_queue( xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza) );
    if( handle != NULL )
    {
        handle->state = STATE_FAILED;
        XMPP_IBB_Open_CB open_cb = ibb_ops_p->ibb_open_fp;
        (*open_cb)(handle);
        ibb_delete_handle_from_queue( handle );
    }
    return 1;
}

int ibb_ack_handler( xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata )
{
    XMPP_IBB_Ops_t *ibb_ops_p = (XMPP_IBB_Ops_t*) userdata;
    xmpp_ibb_session_t *handle = ibb_get_handle_from_queue( xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza) );
    if( handle != NULL )
    {
        if( handle->state == STATE_WAITING )
        {
            //session created ack
            handle->state = STATE_READY;
            XMPP_IBB_Open_CB open_cb = ibb_ops_p->ibb_open_fp;
            (*open_cb)(handle);
        }
        else if( handle->state == STATE_READY )
        {
            //data sent ack
            XMPP_IBB_Send_CB send_cb = ibb_ops_p->ibb_send_fp;
            handle->data_ack_count++;
            (*send_cb)(handle);
        }
    }

    return 1;
}


int iq_ibb_open_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza, void * const userdata)
{
printf("in ibb_open_handle\n");
    XMPP_IBB_Ops_t* ibb_ops_p = (XMPP_IBB_Ops_t*) userdata;
    xmpp_ibb_session_t* session_p;

    if (xmpp_stanza_get_child_by_name(stanza, "open") != NULL) {

        session_p = malloc(sizeof(xmpp_ibb_session_t));
        session_p->conn = conn;
        snprintf( session_p->id, sizeof(session_p->id), "%s", xmpp_stanza_get_id(stanza) );
        snprintf( session_p->sid, sizeof(session_p->sid), "%s", XMPP_IBB_Get_Sid(stanza) );
        session_p->block_size = atoi( xmpp_stanza_get_attribute(xmpp_stanza_get_child_by_name(stanza, "open"), "block-size") );
        snprintf( session_p->peer, sizeof(session_p->peer), "%s", xmpp_stanza_get_attribute(stanza, "from") );
        session_p->state = STATE_READY;
        session_p->send_seq = -1;
        session_p->recv_seq = -1;
        session_p->data_ack_count = -1;
        session_p->recv_data = NULL;
        session_p->next = NULL;

        ibb_add_handle_to_queue( session_p );
        XMPP_IBB_Ack_Send( session_p );

        XMPP_IBB_Open_CB open_cb;
        open_cb = ibb_ops_p->ibb_open_fp;
        (*open_cb)(session_p);

    } else if (xmpp_stanza_get_child_by_name(stanza, "data") != NULL) {

        session_p = ibb_get_handle_from_queue( xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza) );
        if( session_p != NULL )
        {
            char *intext = xmpp_stanza_get_text(xmpp_stanza_get_child_by_name(stanza, "data"));
            XMPP_IBB_Ack_Send( session_p );
            session_p->recv_seq = atoi( xmpp_stanza_get_attribute( xmpp_stanza_get_child_by_name( stanza, "data" ), "seq" ) );
            session_p->recv_data = base64_decode( conn->ctx, intext, strlen(intext) );
            XMPP_IBB_Recv_CB recv_fp = ibb_ops_p->ibb_recv_fp;
            (*recv_fp)(session_p);
        }

    } else if (xmpp_stanza_get_child_by_name(stanza, "close") != NULL) {

        session_p = ibb_get_handle_from_queue( xmpp_stanza_get_id(stanza), XMPP_IBB_Get_Sid(stanza) );
        if( session_p != NULL )
        {
            printf("in close session_p != nULL\n");
            XMPP_IBB_Ack_Send( session_p );
            session_p->state = STATE_CLOSED;
            XMPP_IBB_Close_CB close_fp = ibb_ops_p->ibb_close_fp;
            (*close_fp)( session_p );
        }
        printf("in close session_p == NULL\n");
    }
        
    time(&glast_ping_time);

    return 1;
}

//return seq number
int XMPP_IBB_Send( xmpp_ibb_session_t *handle, char *message )
{
    if( ibb_check_handle( handle ) == 0 )
    {
        xmpp_stanza_t *iq, *data, *text;
        char *encode, seqchar[16]="";

        iq = xmpp_stanza_new( handle->conn->ctx );
        data = xmpp_stanza_new( handle->conn->ctx );
        text = xmpp_stanza_new( handle->conn->ctx );
        
        xmpp_stanza_set_name( iq, "iq" );
        xmpp_stanza_set_type( iq, "set" );
        xmpp_stanza_set_id( iq, handle->id );
        xmpp_stanza_set_attribute( iq, "to", handle->peer );
        xmpp_stanza_set_attribute( iq, "from", handle->conn->jid );

        xmpp_stanza_set_name( data, "data" );
        xmpp_stanza_set_ns( data, XMLNS_IBB );
        xmpp_stanza_set_attribute( data, "sid", handle->sid );
        snprintf( seqchar, sizeof(seqchar), "%d", ++handle->send_seq );
        xmpp_stanza_set_attribute( data, "seq", seqchar );

        encode = base64_encode( handle->conn->ctx, (unsigned char*)message, strlen(message) );
        xmpp_stanza_set_text_with_size( text, encode, strlen(encode) );

        xmpp_stanza_add_child( data, text );
        xmpp_stanza_add_child( iq, data );
        xmpp_send( handle->conn, iq );

        //handle->send_seq++;
        xmpp_free( handle->conn->ctx, encode );
        xmpp_stanza_release(text);
        xmpp_stanza_release(data);
        xmpp_stanza_release(iq);
        
        return handle->send_seq;
    }
    return -1;
}

//return 0 for success -1 for failure
int XMPP_IBB_Establish( xmpp_conn_t * const conn, char *destination, xmpp_ibb_session_t *session_handle )
{
    xmpp_ctx_t *ctx = conn->ctx;
    xmpp_stanza_t *iq, *open;

    char *sizetemp = "4096";
    char idtemp[9]="", sidtemp[9]="";
    snprintf( idtemp, sizeof(idtemp), "%s", generate_random_id() );
    snprintf( sidtemp, sizeof(sidtemp) ,"%s", generate_random_id() );
    
    iq = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(iq, "iq");
    xmpp_stanza_set_type(iq, "set");
    xmpp_stanza_set_id(iq, idtemp);
    xmpp_stanza_set_attribute(iq, "from", conn->jid);
    xmpp_stanza_set_attribute(iq, "to", destination);

    open = xmpp_stanza_new(ctx);
    xmpp_stanza_set_name(open, "open");
    xmpp_stanza_set_ns(open, XMLNS_IBB);
    xmpp_stanza_set_attribute(open, "block-size", sizetemp );
    xmpp_stanza_set_attribute(open, "sid", sidtemp);
    xmpp_stanza_set_attribute(open, "stanza", "iq");

    xmpp_stanza_add_child(iq, open);
    xmpp_stanza_release(open);
    xmpp_send(conn, iq);
    xmpp_stanza_release(iq);

    if( session_handle != NULL )
    {
        session_handle->conn = conn;
        snprintf( session_handle->id, sizeof(session_handle->id), "%s", idtemp );
        snprintf( session_handle->sid, sizeof(session_handle->sid), "%s", sidtemp );
        session_handle->block_size = atoi(sizetemp);
        session_handle->conn = conn;
        snprintf( session_handle->peer, sizeof(session_handle->peer), "%s", destination );
        session_handle->state = STATE_WAITING;
        session_handle->send_seq = -1;
        session_handle->recv_seq = -1;
        session_handle->data_ack_count = -1;
        session_handle->recv_data = NULL;
        session_handle->next = NULL;

        /*
        if( session_handle != session_handle->next )
            printf("before add, session_handle != handle->next\n");
        else
            printf("before add, session_hadnel == handle->nect\n");
        */

        ibb_add_handle_to_queue( session_handle );
//printf("8\n");
        return 0;
    }
    else
    {
//printf("9\n");
        return -1;
    }
}

void XMPP_IBB_Ack_Send( xmpp_ibb_session_t *handle )
{
    xmpp_stanza_t *iq;

    iq = xmpp_stanza_new( handle->conn->ctx );
    xmpp_stanza_set_name( iq, "iq" );
    xmpp_stanza_set_type( iq, "result" );

    xmpp_stanza_set_id( iq, handle->id );
    xmpp_stanza_set_attribute( iq, "to", handle->peer );
    xmpp_stanza_set_attribute( iq, "from", handle->conn->jid );
    xmpp_send( handle->conn, iq );
    xmpp_stanza_release( iq );
}


void XMPP_IBB_Close( xmpp_ibb_session_t *handle )
{
    xmpp_stanza_t *iq, *close;

    iq = xmpp_stanza_new( handle->conn->ctx );
    close = xmpp_stanza_new( handle->conn->ctx );

    xmpp_stanza_set_name( iq, "iq" );
    xmpp_stanza_set_type( iq, "set" );
    xmpp_stanza_set_id( iq, handle->id );
    xmpp_stanza_set_attribute( iq, "to", handle->peer );
    xmpp_stanza_set_attribute( iq, "from", handle->conn->jid );

    xmpp_stanza_set_name( close, "close" );
    xmpp_stanza_set_ns( close, XMLNS_IBB );
    xmpp_stanza_set_attribute( close, "sid", handle->sid );

    xmpp_stanza_add_child( iq, close );
    xmpp_send( handle->conn, iq );
    xmpp_stanza_release( close );
    xmpp_stanza_release( iq );

    handle->state = STATE_CLOSED;
    ibb_delete_handle_from_queue( handle );
}

char* XMPP_IBB_Get_Sid(xmpp_stanza_t * const stanza)
{
    if( xmpp_stanza_get_child_by_name(stanza, "open") != NULL )
        return xmpp_stanza_get_attribute(xmpp_stanza_get_child_by_name(stanza, "open"), "sid");
    else if( xmpp_stanza_get_child_by_name(stanza, "data") != NULL )
        return xmpp_stanza_get_attribute(xmpp_stanza_get_child_by_name(stanza, "data"), "sid");
    else 
        return NULL;

}

/* Initialize IBB handle. */
void XMPP_IBB_Init(xmpp_conn_t * const conn, XMPP_IBB_Ops_t* ibb_ops)
{
    srand(time(NULL)); //for generate random string
    xmpp_handler_add( conn, iq_ibb_open_handler, XMLNS_IBB, "iq", "set", ibb_ops );
    xmpp_handler_add( conn, ibb_ack_handler, NULL, "iq", "result", ibb_ops );
    xmpp_handler_add( conn, ibb_error_handler, NULL, "iq", "error", ibb_ops );

    //gXMPP_IBB_handle = malloc(sizeof(xmpp_ibb_session_t));

    /*
    gXMPP_IBB_handle_head->conn = NULL;
    memset( gXMPP_IBB_handle_head->sid, 0, MAX_SID_LEN );
    gXMPP_IBB_handle_head->block_size = 0;
    memset( gXMPP_IBB_handle_head->peer, 0, MAX_JID_LEN );
    gXMPP_IBB_handle_head->send_seq = 0;
    gXMPP_IBB_handle_head->recv_seq = 0;
    gXMPP_IBB_handle_head->recv_data = NULL;
    gXMPP_IBB_handle_head->next = NULL;

    gXMPP_IBB_handle_tail->conn = NULL;
    memset( gXMPP_IBB_handle_tail->sid, 0, MAX_SID_LEN );
    gXMPP_IBB_handle_tail->block_size = 0;
    memset( gXMPP_IBB_handle_tail->peer, 0, MAX_JID_LEN );
    gXMPP_IBB_handle_tail->recv_seq = 0;
    gXMPP_IBB_handle_tail->send_seq = 0;
    gXMPP_IBB_handle_tail->recv_data = NULL;
    gXMPP_IBB_handle_tail->next = NULL;
    */
}

