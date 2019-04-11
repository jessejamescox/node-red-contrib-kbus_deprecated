##----------------------------------------------------------------------------------------------------------------------
### Copyright (c) WAGO Kontakttechnik GmbH & Co. KG
###
### PROPRIETARY RIGHTS of WAGO Kontakttechnik GmbH & Co. KG are involved in
### the subject matter of this material. All manufacturing, reproduction,
### use, and sales rights pertaining to this subject matter are governed
### by the license agreement. The recipient of this software implicitly
### accepts the terms of the license.
###
##----------------------------------------------------------------------------------------------------------------------
##----------------------------------------------------------------------------------------------------------------------
###
###  \file     Makefile
###
###  \version  $Revision: 0001 $
###
###  \brief    Makefile for a set of "C" programs for using KBUS interface with NodeJS and Python
###
###  \author   cm : Jesse Cox
##----------------------------------------------------------------------------------------------------------------------

#
# add more flags here, but don't remove the "+="
#
CFLAGS += -O0 -g3  -Wall #-O2 -g   -Wsign-compare -Wfloat-equal -Wformat-security #-Werror
CFLAGS += -I$(SYSROOT)/usr/include/OsLinux/

LDFLAGS += -lpthread -lrt -lffi -lglib-2.0 -llibloader -ldbuskbuscommon -ljson-c -lmosquitto
EXTRALIBS += $(DIST_DIR)/sysroot-target/usr/lib/libdal.a

all: kbusproc

kbusproc: kbusproc.o
	$(CC) -o kbusproc kbusproc.o $(EXTRALIBS) $(LDFLAGS)

clean:
	-rm -f kbusproc

install:


.PHONY: all install clean

# end of development makefile

