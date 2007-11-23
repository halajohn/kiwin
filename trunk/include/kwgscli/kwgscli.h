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

#ifndef _KWGSCLI_H_
#define _KWGSCLI_H_

#include <kwgsshr/kwgsshr.h>

/* ==============
 *     macros
 * ============== */

#define BAD_GC_ID  0

#define REQBUF_INIT_SIZE        2048  /* initial request buffer size */

#define AllocReq(wp, name) \
         ((KW##name##Req_t *)alloc_req(wp, KW##name##ReqNum, sizeof(KW##name##Req_t), 0))
#define AllocReqExtra(wp, name, n) \
         ((KW##name##Req_t *)alloc_req(wp, KW##name##ReqNum, sizeof(KW##name##Req_t), n))

/* =========================
 *     struct defination
 * ========================= */

typedef struct _KWGc_t {
  unsigned int     id;              /* graphics context id               */
  KWColor_t        foreground;      /* foreground color                  */
  KWColor_t        background;      /* background color                  */
  struct _KWGc_t  *next;            /* next graphics context             */
} KWGc_t;

/* queued request buffer */
typedef struct _KWReqBuf_t {
  unsigned char *bufptr;       /* next unused buffer location       */
  unsigned char *bufmax;       /* max buffer location               */
  unsigned char *buffer;       /* request buffer                    */
} KWReqBuf_t;

typedef struct _KWWnd_t {
  unsigned int                 id;  /* window id */
  int                          socket_fd;
  KWReqBuf_t                   reqbuf;
  unsigned int                 props;
  bool                         getevent_active;
  struct _KWWnd_t             *next;
  KWColor_t                    bg_color; /* background color */
} KWWnd_t;

typedef enum _KWWMProps_t {
  KWWmPropsDefBorderSize = 0,
  KWWmPropsDefCaptionHeight = 1,
  KWWmPropsDefCaptionLightColor = 2,
  KWWmPropsDefCaptionDarkColor = 3
} KWWMProps_t;

/* ========================
 *     export variables
 * ======================== */

extern pthread_key_t   thread_specific_wndlist_key;

/* ========================
 *     export functions
 * ======================== */

extern void  drawWndPoint(KWWnd_t *wp, KWColor_t color, int x, int y);
extern void  drawWndLine(KWWnd_t *wp, KWColor_t color, int x1, int y1, int x2, int y2, bool dotted);
extern void  drawWndRect(KWWnd_t *wp, KWColor_t color, int x, int y, int width, int height, bool dotted);
extern void  fillWndRect(KWWnd_t *wp, KWColor_t color, int x, int y, int width, int height, bool dotted);

extern void  KDrawPoint(unsigned int id, unsigned int gcid, int x, int y);
extern void  KDrawLine(unsigned int id, unsigned int gcid, int x1, int y1, int x2, int y2, bool dotted);
extern void  KDrawRect(unsigned int id, unsigned int gcid, int x, int y, int width, int height, bool dotted);
extern void  KFillRect(unsigned int id, unsigned int gcid, int x, int y, int width, int height, bool dotted);

extern bool  queue_event(KWEvent_t *);
extern void  KGetNextEvent(KWEvent_t *ep);
extern bool  KSelectEvents(unsigned int wid, unsigned int eventmask);

extern KWGc_t*       find_gc(unsigned int gcid);
extern unsigned int  KCreateGC(void);
extern void          KSetGCFg(unsigned int gc, KWColor_t foreground);
extern void          KSetGCBg(unsigned int gc, KWColor_t background);

extern bool write_data_to_srv(KWWnd_t *wp, char *buf, int length);
extern bool read_data_from_srv(KWWnd_t *wp, void *b, int n);
extern bool read_typed_data_from_srv(KWWnd_t *wp, void *b, int n, int type);

extern void *alloc_req(KWWnd_t *wp, int type, int size, int extra);
extern bool  flush_reqbuf(KWWnd_t *wp, int size_needed);

extern unsigned int  KRegisterWManager(unsigned int ability);

extern unsigned int  KCreateWindow(unsigned int parent, int x, int y, int width, int height, KWColor_t bg_color, unsigned int props);
extern void          KMapWindow(unsigned int wid);
extern KWWnd_t*      find_window(unsigned int id);

#endif
