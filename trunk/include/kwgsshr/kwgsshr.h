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

#ifndef _KWGSSHR_H_
#define _KWGSSHR_H_

#include <kwcolor/kwcolor.h>
#include <kwcommon/kwcommon.h>

/* ==============
 *     macros
 * ============== */

#define KW_GS_NAMED_SOCKET           "/tmp/.kiwin"

#define BAD_WND_ID                   0
#define BAD_WMANAGER_ID              0

#define	MIN(a,b)                     ((a) < (b) ? (a) : (b))
#define	MAX(a,b)                     ((a) > (b) ? (a) : (b))

#define WNDPROPS_HAVE_BORDER         0x10000000    /* border           */
#define WNDPROPS_HAVE_CAPTION        0x20000000    /* Title bar        */
#define WNDPROPS_HAVE_DRAGBAR        0x40000000    /* drag bar         */

#define EVENT_TYPE_NONE              0
#define EVENT_TYPE_EXPOSURE          1
#define EVENT_TYPE_FOCUS_IN          2
#define EVENT_TYPE_FOCUS_OUT         3
#define EVENT_TYPE_BUTTON_PRESS      4
#define EVENT_TYPE_MOTION            5
#define EVENT_TYPE_INIT_WMCOMPONENT  6

#define EVENT_MASK_EXPOSURE          (1 << EVENT_TYPE_EXPOSURE)
#define EVENT_MASK_FOCUS_IN          (1 << EVENT_TYPE_FOCUS_IN)
#define EVENT_MASK_FOCUS_OUT         (1 << EVENT_TYPE_FOCUS_IN)
#define EVENT_MASK_BUTTON_PRESS      (1 << EVENT_TYPE_BUTTON_PRESS)
#define EVENT_MASK_MOTION            (1 << EVENT_TYPE_MOTION)

#define MAX_REQUEST_SIZE             30000

#define KWNewWndReqNum               0
#define KWDrawPointReqNum            1
#define KWDrawLineReqNum             2
#define KWDrawRectReqNum             3
#define KWFillRectReqNum             4
#define KWGetNextEventReqNum         5
#define KWSelectEventsReqNum         6
#define KWMapWndReqNum               7
#define KWNewWManagerReqNum          8
#define TotalReqNum                  9

/* ==========================
 *     struct definations
 * ========================== */

typedef struct _KWEventExposure_t {
  int           type;            /* event type                      */
  unsigned int  wid;
  int           x;               /* window x coordinate of exposure */
  int           y;               /* window y coordinate of exposure */
  int           width;           /* width of exposure               */
  int           height;          /* height of exposure              */
} KWEventExposure_t;

typedef struct _KWEventFocusChange_t {
  int           type;            /* event type                      */
  unsigned int  wid;
} KWEventFocusChange_t;

typedef struct _KWEventInitWMComponent_t {
  int           type;
  unsigned int  parentid;
  int           width;
  int           height;
  unsigned int  props;
} KWEventInitWMComponent_t;

typedef struct _KWEventButton_t {
  int           type;
  unsigned int  wid;
  int           x;
  int           y;
} KWEventButton_t;

typedef struct _KWEventMotion_t {
  int           type;
  unsigned int  wid;
  int           x;
  int           y;
} KWEventMotion_t;

/* Union of all possible event structures.
 * This is the structure returned by the get_next_event() and similar routines.
 */
typedef union _KWEvent_t {
  int                       type;              /* event type */
  KWEventExposure_t         exposure;
  KWEventFocusChange_t      focusChange;
  KWEventInitWMComponent_t  initWMComponent;
} KWEvent_t;

/* event queue element */
typedef struct _KWEventList_t {
  struct _KWEventList_t *next;   /* next element in the event queue */
  KWEvent_t      event;  /* event */
} KWEventList_t;

/* all requests share this header */
typedef struct _KWReqHeader_t {
  unsigned char  reqType;      /* request code                      */
  unsigned short length;       /* lower 16 bits of unaligned length */
} KWReqHeader_t;

typedef struct _KWNewWndReq_t {
  unsigned char  reqType;
  unsigned short length;
  unsigned int   parentid;
  int            x;
  int            y;
  int            width;
  int            height;
  unsigned int   props;
} KWNewWndReq_t;

typedef struct _KWNewWManagerReq_t {
  unsigned char  reqType;
  unsigned short length;
} KWNewWManagerReq_t;

typedef struct _KWDrawPointReq_t {
  unsigned char  reqType;
  unsigned short length;
  KWColor_t      color;
  short          x;
  short          y;
} KWDrawPointReq_t;

typedef struct _KWDrawLineReq_t {
  unsigned char  reqType;
  unsigned short length;
  KWColor_t      color;
  short          x1;
  short          y1;
  short          x2;
  short          y2;
  bool           dotted;
} KWDrawLineReq_t;

typedef struct _KWDrawRectReq_t {
  unsigned char  reqType;
  unsigned short length;
  KWColor_t      color;
  short          x;
  short          y;
  short          width;
  short          height;
  bool           dotted;
} KWDrawRectReq_t;

typedef struct _KWFillRectReq_t {
  unsigned char  reqType;
  unsigned short length;
  KWColor_t      color;
  short          x;
  short          y;
  short          width;
  short          height;
  bool           dotted;
} KWFillRectReq_t;

typedef struct _KWGetNextEventReq_t {
  unsigned char  reqType;
  unsigned short length;
} KWGetNextEventReq_t;

typedef struct _KWSelectEventsReq_t {
  unsigned char  reqType;
  unsigned short length;
  unsigned int   eventmask;
} KWSelectEventsReq_t;

typedef struct _KWMapWndReq_t {
  unsigned char  reqType;
  unsigned short length;
} KWMapWndReq_t;

#endif
