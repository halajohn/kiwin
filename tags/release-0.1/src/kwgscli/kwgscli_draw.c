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

#include <unistd.h>     /* getpid() */
#include <sys/types.h>  /* getpid() */

#include <kwgsshr/kwgsshr.h>
#include <kwgscli/kwgscli.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export functions
 * ======================== */

/* ==================
 *     draw point
 * ================== */

void
drawWndPoint(KWWnd_t *wp, KWColor_t color, int x, int y)
{
  KWDrawPointReq_t *req = NULL;
  
  req = AllocReq(wp, DrawPoint);
  
  req->color = find_color(TRUECOLOR_0888, color);
  req->x = x;
  req->y = y;
}

void
KDrawPoint(unsigned int wid, unsigned int gcid, int x, int y)
{
  KWGc_t *gcp = NULL;
  KWWnd_t *wp = NULL;
  
  wp = find_window(wid);
  if (wp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding window (id = %d).\n", getpid(), wid));
    return;
  }
  
  gcp = find_gc(gcid);
  if (gcp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding graphic context (id = %d).\n", getpid(), gcid));
    return;
  }
  
  drawWndPoint(wp, gcp->foreground, x, y);
}

/* =================
 *     draw line
 * ================= */

void 
drawWndLine(KWWnd_t *wp, KWColor_t color, int x1, int y1, int x2, int y2, bool dotted)
{
  KWDrawLineReq_t *req = NULL;
  
  req = AllocReq(wp, DrawLine);
  
  req->color = find_color(TRUECOLOR_0888, color);
  req->x1 = x1;
  req->y1 = y1;
  req->x2 = x2;
  req->y2 = y2;
  req->dotted = dotted;
}

void 
KDrawLine(unsigned int wid, unsigned int gcid, int x1, int y1, int x2, int y2, bool dotted)
{
  KWWnd_t *wp = NULL;
  KWGc_t  *gcp = NULL;
  
  wp = find_window(wid);
  if (wp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding window (id = %d).\n", getpid(), wid));
    return;
  }  
  
  gcp = find_gc(gcid);
  if (gcp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding graphic context (id = %d).\n", getpid(), gcid));
    return;
  }
  
  drawWndLine(wp, gcp->foreground, x1, y1, x2, y2, dotted);
}

/* =================
 *     draw rect
 * ================= */

void
drawWndRect(KWWnd_t *wp, KWColor_t color, int x, int y, int width, int height, bool dotted)
{
  KWDrawRectReq_t *req = NULL;
  
  req = AllocReq(wp, DrawRect);
  
  req->color = find_color(TRUECOLOR_0888, color);
  req->x = x;
  req->y = y;
  req->width = width;
  req->height = height;
  req->dotted = dotted;
}

void 
KDrawRect(unsigned int wid, unsigned int gcid, int x, int y, int width, int height, bool dotted)
{
  KWWnd_t *wp = NULL;
  KWGc_t  *gcp = NULL;
  
  wp = find_window(wid);
  if (wp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding window (id = %d).\n", getpid(), wid));
    return;
  }  
  
  gcp = find_gc(gcid);
  if (gcp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding graphic context: %d.", getpid(), gcid));
    return;
  }
  
  drawWndRect(wp, gcp->foreground, x, y, width, height, dotted);
}

/* =================
 *     fill rect
 * ================= */

void 
fillWndRect(KWWnd_t *wp, KWColor_t color, int x, int y, int width, int height, bool dotted)
{
  KWFillRectReq_t *req = NULL;
  
  req = AllocReq(wp, FillRect);
  
  req->color = find_color(TRUECOLOR_0888, color);
  req->x = x;
  req->y = y;
  req->width = width;
  req->height = height;
  req->dotted = dotted;
}

void 
KFillRect(unsigned int wid, unsigned int gcid, int x, int y, int width, int height, bool dotted)
{
  KWWnd_t *wp = NULL;
  KWGc_t *gcp = NULL;
  
  wp = find_window(wid);
  if (wp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding window (id = %d).\n", getpid(), wid));
    return;
  }
  
  gcp = find_gc(gcid);
  if (gcp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding graphic context: %d.", getpid(), gcid));
    return;
  }
  
  fillWndRect(wp, gcp->background, x, y, width, height, dotted);
}
