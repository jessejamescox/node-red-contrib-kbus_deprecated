##----------------------------------------------------------------------------------------------------------------------
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation, either version 3 of the License, or
##    (at your option) any later version.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

