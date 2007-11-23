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

/* ========================
 *     static variables   
 * ======================== */

static struct fb_fix_screeninfo fb_finfo;
static struct fb_var_screeninfo fb_vinfo;

static unsigned char *fb_start_addr;
static int            fb_mmap_size;
static int            fb_pixel_bytes;
static int            fb_fd;

/* ========================
 *     static functions
 * ======================== */

#include "kwgssrv_screenfb32.c"
#include "kwgssrv_screenfb_log_info.c"

static void
fb_draw_line(int x1, int y1, int x2, int y2, int color, bool dotted)
{
  int dx = x2 - x1;
  int dy = y2 - y1;
  int ax = abs(dx) << 1;
  int ay = abs(dy) << 1;
  int sx;
  int sy;
  int x = x1;
  int y = y1;
  
  if (dotted == false) {
    sx = (dx >= 0) ? 1 : -1;
    sy = (dy >= 0) ? 1 : -1;
  } else {
    sx = (dx >= 0) ? 2 : -2;
    sy = (dy >= 0) ? 2 : -2;
  }
  
  if (ax > ay) {
    int d = ay - (ax >> 1);

    if (x1 < x2) {
      while (x < x2) {
	current_screen.draw_pixel(x, y, color);
	
	if ((d > 0) || ((d == 0) && (sx == 1))) {
	  y += sy;
	  d -= ax;
	}
	x += sx;
	d += ay;
      }
    } else {
      while (x > x2) {
	current_screen.draw_pixel(x, y, color);
	
	if ((d > 0) || ((d == 0) && (sx == 1))) {
	  y += sy;
	  d -= ax;
	}
	x += sx;
	d += ay;
      }
    }
    
    if (x == x2) {
      current_screen.draw_pixel(x, y, color);
    }
  } else {
    int d = ax - (ay >> 1);

    if (y1 < y2) {
      while (y < y2) {
	current_screen.draw_pixel(x, y, color);
	
	if ((d > 0) || ((d == 0) && (sy == 1))) {
	  x += sx;
	  d -= ay;
	}
	y += sy;
	d += ax;
      }
    } else {
      while (y > y2) {
	current_screen.draw_pixel(x, y, color);
	
	if ((d > 0) || ((d == 0) && (sy == 1))) {
	  x += sx;
	  d -= ay;
	}
	y += sy;
	d += ax;
      }
    }
    
    if (y == y2) {
      current_screen.draw_pixel(x, y, color);
    }
  }
}

static void
fb_draw_vertical_line(int x, int y1, int y2, int color, bool dotted)
{
  fb_draw_line(x, y1, x, y2, color, dotted);
}

static void
fb_draw_horizontal_line(int y, int x1, int x2, int color, bool dotted)
{
  fb_draw_line(x1, y, x2, y, color, dotted);
}

static void
fb_draw_rect(int x, int y, int width, int height, int color, bool dotted)
{
  if ((width <= 0) || (height <= 0)) {
    return;
  }
  
  fb_draw_line(x, y, x + width, y, color, dotted);
  fb_draw_line(x, y + height, x + width, y + height, color, dotted);
  
  fb_draw_line(x, y, x, y + height, color, dotted);
  fb_draw_line(x + width, y, x + width, y + height, color, dotted);
}

static void
fb_fill_rect(int x, int y, int width, int height, int color, bool dotted)
{
  int maxy = y + height - 1;
  int maxx = x + width - 1;
  
  if (width <= 0 || height <= 0) {
    return;
  }
  
  while (y <= maxy) {
    fb_draw_line(x, y, maxx, y, color, dotted);
    y++;
  }
}

static bool
fb_init(void)
{
  /* try to open /dev/fb first. */
  if ((fb_fd = open("/dev/fb", O_RDWR)) < 0) {
    if (errno == ENOENT) {
      LOG(("%s\n", "<Server Warning> /dev/fb does not exist."));
      LOGC(("%s\n", "<Server Warning> try to use /dev/fb0 instead."));
      
      if ((fb_fd = open("/dev/fb0", O_RDWR)) < 0) {
	LOG(("%s\n", "<Server Error> fatal error occurs when opening /dev/fb0"));
	return false;
      }
    } else {
      LOG(("%s\n", "<Server Error> fatal error occurs when opening /dev/fb"));
      return false;
    }
  }
  
  /* get the frame buffer _fixed_ info */
  ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_finfo);
#ifdef LOG_NONE_ERROR
  fb_log_fix_screeninfo(&fb_finfo);
#endif
  
  /* get the frame buffer _variable_ info */
  ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_vinfo);
#ifdef LOG_NONE_ERROR
  fb_log_var_screeninfo(&fb_vinfo);
#endif
  
  /* var info */
  current_screen.type                 = screen_fb;
  current_screen.width                = fb_vinfo.xres;
  current_screen.height               = fb_vinfo.yres;
  current_screen.draw_line            = fb_draw_line;
  current_screen.draw_vertical_line   = fb_draw_vertical_line;
  current_screen.draw_horizontal_line = fb_draw_horizontal_line;
  current_screen.draw_rect            = fb_draw_rect;
  current_screen.fill_rect            = fb_fill_rect;
  current_screen.draw_pixel           = fb32_draw_pixel;
  current_screen.read_pixel           = fb32_read_pixel;
  
  fb_pixel_bytes = fb_vinfo.bits_per_pixel / 8;
  fb_mmap_size = current_screen.width * current_screen.height * fb_pixel_bytes;
  fb_start_addr = mmap(0, fb_finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
  if (fb_start_addr < 0) {
    LOG(("%s\n", "<Server Error> fatal error occurs when memory mapping for frame buffer device"));
    return false;
  } else {
    memset(fb_start_addr, 0, fb_mmap_size);
    
#ifdef LOG_NONE_ERROR
    LOG(("<Server Info> %dk videoram mapped to %p\n", fb_mmap_size >> 10, fb_start_addr));
#endif
  }
    
  return true;
}

static bool
fb_finalize(void)
{
  if (close(fb_fd) == -1) {
    LOG(("%s\n", "<Server Error> an error occurs when closing the frame buffer device file"));
    return false;
  }
  
  if (munmap(fb_start_addr, fb_mmap_size) < 0) {
    LOG(("%s\n", "<Server Error> an error occurs when munmap the frame buffer device memory."));
    return false;
  }
  
  return true;
}
