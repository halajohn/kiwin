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
#include <sys/types.h>

#include <kwcommon/kwcommon.h>
#include <kwcolor/kwcolor.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export functions
 * ======================== */

/* Convert a palette-independent value to a hardware color. */
unsigned int
find_color(KWPixelType_t pixel_type, KWColor_t kwcolor)
{
  switch (pixel_type) {
  case TRUECOLOR_0888:
    return KWCOLOR_TO_PIXEL0888(kwcolor);
    
  case TRUECOLOR_888:
    /* create 24 bit 8/8/8 format pixel (bbbb bbbb gggg gggg rrrr rrrr) from KWColor colorval */
    return KWCOLOR_TO_PIXEL888(kwcolor);
    
  case TRUECOLOR_565:
    /* create 16 bit 5/6/5 format pixel (bbbbb gggggg rrrrr) from KWColor colorval */
    return KWCOLOR_TO_PIXEL565(kwcolor);
    
  case TRUECOLOR_555:
    /* create 16 bit 5/5/5 format pixel (x bbbbb ggggg rrrrr) from KWColor colorval */
    return KWCOLOR_TO_PIXEL555(kwcolor);
    
  case TRUECOLOR_332:
    /* create 8 bit 3/3/2 format pixel (bbb ggg rr) from KWColor colorval */
    return KWCOLOR_TO_PIXEL332(kwcolor);
    
  case PALETTE:
    return kwcolor; /* TODO */
  }
  
  return 0; /* avoid compiler's warning. */
}

void
insert_color_modify(KWColorModify_t *modify, KWColorModifyWay_t way, int value)
{
  modify->way   = way;
  modify->value = value;
  
  return;
}

KWColor_t
modify_color_light(KWColor_t color, KWColorModify_t *modify)
{
  int red   = 0;
  int green = 0;
  int blue  = 0;
  int alpha = 0;
  bool jump = false;
  
  if (modify == NULL) {
    LOG(("<Client Warning> client %d: call modify_color_light() with NULL second argument.\n", getpid()));
    return color;
  }
  
  red    = KWCOLOR_RED(color);
  green  = KWCOLOR_GREEN(color);
  blue   = KWCOLOR_BLUE(color);
  alpha  = KWCOLOR_ALPHA(color);
  
  while (jump == false) {
    switch (modify->way) {
    case add:
      blue  = blue  + modify->value;
      green = green + modify->value;
      red   = red   + modify->value;
      break;
      
    case minus:
      blue  = blue  - modify->value;
      green = green - modify->value;
      red   = red   - modify->value;
      break;
      
    case multiply:
      blue  = blue  * modify->value;
      green = green * modify->value;
      red   = red   * modify->value;
      break;
      
    case divide:
      blue  = blue  / modify->value;
      green = green / modify->value;
      red   = red   / modify->value;
      break;
      
    case sign:
      blue  = -blue;
      green = -green;
      red   = -red;
      break;
      
    case nop:
      jump = true;
      break;
    }
    
    modify = ((KWColorModify_t *)modify) + 1;
  }
  
  if (blue  > 255)  blue  = 255;
  if (green > 255)  green = 255;
  if (red   > 255)  red   = 255;
  if (blue  <   0)  blue  = 0;
  if (green <   0)  green = 0;
  if (red   <   0)  red   = 0;
  
  return CREATE_KWCOLOR(alpha, red, green, blue);
}
