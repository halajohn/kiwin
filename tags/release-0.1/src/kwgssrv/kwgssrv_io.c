/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * Copyright (C) <1999-2001>  Greg Haerr <greg@censoft.com>
 * Copyright (C) <1999>  Alex Holden <alex@linuxhacker.org>
 * Copyright (C) <2000>  Vidar Hokstad
 * Copyright (C) <2000>  Morten Rolland <mortenro@screenmedia.no>
 * Portions Copyright (C) <1991>  David I. Bell
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

#include <unistd.h>
#include <errno.h>

#include <kwlog/kwlog.h>

/* ========================
 *     export functions
 * ======================== */

/* This is a wrapper to write() */
bool
write_data_to_cli(int fd, void *buf, int c)
{
  int e;
  int n = 0;
  
  while (n < c) {
    e = write(fd, ((char *)buf + n), (c - n));
    if (e <= 0) {
      LOG(("%s\n", "<Server Error> can not send data to the client."));
      return false;
    }
    n += e;
  }
  
  return true;
}

bool
write_typed_data_to_cli(int fd, short type)
{
  return write_data_to_cli(fd, &type, sizeof(type));
}

bool
read_data_from_cli(int fd, void *buf, int c)
{
  int e;
  int n = 0;
  
  while (n < c) {
    e = read(fd, (buf + n), (c - n));
    
    if (e <= 0) {
      if (e == 0) {
	/* read EOF */
	LOG(("<Server Error> client closed socket: fd = %d\n", fd));
      } else {
	LOG(("<Server Error> read from client window failed, return value = %d, already read %d bytes\n", e, n));
      }
      
      return false;
    }
    n += e;
  }
  
  return true;
}
