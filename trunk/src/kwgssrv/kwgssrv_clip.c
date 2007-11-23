/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * Copyright (C) <1999>  Greg Haerr <greg@censoft.com>
 * Copyright (C) <1991>  David I. Bell
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

#include <kwcommon/kwcommon.h>
#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ========================
 *     static functions
 * ======================== */

static void
reset_window_clip(KWWnd_t *wp)
{
  /* If there were no surviving clip rectangles,
   * then set the clip cache to prevent all drawing.
   */
  if (wp->clip.region.rects_use == 0) {
    wp->clip.clipminx = MIN_COORD;
    wp->clip.clipminy = MIN_COORD;
    wp->clip.clipmaxx = MAX_COORD;
    wp->clip.clipmaxy = MAX_COORD;
    wp->clip.clipresult = false;
    
    return;
  }
  
  /* There was at least one valid clip rectangle.
   * Default the clip cache to be the first clip rectangle.
   */
  wp->clip.clipminx = wp->clip.region.rects[0].left;
  wp->clip.clipminy = wp->clip.region.rects[0].top;
  wp->clip.clipmaxx = wp->clip.region.rects[0].right;
  wp->clip.clipmaxy = wp->clip.region.rects[0].bottom;
  wp->clip.clipresult = true;
}

/* ========================
 *     export functions
 * ======================== */

void
calc_window_clip(KWWnd_t *wp)
{
  KWWnd_t     *orgwp = NULL;
  KWWnd_t     *pwp = NULL;
  KWWnd_t     *sibwp = NULL;
  int          diff;
  int          x, y, width, height;
  KWRegion_t   r;
  KWRect_t     rrect;
  
  if (wp->mapped == false) {
    /* The un-mapped window doesn't need any
     * information about the clipping region.
     * So just return.
     */
    return;
  }
  
  /* Start with the rectangle for the complete window.
   * We will then cut pieces out of it as needed.
   */
  x = wp->x;
  y = wp->y;
  width = wp->width;
  height = wp->height;
  
  /* First walk upwards through all parent windows,
   * and restrict the visible part of this window to the part
   * that shows through all of those parent windows.
   */
  pwp = wp;
  while (pwp != root_window_ptr) {
    pwp = pwp->parent;
    
    diff = pwp->x - x;
    if (diff > 0) {
      width -= diff;
      x = pwp->x;
    }
    
    diff = (pwp->x + pwp->width) - (x + width);
    if (diff < 0) {
      width += diff;
    }
    
    diff = pwp->y - y;
    if (diff > 0) {
      height -= diff;
      y = pwp->y;
    }
    
    diff = (pwp->y + pwp->height) - (y + height);
    if (diff < 0) {
      height += diff;
    }
  }
  
  /* If the window is completely clipped out of view, then
   * set the clipping region to indicate that.
   */
  if ((width <= 0) || (height <= 0)) {
    wp->clip.region.rects_use = 0;
    reset_window_clip(wp);
    return;
  }
  
  /* the following geometry is relative geometry. */
  if (init_region(&(wp->clip.region), x - wp->x, y - wp->y, width - 1, height - 1) == false) {
    LOG(("<Server Error> window %d: initial the first region failed.\n", wp->id));
    return;
  }
    
  r.rects = &rrect;
  r.rects_have = 1;
  r.rects_use = 1;  /* pre-setup this field to avoid many following setups. */
  
  /* Now examine all windows that obscure this window, and
   * for each obscuration, break up the clip rectangles into
   * the smaller pieces that are still visible.  The windows
   * that can obscure us are the earlier siblings of all of
   * our parents.
   */
  orgwp = wp;
  pwp = wp;
  while (pwp != NULL) {
    wp = pwp;
    pwp = wp->parent;
    
    if (pwp == NULL) {
      /* We are clipping the root window now.
       * Start with the root window's children.
       */
      sibwp = root_window_ptr->children;
      
      /* search all root window's children. */
      wp = NULL;
    } else {
      sibwp = pwp->children;
    }
    
    for (; sibwp != wp; sibwp = sibwp->siblings) {
      if (sibwp->mapped == false) {
	continue;
      }
      
      /* the following geometry is relative geometry. */
      r.rects->left   = r.extents.left   = sibwp->x - wp->x;
      r.rects->top    = r.extents.top    = sibwp->y - wp->y;
      r.rects->right  = r.extents.right  = sibwp->x + sibwp->width - wp->x - 1;
      r.rects->bottom = r.extents.bottom = sibwp->y + sibwp->height - wp->y - 1;
      
      subtract_region(&(wp->clip.region), &(wp->clip.region), &r);
    }
    
    /* If not clipping the root window,
     * stop when we reach it.
     */
    if (pwp == root_window_ptr) {
      break;
    }
  }
  
  wp = orgwp;
  /* If not the root window, clip all children.
   * (Root window's children are are clipped above)
   */
  if (wp != root_window_ptr) {
    for (sibwp = wp->children; sibwp != NULL; sibwp = sibwp->siblings) {
      if (sibwp->mapped == false) {
	continue;
      }
      
      r.rects->left   = r.extents.left   = sibwp->x - wp->x;
      r.rects->top    = r.extents.top    = sibwp->y - wp->y;
      r.rects->right  = r.extents.right  = sibwp->x + sibwp->width - wp->x - 1;
      r.rects->bottom = r.extents.bottom = sibwp->y + sibwp->height - wp->y - 1;
      
      subtract_region(&(wp->clip.region), &(wp->clip.region), &r);
    }
  }
  
  reset_window_clip(wp);
}

bool
check_point_in_clip(KWWnd_t *wp, int x, int y)
{
  KWRect_t  *rp = NULL;
  int        count;
  
  if ((x >= wp->clip.clipminx) && (x <= wp->clip.clipmaxx) &&
      (y >= wp->clip.clipminy) && (y <= wp->clip.clipmaxy)) {
    return wp->clip.clipresult;
  }
  
  if (x < 0) {
    wp->clip.clipminx = MIN_COORD;
    wp->clip.clipmaxx = -1;
    wp->clip.clipminy = MIN_COORD;
    wp->clip.clipmaxy = MAX_COORD;
    wp->clip.clipresult = false;
    return false;
  }
  if (y < 0) {
    wp->clip.clipminx = MIN_COORD;
    wp->clip.clipmaxx = MAX_COORD;
    wp->clip.clipminy = MIN_COORD;
    wp->clip.clipmaxy = -1;
    wp->clip.clipresult = false;
    return false;
  }
  if (x >= wp->width) {
    wp->clip.clipminx = wp->width;
    wp->clip.clipmaxx = MAX_COORD;
    wp->clip.clipminy = MIN_COORD;
    wp->clip.clipmaxy = MAX_COORD;
    wp->clip.clipresult = false;
    return false;
  }
  if (y >= wp->height) {
    wp->clip.clipminx = MIN_COORD;
    wp->clip.clipmaxx = MAX_COORD;
    wp->clip.clipminy = wp->height;
    wp->clip.clipmaxy = MAX_COORD;
    wp->clip.clipresult = false;
    return false;
  }
  
  count = wp->clip.region.rects_use;
  if (count <= 0) {
    wp->clip.clipminx = 0;
    wp->clip.clipmaxx = wp->width - 1;
    wp->clip.clipminy = 0;
    wp->clip.clipmaxy = wp->height - 1;
    wp->clip.clipresult = true;
    return true;
  }
  
  for (rp = wp->clip.region.rects; count-- > 0; rp++) {
    if ((x >= rp->left) && (y >= rp->top) &&
	(x <= rp->right) && (y <= rp->bottom)) {
      wp->clip.clipminx = rp->left;
      wp->clip.clipminy = rp->top;
      wp->clip.clipmaxx = rp->right;
      wp->clip.clipmaxy = rp->bottom;
      wp->clip.clipresult = true;
      return true;
    }
  }
  
  wp->clip.clipminx = MIN_COORD;
  wp->clip.clipminy = MIN_COORD;
  wp->clip.clipmaxx = MAX_COORD;
  wp->clip.clipmaxy = MAX_COORD;
  count = wp->clip.region.rects_use;
  
  for (rp = wp->clip.region.rects; count-- > 0; rp++) {
    if ((x < rp->left) && (rp->left <= wp->clip.clipmaxx)) {
      wp->clip.clipmaxx = rp->left - 1;
    }
    
    if ((x > rp->right) && (rp->right >= wp->clip.clipminx)) {
      wp->clip.clipminx = rp->right + 1;
    }
    
    if ((y < rp->top) && (rp->top <= wp->clip.clipmaxy)) {
      wp->clip.clipmaxy = rp->top - 1;
    }
    
    if ((y > rp->bottom) && (rp->bottom >= wp->clip.clipminy)) {
      wp->clip.clipminy = rp->bottom + 1;
    }
  }
  
  wp->clip.clipresult = false;
  return false;
}

int
check_area_in_clip(KWWnd_t *wp, int x1, int y1, int x2, int y2)
{
  if ((x1 < wp->clip.clipminx) || (x1 > wp->clip.clipmaxx) ||
      (y1 < wp->clip.clipminy) || (y1 > wp->clip.clipmaxy)) {
    check_point_in_clip(wp, x1, y1);
  }
  
  if ((x2 >= wp->clip.clipminx) && (x2 <= wp->clip.clipmaxx) &&
      (y2 >= wp->clip.clipminy) && (y2 <= wp->clip.clipmaxy)) {
    if (wp->clip.clipresult == false) {
      return CLIP_INVISIBLE;
    } else {
      return CLIP_VISIBLE;
    }
  }
  
  return CLIP_PARTIAL;
}
