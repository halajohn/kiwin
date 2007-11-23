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

#ifndef _KWCOLOR_H_
#define _KWCOLOR_H_

/* ==============
 *     macros
 * ============== */

/* the kwcolor pixel format is 0xAARRGGBB */
#define CREATE_KWCOLOR(a, r, g, b) \
        ((unsigned int)((((a) & 0xff) << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff)))

#define KWCOLOR_ALPHA(color)           ((unsigned char)(((color) & 0xff000000) >> 24))
#define KWCOLOR_RED(color)             ((unsigned char)(((color) & 0xff0000)   >> 16))
#define KWCOLOR_GREEN(color)           ((unsigned char)(((color) & 0xff00)     >> 8))
#define KWCOLOR_BLUE(color)            ((unsigned char)((color) & 0xff))

#define BLACK       CREATE_KWCOLOR(0,   0,   0,   0)
#define BLUE        CREATE_KWCOLOR(0,   0,   0, 128)
#define GREEN       CREATE_KWCOLOR(0,   0, 128,   0)
#define CYAN        CREATE_KWCOLOR(0,   0, 128, 128)
#define RED         CREATE_KWCOLOR(0, 128,   0,   0)
#define MAGENTA     CREATE_KWCOLOR(0, 128,   0, 128)
#define BROWN       CREATE_KWCOLOR(0, 128,  64,   0)
#define LTGRAY      CREATE_KWCOLOR(0, 192, 192, 192)
#define GRAY        CREATE_KWCOLOR(0, 128, 128, 128)
#define LTBLUE      CREATE_KWCOLOR(0,   0,   0, 255)
#define LTGREEN     CREATE_KWCOLOR(0,   0, 255,   0)
#define LTCYAN      CREATE_KWCOLOR(0,   0, 255, 255)
#define LTRED       CREATE_KWCOLOR(0, 255,   0,   0)
#define LTMAGENTA   CREATE_KWCOLOR(0, 255,   0, 255)
#define YELLOW      CREATE_KWCOLOR(0, 255, 255,   0)
#define WHITE       CREATE_KWCOLOR(0, 255, 255, 255)

/* The following macros are now for x86 platform only. */

/* The screen pixel arrangement of x86 framebuffer is (take 32-bit pixel format, for example)
 *
 *    BBBB BBBB GGGG GGGG RRRR RRRR AAAA AAAA ... BBBB BBBB GGGG GGGG RRRR RRRR AAAA AAAA ...
 *
 * However, x86 is a little-endian (little end first) platform,
 * so if there is a int a = 0x80 00 00 00,
 * 
 * then *((char *)(&a)) = 0x00
 *      *((char *)(&a) + 2) = 0x00
 *      *((char *)(&a) + 3) = 0x00
 *      *((char *)(&a) + 4) = 0x80
 * 
 * thus, when we want to memcpy this int a into the framebuffer mmap memory,
 * if we do NOT want to do a transform between 0x80 00 00 00 to 0x00 00 00 80
 * then we better assign int a = 0x00 00 00 80.
 *
 * i.e. on the x86 platform, if we want to use memcpy() or the like to insert
 * the color pixel into framebuffer memory,
 * we better define the pixel format as 0xAA RR GG BB.
 */
/* create a x86 platform framebuffer pixel from independent A, R, G, B color value. */
#define CREATE_PIXEL0888(a, r, g, b)  \
	((((a) & 0xff) << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))
#define CREATE_PIXEL888(a, r, g, b)   \
	((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))
#define CREATE_PIXEL565(a, r, g, b)   \
	((((r) & 0xf8) << 11) | (((g) & 0xfc) << 5) | ((b) & 0xf8))
#define CREATE_PIXEL555(a, r, g, b)   \
	((((r) & 0xf8) << 10) | (((g) & 0xf8) << 5) | ((b)& 0xf8))
#define CREATE_PIXEL332(a, r, g, b)   \
	(((r) & 0xe0) | (((g) & 0xe0) >> 3) | (((g)& 0xc0) >> 6))

/* create a x86 platform framebuffer pixel from kwcolor.
 * the kwcolor pixel format is 0xAARRGGBB.
 */
#define KWCOLOR_TO_PIXEL0888(c)	(c)
#define KWCOLOR_TO_PIXEL888(c)	((c) & 0xffffff)
#define KWCOLOR_TO_PIXEL565(c)	\
	((((c) & 0xf8) << 11) | (((c) & 0xfc00) << 5) | ((c) & 0xf80000))
#define KWCOLOR_TO_PIXEL555(c)	\
	((((c) & 0xf8) << 10) | (((c) & 0xf800) << 5) | ((c)& 0xf80000))
#define KWCOLOR_TO_PIXEL332(c)	\
	(((c) & 0xe0) | (((c) & 0xe000) >> 3) | (((c)& 0xc00000) >> 6))

/* ==========================
 *     struct definations
 * ========================== */

typedef unsigned int  KWColor_t;  /* KWColor's format is AARRGGBB */

typedef enum _KWColorModifyWay_t {
  nop      = 0,
  add      = 1,
  minus    = 2,
  multiply = 3,
  divide   = 4,
  sign     = 5
} KWColorModifyWay_t;

typedef struct _KWColorModify_t {
  enum _KWColorModifyWay_t  way;
  int                      value;
} KWColorModify_t;

typedef enum _KWPixelType_t {
  PALETTE        = 0,   /* pixel is packed  8 bits 1, 4 or 8 pal index */
  TRUECOLOR_0888 = 1,   /* pixel is packed 32 bits 8/8/8 truecolor     */
  TRUECOLOR_888  = 2,   /* pixel is packed 24 bits 8/8/8 truecolor     */
  TRUECOLOR_565  = 3,   /* pixel is packed 16 bits 5/6/5 truecolor     */
  TRUECOLOR_555  = 4,   /* pixel is packed 16 bits 5/5/5 truecolor     */
  TRUECOLOR_332  = 5    /* pixel is packed  8 bits 3/3/2 truecolor     */
} KWPixelType_t;

/* ========================
 *     export functions
 * ======================== */

extern unsigned int  find_color(KWPixelType_t pixel_type, KWColor_t kwcolor);
extern void          insert_color_modify(KWColorModify_t *dest, KWColorModifyWay_t way, int value);
extern KWColor_t     modify_color_light(KWColor_t color, KWColorModify_t *modify);

#endif
