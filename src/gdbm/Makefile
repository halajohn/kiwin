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

include $(MakDir)/Rules.config.lib

all: depend $(LibKwGdbmObjFiles) $(LibKwGdbmObjFiles_g)

clean:
	@$(RM) -f *~ .lib_depend .lib_depend_g \
	$(LibKwGdbmObjFiles) $(LibKwGdbmObjFiles_g)

depend: .lib_depend .lib_depend_g

ifeq (.lib_depend,$(wildcard .lib_depend))
include .lib_depend
endif

ifeq (.lib_depend_g,$(wildcard .lib_depend_g))
include .lib_depend_g
endif

include $(MakDir)/Rules.build.lib
