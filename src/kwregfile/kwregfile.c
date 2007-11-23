/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <kwcommon/kwcommon.h>
#include <gdbm/gdbm.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export variables
 * ======================== */

gdbm_file_info *reg_file = NULL;

/* ========================
 *     export functions
 * ======================== */

bool
regfile_init(void)
{
  reg_file = gdbm_open(KWDIR"/etc/regfile.reg", 0, GDBM_READER, 00664, NULL);
  if (reg_file == NULL) {
    LOG(("%s\n", "gdbm_open() failed."));
    return false;
  }
  
  return true;
}

bool
regfile_finalize(void)
{
  gdbm_close(reg_file);
  
  return true;
}
