#include <stdio.h>
#include <string.h>
#include <math.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "xmpp_utils.h"

char *xmpp_b64encode(const char *data, size_t dlen, char **encdata)
{
    BIO *bio, *b64;
    FILE *stream;
    int encSize = 4 * ceil((double)dlen/3);

    *encdata = (char *) malloc(encSize + 1);
    stream = fmemopen((void *) *encdata, encSize + 1, "w");

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_fp(stream, BIO_NOCLOSE);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, dlen);
    BIO_flush(bio);

    BIO_free_all(bio);
    fclose(stream);

    return *encdata;
}

static size_t decDataLength(const char* encdata) {
      
    int len = strlen(encdata);
    int padding = 0;

    if (encdata[len-1] == '=' && encdata[len-2] == '=') //last two chars are =
        padding = 2;
    else if (encdata[len-1] == '=') //last char is =
        padding = 1;

    return (size_t)((len * 3) / 4) - padding;
}

char *xmpp_b64decode(const char *encdata, char **decdata, size_t *dlen)
{
    BIO *bio, *b64;
    FILE *stream;
    int  len;

    *dlen = decDataLength(encdata);
    *decdata = (char *) malloc((*dlen) + 1);
    stream = fmemopen((void *) encdata, strlen(encdata), "r");

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_fp(stream, BIO_NOCLOSE);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    len = BIO_read(bio, *decdata, strlen(encdata));
    (*decdata)[len] = '\0';
    BIO_free_all(bio);
    fclose(stream);

    return *decdata;
}

void xmpp_b64free(void *ptr)
{
    free(ptr);
}

typedef struct _node_t {
    void *item;
    struct _node_t *next;
} node_t ;

struct _ilist_t {
    int size;
    node_t *head;
//    node_t *tail;
};

ilist_t * ilist_new()
{
    ilist_t *il;
    il = (ilist_t *) malloc(sizeof(struct _ilist_t));
    il->size = 0;
    il->head = NULL;
//    il->tail = NULL;
    return il;
}

static node_t * ilist_find_node(ilist_t *il, void *item)
{
    node_t *nd;
    for (nd = il->head; nd != NULL; nd = nd->next) {
        if (nd->item == item) return nd;
    }
    return NULL;
}

static void ilist_remove_node(ilist_t *il, node_t *nd)
{
    node_t *tmp;
    if(nd == NULL)
        return;
    if (il->head == nd) {
        il->head = nd->next;
    } else {
        for (tmp = il->head; tmp != NULL; tmp = tmp->next) {
            if (tmp->next == nd) {
                tmp->next = nd->next;
                break;
            }
        }
    }
    il->size--;
    free(nd);
}

void ilist_destroy(ilist_t *il)
{
    node_t *nd;
    while ((nd = il->head) != NULL) {
        ilist_remove_node(il, nd);
    }
    free(il);
}

void ilist_add(ilist_t *il, void *item)
{
    node_t *nd;
    nd = (node_t *) malloc(sizeof(struct _node_t));
    nd->item = item;
    nd->next = il->head;
    il->head = nd;
    il->size++;
}

void ilist_remove(ilist_t *il, void *item)
{
    node_t *nd;
    nd = ilist_find_node(il, item);
    if (nd != NULL) {
        ilist_remove_node(il, nd);
    }
}

void * ilist_finditem_func(ilist_t *il, find_fp ff, void *key)
{
    node_t *nd;
    if (il == NULL || ff == NULL || key == NULL )
        return NULL;
    for (nd = il->head; nd != NULL; nd = nd->next) {
        if (ff(nd->item, key)) {
//            printf("\n  %s-%d: %s(%s) found!!", __FILE__, __LINE__, __FUNCTION__, (char *) key);
            return nd->item;
        }
    }
    return NULL;
}

bool ilist_foundinlist(ilist_t *il, void * item)
{
    node_t *nd;
    for (nd = il->head; nd != NULL; nd = nd->next) {
        if (nd->item == item) {
            return true;
        }
    }
    return false;
}