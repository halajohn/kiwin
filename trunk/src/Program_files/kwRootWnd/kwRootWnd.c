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

#include <stdlib.h>

#include <kwlog/kwlog.h>
#include <kwgsshr/kwdefs.h>
#include <kwgsshr/kwevent.h>
#include <kwgscli/kwevent.h>
#include <kwgscli/kwwnd.h>
#include <kwgscli/kwdraw.h>
#include <kwgscli/kwgc.h>

#define ROOT_WND_BGCOLOR  BLUE

int
main(int argc, char **argv)
{
  unsigned int  mainWndId;
  unsigned int  gc;
  KWEvent       event;
  
  mainWndId = new_main_window_complex(0, 0, 1024, 768, ROOT_WND_BGCOLOR, NULL, 0);
  if (mainWndId == 0) {
    mainWndId = new_main_window_complex(0, 0, 800, 600, ROOT_WND_BGCOLOR, NULL, 0);
    
    if (mainWndId == 0) {
      mainWndId = new_main_window_complex(0, 0, 640, 480, ROOT_WND_BGCOLOR, NULL, 0);
      
      if (mainWndId == 0) {
	LOG(("%s\n", "<Root Window Error> The geometry of the screen has to be 1024x768 or 800x600 or 640x480."));
	exit (-1);
      }
    }
  }
  if (mainWndId == BAD_WND_ID) {
    LOG(("%s\n", "<Root Window Error> can not create the main window."));
    exit (-1);
  }
  
  gc = new_gc();
  
  select_events(mainWndId, EVENT_MASK_EXPOSURE);
  map_window(mainWndId);
  
  for (;;) {
    get_next_event(&event);
    switch (event.type) {
    case EVENT_TYPE_EXPOSURE:
      if (event.exposure.wid == mainWndId) {
	set_gc_background(gc, ROOT_WND_BGCOLOR);
	fill_rect(mainWndId, gc, event.exposure.x, event.exposure.y,
		  event.exposure.width, event.exposure.height);
      }
      break;
    }
  }
}
