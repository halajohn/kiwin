# KiWin - A small GUI for the embedded system
# Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

LIBDIR          = $(KWDIR)/lib
PREFIX          = $(KWDIR)/Program_files/demo2
BINDIR          = $(PREFIX)/bin

CC		= /usr/bin/gcc-3.2
MKDIR		= /bin/mkdir
CP              = /bin/cp
RM		= /bin/rm
SED             = /bin/sed
STRIP           = /usr/bin/strip

demo2           = demo2
demo2_g         = demo2_g
demo2ObjFiles   = demo2.o
demo2ObjFiles_g = demo2.go

INCPATH         = -I$(KWDIR)/include

CFLAGS          = -Wall -O2 $(INCPATH)
LDFLAGS         = -L$(LIBDIR)
LIBS            = -lkwlog -lkwgscli

CFLAGS_G        = -Wall -g3 -gdwarf-2 $(INCPATH)
LDFLAGS_G       = -L$(LIBDIR)
LIBS_G          = -lkwlog_g -lkwgscli_g

%.o:    %.c
	@echo " --- Compiling the object code               $@"
	@$(CC) $(CFLAGS) -c $< -o $@

%.go:   %.c
	@echo " --- Compiling the object code (debug mode)  $@"
	@$(CC) $(CFLAGS_G) -c $< -o $@

all: depend $(demo2) $(demo2_g)

$(demo2):  $(demo2ObjFiles)
	@echo " --- Linking demo2 excutable"
	@$(CC) $(LDFLAGS) -o $(demo2) $(demo2ObjFiles) $(LIBS)
	@$(STRIP) $(demo2)

$(demo2_g): $(demo2ObjFiles_g)
	@echo " --- Linking demo2 excutable (debug mode)"
	@$(CC) $(LDFLAGS_G) -o $(demo2_g) $(demo2ObjFiles_g) $(LIBS_G)

clean:
	@$(RM) -f *~ .depend .depend_g \
	$(demo2ObjFiles) $(demo2ObjFiles_g) \
	$(demo2) $(demo2_g)

install: all
	@if [ ! -e $(BINDIR) ]; then \
		$(MKDIR) -p $(BINDIR); \
	fi
	@$(CP) $(demo2) $(BINDIR)
	@$(CP) $(demo2_g) $(BINDIR)

depend: .depend .depend_g

ifeq (.depend,$(wildcard .depend))
include .depend
endif

ifeq (.depend_g,$(wildcard .depend_g))
include .depend_g
endif

.depend: *.c
	@echo " --- Make dependencies of demo2 object files"
	@$(CC) $(CFLAGS) -M -E *.c > .depend

.depend_g: *.c
	@echo " --- Make dependencies of demo2 object files (debug mode)"
	@$(CC) $(CFLAGS_G) -M -E *.c > .depend_g_orig
	@$(SED) 's/\(.*\)\.o/\1\.go/g' .depend_g_orig > .depend_g
	@$(RM) -f .depend_g_orig
