/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * Copyright (C) <1999, 2000, 2001>  Greg Haerr <greg@censoft.com>
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

#include <kwgssrv/kwgssrv.h>

/* ========================
 *     static functions
 * ======================== */

static void
drawrow(KWWnd_t *wp, int x1, int x2, int y, unsigned int color, bool dotted)
{
  int temp;
  
  /* reverse endpoints if necessary */
  if (x1 > x2) {
    temp = x1;
    x1 = x2;
    x2 = temp;
  }
  
  /* clip to physical device */
  if (x1 < 0) {
    x1 = 0;
  }
  if (x2 >= current_screen.width) {
    x2 = current_screen.width - 1;
  }
  
  while (x1 <= x2) {
    if (check_point_in_clip(wp, x1, y) == true) {
      temp = MIN(wp->clip.clipmaxx, x2);
      current_screen.draw_horizontal_line(wp->y + y, wp->x + x1, wp->x + temp, color, dotted);
    } else {
      temp = MIN(wp->clip.clipmaxx, x2);
    }
    
    x1 = temp + 1;
  }
}

static void
drawcol(KWWnd_t *wp, int x, int y1, int y2, unsigned int color, bool dotted)
{
  int temp;
  
  /* reverse endpoints if necessary */
  if (y1 > y2) {
    temp = y1;
    y1 = y2;
    y2 = temp;
  }
  
  /* clip to physical device */
  if (y1 < 0) {
    y1 = 0;
  }
  if (y2 >= current_screen.height) {
    y2 = current_screen.height - 1;
  }
  
  while (y1 <= y2) {
    if (check_point_in_clip(wp, x, y1)) {
      temp = MIN(wp->clip.clipmaxy, y2);
      current_screen.draw_vertical_line(wp->x + x, wp->y + y1, wp->y + temp, color, dotted);
    } else {
      temp = MIN(wp->clip.clipmaxy, y2);
    }
    
    y1 = temp + 1;
  }
}

/* ========================
 *     export functions
 * ======================== */

void
draw_point(KWWnd_t *wp, int x, int y, unsigned int color)
{
  if (check_point_in_clip(wp, x, y) == true) {
    current_screen.draw_pixel(wp->x + x, wp->y + y, color);
  }
}

void
draw_line(KWWnd_t *wp, int x1, int y1, int x2, int y2, unsigned int color, bool dotted)
{
  int xdelta;   /* width of rectangle around line    */
  int ydelta;   /* height of rectangle around line   */
  int xinc;     /* increment for moving x coordinate */
  int yinc;     /* increment for moving y coordinate */
  int rem;      /* current remainder                 */
  
  /* See if the line is horizontal or vertical. If so, then call
   * special routines.
   */
  if (y1 == y2) {
    /* call faster line drawing routine */
    drawrow(wp, x1, x2, y1, color, dotted);
    return;
  }
  
  if (x1 == x2) {
    /* call faster line drawing routine */
    drawcol(wp, x1, y1, y2, color, dotted);
    return;
  }
  
  /* See if the line is either totally visible or totally invisible.
   * If so, then the line drawing is easy.
   */
  switch (check_area_in_clip(wp, x1, y1, x2, y2)) {
  case CLIP_VISIBLE:
    current_screen.draw_line(wp->x + x1, wp->y + y1, wp->x + x2, wp->y + y2, color, dotted);
    return;
    
  case CLIP_INVISIBLE:
    return;
  }
  
  /* The line may be partially obscured. Do the draw line algorithm
   * checking each point against the clipping regions.
   */
  xdelta = x2 - x1;
  ydelta = y2 - y1;
  
  if (xdelta < 0) {
    xdelta = -xdelta;
  }
  if (ydelta < 0) {
    ydelta = -ydelta;
  }
  
  xinc = (x2 > x1) ? 1 : -1;
  yinc = (y2 > y1) ? 1 : -1;
  
  if (check_point_in_clip(wp, x1, y1) == true) {
    current_screen.draw_pixel(wp->x + x1, wp->y + y1, color);
  }
  
  if (xdelta >= ydelta) {
    rem = xdelta / 2;
    
    for (;;) {
      x1 += xinc;
      rem += ydelta;
      
      if (rem >= xdelta) {
	rem -= xdelta;
	y1 += yinc;
      }
      
      if (check_point_in_clip(wp, x1, y1) == true) {
	current_screen.draw_pixel(wp->x + x1, wp->y + y1, color);
      }
      
      if (x1 == x2) {
	break;
      }
    }
  } else {
    rem = ydelta / 2;
    
    for (;;) {
      y1 += yinc;
      rem += xdelta;
      
      if (rem >= ydelta) {
	rem -= ydelta;
	x1 += xinc;
      }
      
      if (check_point_in_clip(wp, x1, y1) == true) {
	current_screen.draw_pixel(wp->x + x1, wp->y + y1, color);
      }
      
      if (y1 == y2) {
	break;
      }
    }
  }
}

void
draw_rect(KWWnd_t *wp, int x, int y, int width, int height, unsigned int color, bool dotted)
{
  int maxx, maxy;
  
  if (width <= 0 || height <= 0) {
    return;
  }
  
  maxx = x + width - 1;
  maxy = y + height - 1;
  
  drawrow(wp, x, maxx, y, color, dotted);
  if (height > 1) {
    drawrow(wp, x, maxx, maxy, color, dotted);
  }
  if (height < 3) {
    return;
  }
  
  y++;
  maxy--;
  drawcol(wp, x, y, maxy, color, dotted);
  if (width > 1) {
    drawcol(wp, maxx, y, maxy, color, dotted);
  }
}

void
fill_rect(KWWnd_t *wp, int x, int y, int width, int height, unsigned int color, bool dotted)
{
  int maxx, maxy;
  
  if (width <= 0 || height <= 0) {
    return;
  }
  
  maxx = x + width - 1;
  maxy = y + height - 1;
  
  /* See if the rectangle is either totally visible or totally
   * invisible. If so, then the rectangle drawing is easy.
   */
  switch (check_area_in_clip(wp, x, y, maxx, maxy)) {
  case CLIP_VISIBLE:
    current_screen.fill_rect(wp->x + x, wp->y + y, width, height, color, dotted);
    return;
    
  case CLIP_INVISIBLE:
    return;
  }
  
  /* The rectangle may be partially obstructed. So do it line by line. */
  while (y <= maxy) {
    drawrow(wp, x, maxx, y++, color, dotted);
  }
}
