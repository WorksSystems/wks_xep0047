/*
 * wksxmpp_types.h
 *
 *  Created on: Aug 5, 2015
 *      Author: user
 */

#include <stdlib.h>

#ifndef INC_WKSXMPP_TYPES_H_
#define INC_WKSXMPP_TYPES_H_

typedef struct _wksxmpp_data_t {
    void    *data;
    int      size;
    char    *tojid;
    char    *from;
} wksxmpp_data_t;

typedef struct _wksxmpp_conninfo_t {
    int      connevent;
    int      error;
    int      errortype;
    char    *errortext;
} wksxmpp_conninfo_t;

#endif /* INC_WKSXMPP_TYPES_H_ */
