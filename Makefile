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

export KWDIR = $(PWD)

export MakDir       = $(KWDIR)/mak
export ObjDir       = $(KWDIR)/obj
export LibDir       = $(KWDIR)/lib
export EtcDir       = $(KWDIR)/etc
export SrvBinDir    = $(KWDIR)/SrvBin
export ProgramDir   = $(KWDIR)/Program_files

include $(MakDir)/Rules.config
include $(MakDir)/Rules.config.lib

all:	PREWORK \
	$(LIB_KW_LOG_NAME) $(LIB_KW_LOG_NAME_G) \
	$(LIB_KW_PNG_NAME) $(LIB_KW_PNG_NAME_G) \
	$(LIB_KW_Z_NAME) $(LIB_KW_Z_NAME_G) \
	$(LIB_KW_FREETYPE_NAME) $(LIB_KW_FREETYPE_NAME_G) \
	$(LIB_KW_ICONV_NAME) $(LIB_KW_ICONV_NAME_G) \
	$(LIB_KW_GDBM_NAME) $(LIB_KW_GDBM_NAME_G) \
	$(LIB_KW_COLOR_NAME) $(LIB_KW_COLOR_NAME_G) \
	$(LIB_KW_REGFILE_NAME) $(LIB_KW_REGFILE_NAME_G) \
	$(LIB_KW_GSCLI_NAME) $(LIB_KW_GSCLI_NAME_G) \
	$(LIB_KW_XPM_NAME) $(LIB_KW_XPM_NAME_G) \
	$(KIWIN) $(KIWIN_G) \
	POSTWORK

PREWORK:
	@if [ ! -e $(ObjDir) ]; then \
		$(MKDIR) $(ObjDir); \
	fi
	@if [ ! -e $(LibDir) ]; then \
		$(MKDIR) $(LibDir); \
	fi
	@if [ ! -e $(SrvBinDir) ]; then \
		$(MKDIR) $(SrvBinDir); \
	fi
	@for i in $(SUBDIRS); do \
		cd $$i; \
		$(MAKE) || exit 1; \
		cd $(KWDIR); \
	done

POSTWORK:
	@$(ECHO) " --- update the shared library cache"
	#@$(LDCONFIG) -f /etc/ld.so.conf -C /etc/ld.so.cache /lib
	#@$(LDCONFIG) -f /etc/ld.so.conf /lib

$(KIWIN): $(KwGsSrvObjFiles)
	@echo " --- Linking kwgssrv executable";
	@$(CC) $(LDFLAGS) -o $(KIWIN) $(KwGsSrvObjFiles) $(LIBS)
	@$(STRIP) $(KIWIN)

$(KIWIN_G): $(KwGsSrvObjFiles_g)
	@echo " --- Linking kwgssrv executable (debug mode)";
	@$(CC) $(LDFLAGS) -o $(KIWIN_G) $(KwGsSrvObjFiles_g) $(LIBS_G);

$(LIB_KW_PNG_NAME): $(LibKwPngObjFiles)
	@echo " --- Linking libkwpng library"
	@$(CC) $(LIB_KW_PNG_LDFLAGS) -o $(LIB_KW_PNG_NAME) $(LibKwPngObjFiles) $(LIB_KW_PNG_LIBS);
	@if [ ! -e $(LIB_KW_PNG_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_PNG_NAME) $(LIB_KW_PNG_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_PNG_NAME);

$(LIB_KW_PNG_NAME_G): $(LibKwPngObjFiles_g)
	@echo " --- Linking libkwpng library (debug mode)"
	@$(CC) $(LIB_KW_PNG_LDFLAGS_G) -o $(LIB_KW_PNG_NAME_G) $(LibKwPngObjFiles_g) $(LIB_KW_PNG_LIBS_G);
	@if [ ! -e $(LIB_KW_PNG_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_PNG_NAME_G) $(LIB_KW_PNG_LNNAME_G); \
	fi;

$(LIB_KW_Z_NAME): $(LibKwZObjFiles)
	@echo " --- Linking libkwz library"
	@$(CC) $(LIB_KW_Z_LDFLAGS) -o $(LIB_KW_Z_NAME) $(LibKwZObjFiles) $(LIB_KW_Z_LIBS);
	@if [ ! -e $(LIB_KW_Z_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_Z_NAME) $(LIB_KW_Z_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_Z_NAME);

$(LIB_KW_Z_NAME_G): $(LibKwZObjFiles_g)
	@echo " --- Linking libkwz library (debug mode)"
	@$(CC) $(LIB_KW_Z_LDFLAGS_G) -o $(LIB_KW_Z_NAME_G) $(LibKwZObjFiles_g) $(LIB_KW_Z_LIBS_G);
	@if [ ! -e $(LIB_KW_Z_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_Z_NAME_G) $(LIB_KW_Z_LNNAME_G); \
	fi;

$(LIB_KW_FREETYPE_NAME): $(LibKwFreetypeObjFiles)
	@echo " --- Linking libkwfreetype library"
	@$(CC) $(LIB_KW_FREETYPE_LDFLAGS) -o $(LIB_KW_FREETYPE_NAME) $(LibKwFreetypeObjFiles) $(LIB_KW_FREETYPE_LIBS);
	@if [ ! -e $(LIB_KW_FREETYPE_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_FREETYPE_NAME) $(LIB_KW_FREETYPE_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_FREETYPE_NAME);

$(LIB_KW_FREETYPE_NAME_G): $(LibKwFreetypeObjFiles_g)
	@echo " --- Linking libkwfreetype library (debug mode)"
	@$(CC) $(LIB_KW_FREETYPE_LDFLAGS_G) -o $(LIB_KW_FREETYPE_NAME_G) $(LibKwFreetypeObjFiles_g) $(LIB_KW_FREETYPE_LIBS_G);
	@if [ ! -e $(LIB_KW_FREETYPE_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_FREETYPE_NAME_G) $(LIB_KW_FREETYPE_LNNAME_G); \
	fi;

$(LIB_KW_GSCLI_NAME): $(LibKwGsCliObjFiles)
	@echo " --- Linking libkwgscli library"
	@$(CC) $(LIB_KW_GSCLI_LDFLAGS) -o $(LIB_KW_GSCLI_NAME) $(LibKwGsCliObjFiles) $(LIB_KW_GSCLI_LIBS);
	@if [ ! -e $(LIB_KW_GSCLI_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_GSCLI_NAME) $(LIB_KW_GSCLI_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_GSCLI_NAME);

$(LIB_KW_GSCLI_NAME_G): $(LibKwGsCliObjFiles_g)
	@echo " --- Linking libkwgscli library (debug mode)"
	@$(CC) $(LIB_KW_GSCLI_LDFLAGS_G) -o $(LIB_KW_GSCLI_NAME_G) $(LibKwGsCliObjFiles_g) $(LIB_KW_GSCLI_LIBS_G);
	@if [ ! -e $(LIB_KW_GSCLI_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_GSCLI_NAME_G) $(LIB_KW_GSCLI_LNNAME_G); \
	fi;

$(LIB_KW_GDBM_NAME): $(LibKwGdbmObjFiles)
	@echo " --- Linking libkwgdbm library"
	@$(CC) $(LIB_KW_GDBM_LDFLAGS) -o $(LIB_KW_GDBM_NAME) $(LibKwGdbmObjFiles) $(LIB_KW_GDBM_LIBS);
	@if [ ! -e $(LIB_KW_GDBM_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_GDBM_NAME) $(LIB_KW_GDBM_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_GDBM_NAME);

$(LIB_KW_GDBM_NAME_G): $(LibKwGdbmObjFiles_g)
	@echo " --- Linking libkwgdbm library (debug mode)"
	@$(CC) $(LIB_KW_GDBM_LDFLAGS_G) -o $(LIB_KW_GDBM_NAME_G) $(LibKwGdbmObjFiles_g) $(LIB_KW_GDBM_LIBS_G);
	@if [ ! -e $(LIB_KW_GDBM_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_GDBM_NAME_G) $(LIB_KW_GDBM_LNNAME_G); \
	fi;

$(LIB_KW_LOG_NAME): $(LibKwLogObjFiles)
	@echo " --- Linking libkwlog library"
	@$(CC) $(LIB_KW_LOG_LDFLAGS) -o $(LIB_KW_LOG_NAME) $(LibKwLogObjFiles) $(LIB_KW_LOG_LIBS);
	@if [ ! -e $(LIB_KW_LOG_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_LOG_NAME) $(LIB_KW_LOG_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_LOG_NAME);

$(LIB_KW_LOG_NAME_G): $(LibKwLogObjFiles_g)
	@echo " --- Linking libkwlog library (debug mode)"
	@$(CC) $(LIB_KW_LOG_LDFLAGS_G) -o $(LIB_KW_LOG_NAME_G) $(LibKwLogObjFiles_g) $(LIB_KW_LOG_LIBS_G);
	@if [ ! -e $(LIB_KW_LOG_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_LOG_NAME_G) $(LIB_KW_LOG_LNNAME_G); \
	fi;

$(LIB_KW_XPM_NAME): $(LibKwXpmObjFiles)
	@echo " --- Linking libkwxpm library"
	@$(CC) $(LIB_KW_XPM_LDFLAGS) -o $(LIB_KW_XPM_NAME) $(LibKwXpmObjFiles) $(LIB_KW_XPM_LIBS);
	@if [ ! -e $(LIB_KW_XPM_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_XPM_NAME) $(LIB_KW_XPM_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_XPM_NAME);

$(LIB_KW_XPM_NAME_G): $(LibKwXpmObjFiles_g)
	@echo " --- Linking libkwxpm library (debug mode)"
	@$(CC) $(LIB_KW_XPM_LDFLAGS_G) -o $(LIB_KW_XPM_NAME_G) $(LibKwXpmObjFiles_g) $(LIB_KW_XPM_LIBS_G);
	@if [ ! -e $(LIB_KW_XPM_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_XPM_NAME_G) $(LIB_KW_XPM_LNNAME_G); \
	fi;

$(LIB_KW_REGFILE_NAME): $(LibKwRegfileObjFiles)
	@echo " --- Linking libkwregfile library"
	@$(CC) $(LIB_KW_REGFILE_LDFLAGS) -o $(LIB_KW_REGFILE_NAME) $(LibKwRegfileObjFiles) $(LIB_KW_REGFILE_LIBS);
	@if [ ! -e $(LIB_KW_REGFILE_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_REGFILE_NAME) $(LIB_KW_REGFILE_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_REGFILE_NAME);

$(LIB_KW_REGFILE_NAME_G): $(LibKwRegfileObjFiles_g)
	@echo " --- Linking libkwregfile library (debug mode)"
	@$(CC) $(LIB_KW_REGFILE_LDFLAGS_G) -o $(LIB_KW_REGFILE_NAME_G) $(LibKwRegfileObjFiles_g) $(LIB_KW_REGFILE_LIBS_G);
	@if [ ! -e $(LIB_KW_REGFILE_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_REGFILE_NAME_G) $(LIB_KW_REGFILE_LNNAME_G); \
	fi;

$(LIB_KW_COLOR_NAME): $(LibKwColorObjFiles)
	@echo " --- Linking libkwcolor library"
	@$(CC) $(LIB_KW_COLOR_LDFLAGS) -o $(LIB_KW_COLOR_NAME) $(LibKwColorObjFiles) $(LIB_KW_COLOR_LIBS);
	@if [ ! -e $(LIB_KW_COLOR_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_COLOR_NAME) $(LIB_KW_COLOR_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_COLOR_NAME);

$(LIB_KW_COLOR_NAME_G): $(LibKwColorObjFiles_g)
	@echo " --- Linking libkwcolor library (debug mode)"
	@$(CC) $(LIB_KW_COLOR_LDFLAGS_G) -o $(LIB_KW_COLOR_NAME_G) $(LibKwColorObjFiles_g) $(LIB_KW_COLOR_LIBS_G);
	@if [ ! -e $(LIB_KW_COLOR_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_COLOR_NAME_G) $(LIB_KW_COLOR_LNNAME_G); \
	fi;

$(LIB_KW_ICONV_NAME): $(LibKwIconvObjFiles)
	@echo " --- Linking libkwiconv library"
	@$(CC) $(LIB_KW_ICONV_LDFLAGS) -o $(LIB_KW_ICONV_NAME) $(LibKwIconvObjFiles) $(LIB_KW_ICONV_LIBS);
	@if [ ! -e $(LIB_KW_ICONV_LNNAME) ]; then \
		$(LN) -s $(LIB_KW_ICONV_NAME) $(LIB_KW_ICONV_LNNAME); \
	fi;
	@$(STRIP) $(LIB_KW_ICONV_NAME);

$(LIB_KW_ICONV_NAME_G): $(LibKwIconvObjFiles_g)
	@echo " --- Linking libkwiconv library (debug mode)"
	@$(CC) $(LIB_KW_ICONV_LDFLAGS_G) -o $(LIB_KW_ICONV_NAME_G) $(LibKwIconvObjFiles_g) $(LIB_KW_ICONV_LIBS_G);
	@if [ ! -e $(LIB_KW_ICONV_LNNAME_G) ]; then \
		$(LN) -s $(LIB_KW_ICONV_NAME_G) $(LIB_KW_ICONV_LNNAME_G); \
	fi;

program:
	@if [ ! -e $(ProgramDir) ]; then \
		$(MKDIR) $(ProgramDir); \
	fi
	@for i in $(PROGRAMDIRS); do \
		cd $$i; \
		$(MAKE) install || exit 1; \
		cd $(KWDIR); \
	done

clean:
	@for i in $(SUBDIRS); do \
		cd $$i; \
		$(MAKE) clean || exit 1; \
		cd $(KWDIR); \
	done
	@if [ "$@" = "clean" ]; then \
		$(RM) -f $(SrvBinDir)/logfile $(SrvBinDir)/kiwin-screenshot.png \
			$(KIWIN) $(KIWIN_DB) $(KIWIN_G) $(KIWIN_DBG) \
			$(LIB_KW_PNG_NAME) $(LIB_KW_PNG_NAME_G) \
			$(LIB_KW_Z_NAME) $(LIB_KW_Z_NAME_G) \
			$(LIB_KW_FREETYPE_NAME) $(LIB_KW_FREETYPE_NAME_G) \
			$(LIB_KW_GSCLI_NAME) $(LIB_KW_GSCLI_NAME_G) \
			$(LIB_KW_PNG_LNNAME) $(LIB_KW_PNG_LNNAME_G) \
			$(LIB_KW_Z_LNNAME) $(LIB_KW_Z_LNNAME_G) \
			$(LIB_KW_FREETYPE_LNNAME) $(LIB_KW_FREETYPE_LNNAME_G) \
			$(LIB_KW_GSCLI_LNNAME) $(LIB_KW_GSCLI_LNNAME_G); \
		$(RM) -rf $(ObjDir) $(LibDir) $(EtcDir) $(ProgramDir) $(SrvBinDir); \
	fi
	@for i in $(PROGRAMDIRS); do \
		cd $$i; \
		$(MAKE) clean || exit 1; \
		cd $(KWDIR); \
	done
