#ifndef __XMPP_UTILS_H__
#define __XMPP_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

char *xmpp_b64encode(const char *data, size_t dlen, char **encdata);

char *xmpp_b64decode(const char *encdata, char **decdata, size_t *dlen);

void xmpp_b64free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif//__XMPP_UTILS_H__
