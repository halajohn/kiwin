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
PREFIX          = $(KWDIR)/Program_files/kwRegFile
BINDIR          = $(PREFIX)/bin

REGDIR          = $(KWDIR)/etc
TXTDIR          = $(KWDIR)/src/etc_src

CC		= /usr/bin/gcc-3.2
MKDIR		= /bin/mkdir
CP              = /bin/cp
RM		= /bin/rm
SED             = /bin/sed
STRIP           = /usr/bin/strip

txt2reg           = txt2reg
txt2reg_g         = txt2reg_g
txt2regObjFiles   = txt2reg.o
txt2regObjFiles_g = txt2reg.go

getattr           = getattr
getattr_g         = getattr_g
getattrObjFiles   = getattr.o
getattrObjFiles_g = getattr.go

reg_txt       = $(TXTDIR)/reg.txt
reg_reg       = $(REGDIR)/regfile.reg

INCPATH         = -I$(KWDIR)/include

CFLAGS          = -Wall -O2 $(INCPATH)
LDFLAGS         = -L$(LIBDIR)
LIBS            = -lkwgdbm -lkwlog

CFLAGS_G        = -Wall -g3 -gdwarf-2 $(INCPATH)
LDFLAGS_G       = -L$(LIBDIR)
LIBS_G          = -lkwgdbm_g -lkwlog_g

%.o:    %.c
	@echo " --- Compiling the object code               $@"
	@$(CC) $(CFLAGS) -c $< -o $@

%.go:   %.c
	@echo " --- Compiling the object code (debug mode)  $@"
	@$(CC) $(CFLAGS_G) -c $< -o $@

all: depend $(txt2reg) $(txt2reg_g) $(getattr) $(getattr_g) $(reg_txt) $(reg_reg)

$(txt2reg):  $(txt2regObjFiles)
	@echo " --- Linking txt2reg excutable"
	@$(CC) $(LDFLAGS) -o $(txt2reg) $(txt2regObjFiles) $(LIBS)
	@$(STRIP) $(txt2reg)

$(txt2reg_g): $(txt2regObjFiles_g)
	@echo " --- Linking txt2reg excutable (debug mode)"
	@$(CC) $(LDFLAGS_G) -o $(txt2reg_g) $(txt2regObjFiles_g) $(LIBS_G)

$(getattr):  $(getattrObjFiles)
	@echo " --- Linking getattr excutable"
	@$(CC) $(LDFLAGS) -o $(getattr) $(getattrObjFiles) $(LIBS)
	@$(STRIP) $(getattr)

$(getattr_g): $(getattrObjFiles_g)
	@echo " --- Linking getattr excutable (debug mode)"
	@$(CC) $(LDFLAGS_G) -o $(getattr_g) $(getattrObjFiles_g) $(LIBS_G)

$(reg_txt):
	@if [ ! -e $(TXTDIR) ]; then \
		$(MKDIR) -p $(TXTDIR); \
	fi
	@echo -n "root_wnd_path " > $(reg_txt)
	@echo "$(KWDIR)/Program_files/kwRootWnd/bin/kwRootWnd" >> $(reg_txt)
	@echo "default_border_size 4" >> $(reg_txt)
	@echo "default_caption_height 20" >> $(reg_txt)
	@echo "default_border_bg_color 808080" >> $(reg_txt)
	@echo "default_caption_light_color 80" >> $(reg_txt)
	@echo "default_caption_dark_color 808080" >> $(reg_txt)

$(reg_reg): $(txt2reg) $(reg_txt)
	@if [ ! -e $(REGDIR) ]; then \
		$(MKDIR) -p $(REGDIR); \
	fi
	@echo " --- Making the KiWin system register file"
	@./$(txt2reg) -s $(reg_txt) -d $(reg_reg)

clean:
	@$(RM) -f *~ .depend .depend_g \
	$(txt2regObjFiles) $(txt2regObjFiles_g) $(getattrObjFiles) $(getattrObjFiles_g) \
	$(txt2reg) $(txt2reg_g) \
	$(getattr) $(getattr_g) \
	$(reg_txt) $(reg_reg)
	@$(RM) -rf $(TXTDIR)

install: all
	@if [ ! -e $(BINDIR) ]; then \
		$(MKDIR) -p $(BINDIR); \
	fi
	@$(CP) $(txt2reg) $(BINDIR)
	@$(CP) $(txt2reg_g) $(BINDIR)
	@$(CP) $(getattr) $(BINDIR)
	@$(CP) $(getattr_g) $(BINDIR)

depend: .depend .depend_g

ifeq (.depend,$(wildcard .depend))
include .depend
endif

ifeq (.depend_g,$(wildcard .depend_g))
include .depend_g
endif

.depend: *.c
	@echo " --- Make dependencies of reg tools object files"
	@$(CC) $(CFLAGS) -M -E *.c > .depend

.depend_g: *.c
	@echo " --- Make dependencies of reg tools object files (debug mode)"
	@$(CC) $(CFLAGS_G) -M -E *.c > .depend_g_orig
	@$(SED) 's/\(.*\)\.o/\1\.go/g' .depend_g_orig > .depend_g
	@$(RM) -f .depend_g_orig
