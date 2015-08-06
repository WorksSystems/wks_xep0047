#ifndef __WKS_XMPP_UTILS_H__
#define __WKS_XMPP_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

char *wksxmpp_b64encode(const char *data, size_t dlen, char **encdata);

char *wksxmpp_b64decode(const char *encdata, char **decdata, size_t *dlen);

void wksxmpp_b64free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif//__WKS_XMPP_UTILS_H__
