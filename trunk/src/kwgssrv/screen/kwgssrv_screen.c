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

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/kd.h>

#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export variables
 * ======================== */

KWScreenDriver_t current_screen;

#include "kwgssrv_screenfb.c"

/* ========================
 *     static functions
 * ======================== */

static KWScreenType_t
probe_screen(void)
{
  return screen_fb;
}

/* ========================
 *     export functions
 * ======================== */

bool
screen_init(void)
{
  KWScreenType_t screen_type = probe_screen();
  bool     result;
  int      tty;
  
  /* open tty, enter graphics mode */
  tty = open("/dev/tty0", O_RDWR);
  if (tty < 0) {
    LOG(("%s\n", "<Server Error> can not open /dev/tty0"));
    return false;
  }
  
  if (ioctl(tty, KDSETMODE, KD_GRAPHICS) == -1) {
    LOG(("%s\n", "<Server Error> can not setting console to graphics mode."));
    close(tty);
    return false;
  }
  close(tty);
  
  if (screen_type == screen_fb) {
    result = fb_init();
    if (result == false) {
      LOG(("%s\n", "<Server Error> can not initialize the frame buffer device."));
      close(tty);
      return false;
    }
  }
  
  return true;
}

bool
screen_finalize(void)
{
  bool result;
  int  tty;
  
  if (current_screen.type == screen_fb) {
    result = fb_finalize();
    
    if (result == false) {
      LOG(("%s\n", "<Server Error> can not finalize the frame buffer device."));
      return false;
    }
  }
  
  /* enter text mode */
  tty = open("/dev/tty0", O_RDWR);
  ioctl(tty, KDSETMODE, KD_TEXT);
  close(tty);
  
  return true;
}
