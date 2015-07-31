.HONY: all clean

XMPPLIB   = /root/libstrophe-2

SRCPATH = ../source
HDRPATH = ../include

BUILDPATH = ../build
OBJPATH = $(BUILDPATH)/obj
BINPATH = $(BUILDPATH)/bin
LIBPATH = $(BUILDPATH)/lib
XMPPLIB_INC = $(XMPPLIB)
XMPPLIB_INC_SRC = $(XMPPLIB)/src
INC_DIRS += -I$(XMPPLIB_INC)
INC_DIRS += -I$(XMPPLIB_INC_SRC)
INC_DIRS += -I$(HDRPATH)


CC = gcc
CFLAGS = -O -Wall  -Wl,--gc-sections -Wl,-s -std=gnu99
DEFS = #-DTB_LOG

VPATH = $(SRCPATH)
SRCS=$(wildcard $(SRCPATH)/*.c) 
HEADERS=$(wildcard $(HDRPATH)/*.h)
NOTDIRSRCS=$(notdir $(SRCS))
OBJS = $(patsubst %.c,$(OBJPATH)/%.o,$(NOTDIRSRCS))

LIBS = $(LIBPATH)/libstrophe.a -lcrypto -lssl -lexpat -lresolv


all: wks_xep0047

wks_xep0047: $(OBJS)
	@mkdir -p $(BINPATH)
	$(CC) -z muldefs  -o $(BINPATH)/$@ $^ $(LIBS)
	@echo Compile WKS-OIC-RACLIENT successful....

$(OBJS): $(OBJPATH)/%.o: %.c
	@mkdir -p $(OBJPATH)
	$(CC) $(CFLAGS) -c -o $@  $< $(INC_DIRS) $(DEFS)

clean:
	rm -rf $(BINPATH) $(OBJPATH) $(MIPS_OBJPATH) $(X86_OBJPATH)
