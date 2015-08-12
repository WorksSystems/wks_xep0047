#ifndef __XMPP_UTILS_H__
#define __XMPP_UTILS_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

char *xmpp_b64encode(const char *data, size_t dlen, char **encdata);

char *xmpp_b64decode(const char *encdata, char **decdata, size_t *dlen);

void xmpp_b64free(void *ptr);

typedef struct _ilist_t ilist_t;
typedef bool (*find_fp)(void *item, void *key);

ilist_t * ilist_new();

void ilist_destroy(ilist_t *il);

void ilist_add(ilist_t *il, void *item);

void ilist_remove(ilist_t *il, void *item);

void * ilist_finditem_func(ilist_t *il, find_fp ff, void *key);

bool ilist_foundinlist(ilist_t *il, void * item);

#ifdef __cplusplus
}
#endif

#endif//__XMPP_UTILS_H__
