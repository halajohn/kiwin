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

#ifdef LOG_NONE_ERROR
static void
fb_log_fix_screeninfo(struct fb_fix_screeninfo *fb_finfo)
{
  LOG(("%s\n", "<Server Info> fb_fix_screeninfo..."));
  LOGC(("           <1> identification string = %s\n", fb_finfo->id));
  LOGC(("           <2> start of frame buffer mem (physical address) = %p\n", (void *)fb_finfo->smem_start));
  LOGC(("           <3> length of frame buffer memory = %d\n", fb_finfo->smem_len));
  LOGC(("           <4> frame buffer type = %d\n", fb_finfo->type));
  LOGC(("           <5> interleave for interleaved planes = %d\n", fb_finfo->type_aux));
  LOGC(("           <6> frame buffer visual = %d\n", fb_finfo->visual));
  LOGC(("           <7> hardware panning for x axis = %d ", fb_finfo->xpanstep));
  LOGC(("%s\n", " (0 if no hardware panning)"));
  LOGC(("           <8> hardware panning for y axis = %d ", fb_finfo->ypanstep));
  LOGC(("%s\n", " (0 if no hardware panning)"));
  LOGC(("           <9> hardware ywrap = %d ", fb_finfo->ywrapstep));
  LOGC(("%s\n", " (0 if no hardware ywrap)"));
  LOGC(("          <10> length of a line in bytes = %d\n", fb_finfo->line_length));
  LOGC(("          <11> start of memory mapped I/O (physical address) = %p\n", (void *)fb_finfo->mmio_start));
  LOGC(("          <12> length of memory mapped I/O = %d\n", fb_finfo->mmio_len));
  LOGC(("          <13> type of acceleration available = %d\n", fb_finfo->accel));
}

static void
fb_log_var_screeninfo(struct fb_var_screeninfo *fb_vinfo)
{
  LOG(("%s\n", "<Server Info> fb_var_screeninfo..."));
  LOGC(("           <1> visible resolution for x axis = %d\n", fb_vinfo->xres));
  LOGC(("           <2> visible resolution for y axis = %d\n", fb_vinfo->yres));
  LOGC(("           <3> virtual resolution for x axis = %d\n", fb_vinfo->xres_virtual));
  LOGC(("           <4> virtual resolution for y axis = %d\n", fb_vinfo->yres_virtual));
  LOGC(("           <5> offset from virtual to visible resolution for x axis = %d\n", fb_vinfo->xoffset));
  LOGC(("           <6> offset from virtual to visible resolution for y axis = %d\n", fb_vinfo->yoffset));
  LOGC(("           <7> bits per pixel = %d\n", fb_vinfo->bits_per_pixel));
  LOGC(("           <8> grayscale = %d ", fb_vinfo->grayscale));
  LOGC(("%s\n", " (!= 0 means graylevels instead of colors)"));
  LOGC(("%s\n", "             *** bitfield in frame buffer memory if true color"));
  LOGC(("%s\n", "             *** else only length is significant."));
  LOGC(("           <9> red    beginning of bitfield = %d\n", fb_vinfo->red.offset));
  LOGC(("                      length of bitfield= %d\n", fb_vinfo->red.length));
  LOGC(("                      most significant bit is right = %d ", fb_vinfo->red.msb_right));
  LOGC(("%s\n", " (!= 0 : most significant bit is right)"));
  LOGC(("          <10> green  beginning of bitfield = %d\n", fb_vinfo->green.offset));
  LOGC(("                      length of bitfield= %d\n", fb_vinfo->green.length));
  LOGC(("                      most significant bit is right = %d ", fb_vinfo->green.msb_right));
  LOGC(("%s\n", " (!= 0 : most significant bit is right)"));
  LOGC(("          <11> blue   beginning of bitfield = %d\n", fb_vinfo->blue.offset));
  LOGC(("                      length of bitfield= %d\n", fb_vinfo->blue.length));
  LOGC(("                      most significant bit is right = %d ", fb_vinfo->blue.msb_right));
  LOGC(("%s\n", " (!= 0 : most significant bit is right)"));
  LOGC(("          <12> transparency   beginning of bitfield = %d\n", fb_vinfo->transp.offset));
  LOGC(("                              length of bitfield= %d\n", fb_vinfo->transp.length));
  LOGC(("                              most significant bit is right = %d ", fb_vinfo->transp.msb_right));
  LOGC(("%s\n", " (!= 0 : most significant bit is right)"));
  LOGC(("          <13> standard pixel format = %d ", fb_vinfo->nonstd));
  LOGC(("%s\n", " (!= 0 means _Non_ standard pixel format)"));
  LOGC(("          <14> frame buffer activate = %d\n", fb_vinfo->activate));
  LOGC(("          <15> height of picture in mm = %d\n", fb_vinfo->height));
  LOGC(("          <16> width of picture in mm = %d\n", fb_vinfo->width));
  LOGC(("          <17> acceleration flags = %d\n", fb_vinfo->accel_flags));
  LOGC(("%s\n", "             *** Timing:"));
  LOGC(("          <18> pixel clock in pico seconds = %d\n", fb_vinfo->pixclock));
  LOGC(("          <19> time from sync to picture (left_margin) = %d\n", fb_vinfo->left_margin));
  LOGC(("          <20> time from picture to sync (right_margin) = %d\n", fb_vinfo->right_margin));
  LOGC(("          <21> time from sync to picture (upper_margin) = %d\n", fb_vinfo->upper_margin));
  LOGC(("          <22> time from picture to sync (lower_margin) = %d\n", fb_vinfo->lower_margin));
  LOGC(("          <23> length of horizontal sync = %d\n", fb_vinfo->hsync_len));
  LOGC(("          <24> length of vertical sync = %d\n", fb_vinfo->vsync_len));
  LOGC(("          <25> frame buffer sync = %d\n", fb_vinfo->sync));
  LOGC(("          <26> frame buffer vmode = %d\n", fb_vinfo->vmode));
}
#endif
