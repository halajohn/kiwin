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

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <kwgssrv/kwgssrv.h>
#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export functions
 * ======================== */

void
process_kbd_event(void)
{
  KWKbdEventInternal_t  event;
  char *buf = (char *)(&event);
  int  c = sizeof(KWKbdEventInternal_t);
  int  n = 0;
  int  e;
  
  while (n < c) {
    e = read(kbd_socket_fd_receive, (buf + n), (c - n));
    
    if (e <= 0) {
      if (e == 0) {
	/* read EOF */
	LOG(("<Server Error> keyboard driver has closed the socket %d\n", kbd_socket_fd_receive));
      } else {
	LOG(("<Server Error> read keyboard data failed, return value = %d, already read %d bytes\n", e, n));
      }
      return;
    }
    n += e;
  }
  
  if (event.type == KEY_DOWN) {
    switch (event.ch) {
    case KWKEY_QUIT:
      break;
      
    case KWKEY_REDRAW:
      break;
      
    case KWKEY_IME:
      break;
      
    case KWKEY_PRINT:
      break;
      
    case KWKEY_LCTRL:
    case KWKEY_RCTRL:
    case KWKEY_LSHIFT:
    case KWKEY_RSHIFT:
    case KWKEY_LALT:
    case KWKEY_RALT:
      break;
      
    default:
      break;
    }
  }
}
