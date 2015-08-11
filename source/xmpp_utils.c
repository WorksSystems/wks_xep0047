#include <stdio.h>
#include <string.h>
#include <math.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

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
