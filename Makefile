XMPPLIB  ?= ../libstrophe

SRCPATH = ./source
HDRPATH = ./include
EXMPATH = ./example

XMPPLIB_LIB = ${XMPPLIB}/.libs
XMPPLIB_INC = ${XMPPLIB}
INC_DIRS += -I${XMPPLIB_INC}
INC_DIRS += -I${HDRPATH}


LIBNAME=wksxmppxep
SHAREDLIB=lib${LIBNAME}.so
STATICLIB=lib${LIBNAME}.a

CC = gcc
AR = ar
#CFLAGS = -O -Wall -Werror -Wl,--gc-sections -Wl,-s -std=gnu99 -fPIC
CFLAGS = -Wall -Werror -fPIC
LDFLAGS = -L./ -L${XMPPLIB_LIB} -l${LIBNAME} -lstrophe -lssl -lcrypto -lexpat -lm -lpthread
DEFS = #-DTB_LOG

VPATH = ${SRCPATH}
SRCS=${wildcard ${SRCPATH}/*.c} 
HEADERS=${wildcard ${HDRPATH}/*.h}
OBJS = ${patsubst %.c,%.o,${SRCS}}

MSRCS=${wildcard ${EXMPATH}/*.c} 
MOBJS = ${patsubst %.c,%.o,${MSRCS}}

TESTTARGET=main main_chat

all: libraries testprogram

testprogram: ${TESTTARGET}

main: example/main.o
	${CC} -o $@ $^ ${LDFLAGS}

main_chat: example/main_chat.o
	${CC} -o $@ $^ ${LDFLAGS}

libraries: ${SHAREDLIB} ${STATICLIB}

${STATICLIB}: ${OBJS}
	${AR} rcs $@ $^
	@echo Compile $@ successful....

${SHAREDLIB}: ${OBJS}
	${CC} -o $@ -shared $^
	@echo Compile $@ successful....

.c.o:
	${CC} ${CFLAGS} -o $@ -c $< ${INC_DIRS} ${DEFS}

clean:
	rm -rf ${OBJS} ${SHAREDLIB} ${STATICLIB} ${TESTTARGET} ${MOBJS}
