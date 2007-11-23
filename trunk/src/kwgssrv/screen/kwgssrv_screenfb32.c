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
 *     static functions
 * ======================== */

static void
fb32_draw_pixel(int x, int y, int color)
{
  if ( (x < 0) || (x >= current_screen.width) || (y < 0) || (y >= current_screen.height) ) {
    return;
  } else {
    int addr = (current_screen.width * y + x) * fb_pixel_bytes;
    
    memcpy(&(fb_start_addr[addr]), (unsigned char *)(&color), 4);
  }
}

static int
fb32_read_pixel(int x, int y)
{
  unsigned int color = 0;
  
  if ( (x < 0) || (x >= current_screen.width) || (y < 0) || (y >= current_screen.height) ) {
    return 0;
  } else {
    int addr = (current_screen.width * y + x) * fb_pixel_bytes;
    
    memcpy((unsigned char *)(&color), &(fb_start_addr[addr]), 4);
    
    return color;
  }
}
