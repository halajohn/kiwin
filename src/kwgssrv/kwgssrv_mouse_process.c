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

#include <kwlog/kwlog.h>
#include <kwgssrv/kwgssrv.h>

/* ========================
 *     export functions
 * ======================== */

void
process_mouse_event(void)
{
  KWMouseEventInternal_t event;
  char *buf = (char *)(&event);
  int c = sizeof(KWMouseEventInternal_t);
  int n = 0;
  int e;
  
  while (n < c) {
    e = read(mouse_socket_fd_receive, (buf + n), (c - n));
    
    if (e <= 0) {
      if (e == 0) {
	/* read EOF */
	LOG(("<Server Error> mouse driver has closed the socket %d\n", mouse_socket_fd_receive));
      } else {
	LOG(("<Server Error> read mouse data failed, return value = %d, already read %d bytes\n", e, n));
      }
      return;
    }
    n += e;
  }
  
  current_x = event.x;
  current_y = event.y;
  
  switch (event.type) {
  case MOUSE_MOVE:
    restore_saved_block(previous_x, previous_y);
    save_current_block(current_x, current_y);
    draw_mouse_pointer(current_x, current_y);
    break;
    
  case LEFT_BUTTON_DOWN:
    break;
    
  case LEFT_BUTTON_DRAG:
    break;
    
  case LEFT_BUTTON_UP:
    break;
    
  case MIDDLE_BUTTON_DOWN:
    break;
    
  case MIDDLE_BUTTON_DRAG:
    break;
    
  case MIDDLE_BUTTON_UP:
    break;
    
  case RIGHT_BUTTON_DOWN:
    break;
    
  case RIGHT_BUTTON_DRAG:
    break;
    
  case RIGHT_BUTTON_UP:
    break;
  }
}
