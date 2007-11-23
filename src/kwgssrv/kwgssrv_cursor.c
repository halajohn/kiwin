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

#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>
#include <kwxpm/kwxpm.h>
#include <kwcolor/kwcolor.h>

/* ======================== 
 *     static variables
 * ======================== */

static unsigned int *mouse_pointer_saved_data = NULL;

/* ======================== 
 *     export variables
 * ======================== */

int        previous_x, previous_y;
int        current_x, current_y;

KWCursor_t  *current_cursor_ptr = NULL;
KWCursor_t  *normal_cursor_ptr  = NULL;
KWCursor_t  *resize_cursor1_ptr = NULL;
KWCursor_t  *resize_cursor2_ptr = NULL;
KWCursor_t  *resize_cursor3_ptr = NULL;
KWCursor_t  *resize_cursor4_ptr = NULL;

/* ======================== 
 *     export functions
 * ======================== */

void
restore_saved_block(int x, int y)
{
  unsigned int *ptr = NULL;
  int i, j;
  
  /* draw the original block at the previous cursor position */
  ptr = mouse_pointer_saved_data;
  x = x - current_cursor_ptr->x_hotspot;
  y = y - current_cursor_ptr->y_hotspot;
  
  for (i = 0; i < current_cursor_ptr->height; i++) {
    for (j = 0; j < current_cursor_ptr->width; j++) {
      if ((*((unsigned int *)(current_cursor_ptr->data) + (i * current_cursor_ptr->width) + j)) != 0) {
	current_screen.draw_pixel(x + j, y + i, *ptr);
	ptr++;
      }
    }
  }
}

void
save_current_block(int x, int y)
{
  unsigned int *ptr = NULL;
  int i, j;
  int savecolor;
  
  /* save the block of the new cursor position */
  ptr = mouse_pointer_saved_data;
  x = x - current_cursor_ptr->x_hotspot;
  y = y - current_cursor_ptr->y_hotspot;
  
  for (i = 0; i < current_cursor_ptr->height; i++) {
    for (j = 0; j < current_cursor_ptr->width; j++) {
      if ((*((unsigned int *)(current_cursor_ptr->data) + (i * current_cursor_ptr->width) + j)) != 0) {
	savecolor = current_screen.read_pixel(x + j, y + i);
	*ptr = savecolor;
	ptr++;
      }
    }
  }
}

void
draw_mouse_pointer(int x, int y)
{
  int i, j; /* Looping variable */
  unsigned int color;
  int nx, ny;
  
  nx = x - current_cursor_ptr->x_hotspot;
  ny = y - current_cursor_ptr->y_hotspot;
  
  /* draw the cursor now! */
  for (i = 0; i < current_cursor_ptr->height; i++) {
    for (j = 0; j < current_cursor_ptr->width; j++) {
      color = *((unsigned int *)(current_cursor_ptr->data) + (i * current_cursor_ptr->width) + j);
      if (color != 0) {
	color = find_color(TRUECOLOR_0888, color);
	current_screen.draw_pixel(nx + j, ny + i, color);
      } else {
	continue;
      }
    }
  }
  
  previous_x = x;
  previous_y = y;
}

bool
cursor_init(void)
{
  bool result = false;
  
  /* ======== Normal ======== */
  normal_cursor_ptr = (KWCursor_t *)malloc(sizeof(KWCursor_t));
  if (normal_cursor_ptr == NULL) {
    LOG(("%s\n", "<Server Error> malloc() failed."));
    goto failed;
  }
  
  result = get_xpm_data(KIWIN_CURSOR_DIR "arrow_m.xpm",
			&(normal_cursor_ptr->width), &(normal_cursor_ptr->height),
			&(normal_cursor_ptr->x_hotspot), &(normal_cursor_ptr->y_hotspot),
			&(normal_cursor_ptr->data));
  if (result == false) {
    LOG(("%s\n", "<Server Error> can not load the normal cursor image."));
    goto failed;
  }
  
  /* ======== Resize 1 ======== */
  resize_cursor1_ptr = (KWCursor_t *)malloc(sizeof(KWCursor_t));
  if (resize_cursor1_ptr == NULL) {
    LOG(("%s\n", "<Server Error> malloc() failed."));
    goto failed;
  }
  
  result = get_xpm_data(KIWIN_CURSOR_DIR "size1_m.xpm",
			&(resize_cursor1_ptr->width), &(resize_cursor1_ptr->height),
			&(resize_cursor1_ptr->x_hotspot), &(resize_cursor1_ptr->y_hotspot),
			&(resize_cursor1_ptr->data));
  if (result == false) {
    LOG(("%s\n", "<Server Error> can not load the resize1 cursor image."));
    goto failed;
  }
  
  /* ======== Resize 2 ======== */
  resize_cursor2_ptr = (KWCursor_t *)malloc(sizeof(KWCursor_t));
  if (resize_cursor2_ptr == NULL) {
    LOG(("%s\n", "<Server Error> malloc() failed."));
    goto failed;
  }
  
  result = get_xpm_data(KIWIN_CURSOR_DIR "size2_m.xpm",
			&(resize_cursor2_ptr->width), &(resize_cursor2_ptr->height),
			&(resize_cursor2_ptr->x_hotspot), &(resize_cursor2_ptr->y_hotspot),
			&(resize_cursor2_ptr->data));
  if (result == false) {
    LOG(("%s\n", "<Server Error> can not load the resize2 cursor image."));
    goto failed;
  }
  
  /* ======== Resize 3 ======== */
  resize_cursor3_ptr = (KWCursor_t *)malloc(sizeof(KWCursor_t));
  if (resize_cursor3_ptr == NULL) {
    LOG(("%s\n", "<Server Error> malloc() failed."));
    goto failed;
  }
  
  result = get_xpm_data(KIWIN_CURSOR_DIR "size3_m.xpm",
			&(resize_cursor3_ptr->width), &(resize_cursor3_ptr->height),
			&(resize_cursor3_ptr->x_hotspot), &(resize_cursor3_ptr->y_hotspot),
			&(resize_cursor3_ptr->data));
  if (result == false) {
    LOG(("%s\n", "<Server Error> can not load the resize3 cursor image."));
    goto failed;
  }
  
  /* ======== Resize 4 ======== */
  resize_cursor4_ptr = (KWCursor_t *)malloc(sizeof(KWCursor_t));
  if (resize_cursor4_ptr == NULL) {
    LOG(("%s\n", "<Server Error> malloc() failed."));
    goto failed;
  }
  
  result = get_xpm_data(KIWIN_CURSOR_DIR "size4_m.xpm",
			&(resize_cursor4_ptr->width), &(resize_cursor4_ptr->height),
			&(resize_cursor4_ptr->x_hotspot), &(resize_cursor4_ptr->y_hotspot),
			&(resize_cursor4_ptr->data));
  if (result == false) {
    LOG(("%s\n", "<Server Error> can not load the resize4 cursor image."));
    goto failed;
  }
  
  current_cursor_ptr = normal_cursor_ptr;
  
  mouse_pointer_saved_data =
    (unsigned int *)malloc(sizeof(unsigned int) * normal_cursor_ptr->width * normal_cursor_ptr->height);
  if (mouse_pointer_saved_data == NULL) {
    goto failed;
  }
  
  current_x = current_screen.width / 2;
  current_y = current_screen.height / 2;
  previous_x = current_screen.width / 2;
  previous_y = current_screen.height / 2;
  
  save_current_block(current_x, current_y);
  draw_mouse_pointer(current_x, current_y);
  
  return true;
  
 failed:
  if (normal_cursor_ptr != NULL) {
    free(normal_cursor_ptr);
  }
  if (resize_cursor1_ptr != NULL) {
    free(resize_cursor1_ptr);
  }
  if (resize_cursor2_ptr != NULL) {
    free(resize_cursor2_ptr);
  }
  if (resize_cursor3_ptr != NULL) {
    free(resize_cursor3_ptr);
  }
  if (resize_cursor4_ptr != NULL) {
    free(resize_cursor4_ptr);
  }
  return false;
}

bool
cursor_finalize(void)
{
  if (normal_cursor_ptr != NULL) {
    free(normal_cursor_ptr);
  }
  if (resize_cursor1_ptr != NULL) {
    free(resize_cursor1_ptr);
  }
  if (resize_cursor2_ptr != NULL) {
    free(resize_cursor2_ptr);
  }
  if (resize_cursor3_ptr != NULL) {
    free(resize_cursor3_ptr);
  }
  if (resize_cursor4_ptr != NULL) {
    free(resize_cursor4_ptr);
  }
  if (mouse_pointer_saved_data != NULL) {
    free(mouse_pointer_saved_data);
  }
  
  return true;
}
