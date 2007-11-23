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

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include <kwgsshr/kwevent.h>
#include <kwgscli/kwwnd.h>
#include <kwgscli/kwgc.h>
#include <kwgscli/kwevent.h>
#include <kwgscli/kwdraw.h>
#include <kwlog/kwlog.h>

#define WIDTH	320
#define HEIGHT	240

int
main(int argc, char **argv)
{
  unsigned int  mainWndId;
  unsigned int  gc;
  KWEvent       event;
  
  mainWndId = new_main_window_simple(50, 50, WIDTH, HEIGHT, "Window 1, This is the first one.");
  if (mainWndId == BAD_WND_ID) {
    LOG(("%s\n", "<Demo2 Error> can not create the main window."));
    exit (-1);
  }
  
  gc = new_gc();
  
  select_events(mainWndId, EVENT_MASK_EXPOSURE);
  map_window(mainWndId);
  
  for (;;) {
    get_next_event(&event);
    switch (event.type) {
    }
  }
}
