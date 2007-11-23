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

#ifndef _KWGSSRV_H_
#define _KWGSSRV_H_

#include <semaphore.h>
#include <kwgsshr/kwgsshr.h>

/* ==============
 *     macros
 * ============== */

#define	MIN_COORD                   ((int) -32768)   /* minimum coordinate value */
#define	MAX_COORD                   ((int) 32767)    /* maximum coordinate value */

/* clip areas */
#define CLIP_VISIBLE                0
#define CLIP_INVISIBLE              1
#define CLIP_PARTIAL                2

#define KIWIN_CURSOR_DIR            KWDIR"/Cursor/"

#define WNDHASH_SZ                  100
#define wnd_hashfn(x)               ((((x) >> 8) ^ (x)) & (WNDHASH_SZ - 1))

/* The reason why I open /dev/tty0 instead of /dev/tty is just because
 * I want to remote debug.
 */
#define KEYBOARD_DEV_FILE           "/dev/tty0"  /* console kbd to open */

#define KEY_RELEASED                0
#define KEY_PRESSED                 1
#define NUM_VGAKEYMAPS	            (1 << KG_CAPSSHIFT)  /* kernel key maps */

#define KWKEY_MODSTATE_NONE         0x0000
#define KWKEY_MODSTATE_LSHIFT	    0x0001
#define KWKEY_MODSTATE_RSHIFT	    0x0002
#define KWKEY_MODSTATE_LCTRL 	    0x0040
#define KWKEY_MODSTATE_RCTRL 	    0x0080
#define KWKEY_MODSTATE_LALT  	    0x0100
#define KWKEY_MODSTATE_RALT  	    0x0200
#define KWKEY_MODSTATE_LMETA 	    0x0400  /* Windows key */
#define KWKEY_MODSTATE_RMETA 	    0x0800  /* Windows key */
#define KWKEY_MODSTATE_NUM   	    0x1000
#define KWKEY_MODSTATE_CAPS  	    0x2000
#define KWKEY_MODSTATE_ALTGR 	    0x4000
#define KWKEY_MODSTATE_SCR	    0x8000

#define KWKEY_MODSTATE_CTRL	    (KWKEY_MODSTATE_LCTRL | KWKEY_MODSTATE_RCTRL)
#define KWKEY_MODSTATE_SHIFT	    (KWKEY_MODSTATE_LSHIFT | KWKEY_MODSTATE_RSHIFT)
#define KWKEY_MODSTATE_ALT	    (KWKEY_MODSTATE_LALT | KWKEY_MODSTATE_RALT)
#define KWKEY_MODSTATE_META	    (KWKEY_MODSTATE_LMETA | KWKEY_MODSTATE_RMETA)

/* the following key value is in the _ascii_ code format,
 * i.e. when we pressed the left key,
 * then we will get 0xF800 ascii code value.
 *
 * when we pressed keypad key 0,
 * we will get 0x0xF80A.
 */
#define KWKEY_UNKNOWN               0
				    
#define KWKEY_BACKSPACE             8
#define KWKEY_TAB                   9
#define KWKEY_ENTER                 13
#define KWKEY_ESCAPE                27
				    
#define KWKEY_FIRST		    0xF800
				    
#define KWKEY_LEFT		    0xF800
#define KWKEY_RIGHT		    0xF801
#define KWKEY_UP		    0xF802
#define KWKEY_DOWN		    0xF803
#define KWKEY_INSERT		    0xF804
#define KWKEY_DELETE		    0xF805
#define KWKEY_HOME		    0xF806
#define KWKEY_END		    0xF807
#define KWKEY_PAGEUP		    0xF808
#define KWKEY_PAGEDOWN		    0xF809

/* Numeric keypad */
#define KWKEY_KP0		    0xF80A
#define KWKEY_KP1		    0xF80B
#define KWKEY_KP2		    0xF80C
#define KWKEY_KP3		    0xF80D
#define KWKEY_KP4		    0xF80E
#define KWKEY_KP5		    0xF80F
#define KWKEY_KP6		    0xF810
#define KWKEY_KP7		    0xF811
#define KWKEY_KP8		    0xF812
#define KWKEY_KP9		    0xF813
#define KWKEY_KP_PERIOD		    0xF814
#define KWKEY_KP_DIVIDE		    0xF815
#define KWKEY_KP_MULTIPLY	    0xF816
#define KWKEY_KP_MINUS		    0xF817
#define KWKEY_KP_PLUS		    0xF818
#define KWKEY_KP_ENTER		    0xF819
#define KWKEY_KP_EQUALS		    0xF81A
				    
/* Function keys */		    
#define KWKEY_F1		    0xF81B
#define KWKEY_F2		    0xF81C
#define KWKEY_F3		    0xF81D
#define KWKEY_F4		    0xF81E
#define KWKEY_F5		    0xF81F
#define KWKEY_F6		    0xF820
#define KWKEY_F7		    0xF821
#define KWKEY_F8		    0xF822
#define KWKEY_F9		    0xF823
#define KWKEY_F10		    0xF824
#define KWKEY_F11		    0xF825
#define KWKEY_F12		    0xF827
				    
/* Key state modifier keys */	    
#define KWKEY_NUMLOCK		    0xF828
#define KWKEY_CAPSLOCK		    0xF829
#define KWKEY_SCROLLOCK		    0xF82A
#define KWKEY_LSHIFT		    0xF82B
#define KWKEY_RSHIFT		    0xF82C
#define KWKEY_LCTRL		    0xF82D
#define KWKEY_RCTRL		    0xF82E
#define KWKEY_LALT		    0xF82F
#define KWKEY_RALT		    0xF830
#define KWKEY_LMETA		    0xF831
#define KWKEY_RMETA		    0xF832
#define KWKEY_ALTGR		    0xF833
				    
/* Misc function keys */	    
#define KWKEY_PRINT		    0xF834
#define KWKEY_SYSREQ		    0xF835
#define KWKEY_PAUSE		    0xF836
#define KWKEY_BREAK		    0xF837
#define KWKEY_QUIT		    0xF838	/* virtual key */
#define KWKEY_MENU		    0xF839	/* virtual key */
#define KWKEY_REDRAW		    0xF83A	/* virtual key */
				    
#define KWKEY_IME                   0xF83B  /* virtual key */
				    
#define KWKEY_LAST		    0xF83B

/* ========================= 
 *     struct defination
 * ========================= */

typedef struct _KWCursor_t {
  int            width;
  int            height;
  int            x_hotspot;
  int            y_hotspot;
  unsigned char *data;
} KWCursor_t;

typedef enum _KWMouseEventTypeInternal_t {
  MOUSE_MOVE         = 0,
  LEFT_BUTTON_DOWN   = 1,
  LEFT_BUTTON_DRAG   = 2,
  LEFT_BUTTON_UP     = 3,
  MIDDLE_BUTTON_DOWN = 4,
  MIDDLE_BUTTON_DRAG = 5,
  MIDDLE_BUTTON_UP   = 6,
  RIGHT_BUTTON_DOWN  = 7,
  RIGHT_BUTTON_DRAG  = 8,
  RIGHT_BUTTON_UP    = 9
} KWMouseEventTypeInternal_t;

typedef struct _KWMouseEventInternal_t {
  enum _KWMouseEventTypeInternal_t type;
  int x;
  int y;
} KWMouseEventInternal_t;

typedef struct _KWMouseEvent_t {
  int  dx, dy;
  int  left, right, middle;
  bool updated;
} KWMouseEvent_t;

typedef enum _KWMouseType_t {
  ps2mouse = 0
} KWMouseType_t;

typedef struct _KWMouseDriver_t {
  enum _KWMouseType_t  type;
  int  (* packet_data)(unsigned char *buf, int buf_size, KWMouseEvent_t *info);
} KWMouseDriver_t;

typedef struct _KWRect_t {
  int         left;
  int         top;
  int         right;
  int         bottom;
} KWRect_t;

typedef struct _KWRegion_t {
  int         rects_have;   /* malloc'd # of rectangles */
  int         rects_use;    /* # rectangles in use      */
  KWRect_t    extents;      /* bounding box             */
  KWRect_t   *rects;        /* rectangle array          */
} KWRegion_t;

typedef struct _KWClip_t {
  KWRegion_t  region;
  int         clipminx;                     /* minimum x value of cache rectangle  */
  int         clipminy;                     /* minimum y value of cache rectangle  */
  int         clipmaxx;                     /* maximum x value of cache rectangle  */
  int         clipmaxy;                     /* maximum y value of cache rectangle  */
  bool        clipresult;                   /* whether clip rectangle is plottable */  
} KWClip_t;

typedef enum _KWScreenType_t {
  screen_fb = 0
} KWScreenType_t;

typedef struct _KWScreenDriver_t {
  KWScreenType_t  type;
  int             width;
  int             height;
  
  void            (* draw_pixel)(int x, int y, int color);
  int             (* read_pixel)(int x, int y);
  void            (* draw_line)(int x1, int y1, int x2, int y2, int color, bool dotted);
  void            (* draw_vertical_line)(int x, int y1, int y2, int color, bool dotted);
  void            (* draw_horizontal_line)(int y, int x1, int x2, int color, bool dotted);
  void            (* draw_rect)(int x, int y, int width, int height, int color, bool dotted);
  void            (* fill_rect)(int x, int y, int width, int height, int color, bool dotted);
} KWScreenDriver_t;

typedef struct _sync_t {
  sem_t                   readWriteMutex;
  sem_t                   OkToRead;
  sem_t                   OkToWrite;
  unsigned int            aw;
  unsigned int            ww;
  unsigned int            ar;
  unsigned int            wr;
} sync_t;

typedef struct _KWWnd_t {
  unsigned int            id;            /* window id                          */
  int                     socket_fd;
  bool                    waiting_for_event;
  sem_t                   eventqueue_sem;
  struct _KWEventList_t  *eventhead;
  struct _KWEventList_t  *eventtail;
  sem_t                   free_event_list_sem;
  struct _KWEventList_t  *free_event_list;
  pthread_t               wnd_thread;
  
  int                     x;
  int                     y;
  int                     width;
  int                     height;
  unsigned int            props;
  struct _KWWnd_t        *parent;
  struct _KWWnd_t        *children;
  struct _KWWnd_t        *siblings;
  unsigned int            eventmask;
  struct _KWWnd_t        *wndlist_next;  /* next window in complete list       */
  struct _KWWnd_t        *wndhash_next;
  bool                    mapped;        /* true if explicitly mapped          */
  struct _KWClip_t        clip;
  bool                    need_recalc_clip;
} KWWnd_t;

typedef struct _KWWndList_t {
  struct _KWWndList_t *next;
  struct _KWWnd_t     *wp;
} KWWndList_t;

typedef enum _KWKbdEventTypeInternal_t {
  KEY_DOWN = 0,
  KEY_UP   = 1
} KWKbdEventTypeInternal_t;

typedef struct _KWKbdEventInternal_t {
  KWKbdEventTypeInternal_t type;
  unsigned short           ch;
} KWKbdEventInternal_t;

typedef void (*voidProcp)();
typedef void (*KWReqHandler_t)(KWWnd_t *, void *);

/* ======================== 
 *     export variables
 * ======================== */

extern int                previous_x, previous_y;
extern int                current_x, current_y;
extern int                srv_socket_fd;    /* the server socket descriptor */
extern int                mouse_socket_fd_receive;
extern int                kbd_socket_fd_receive;
extern KWScreenDriver_t   current_screen;
extern KWWnd_t           *drag_window_ptr;
extern KWWnd_t           *list_window_ptr;
extern KWWnd_t           *root_window_ptr;
extern sync_t             wmp_sync;
extern KWWnd_t           *wmp;

/* ========================
 *     export functions
 * ======================== */

extern bool       gs_init(void);
extern void       gs_finalize(void);
extern bool       cursor_init(void);
extern bool       cursor_finalize(void);
extern bool       mouse_init(void);
extern bool       mouse_finalize(void);
extern bool       screen_init(void);
extern bool       screen_finalize(void);
extern bool       kbd_init(void);
extern bool       kbd_finalize(void);
extern bool       wnd_prework_init(void);
extern void       wnd_prework_finalize(void);
extern bool       rootWnd_init(void);
	          
extern void       process_kbd_event(void);
extern void       process_mouse_event(void);
	          
extern void       calc_window_clip(KWWnd_t *wp);
extern bool       check_point_in_clip(KWWnd_t *wp, int x, int y);
extern int        check_area_in_clip(KWWnd_t *wp, int x1, int y1, int x2, int y2);

extern void       restore_saved_block(int x, int y);
extern void       save_current_block(int x, int y);
extern void       draw_mouse_pointer(int x, int y);
	         
extern bool       write_data_to_cli(int fd, void *buf, int c);
extern bool       write_typed_data_to_cli(int fd, short type);
extern bool       read_data_from_cli(int fd, void *buf, int c);
	         
extern void       draw_point(KWWnd_t *wp, int x, int y, unsigned int color);
extern void       draw_line(KWWnd_t *wp, int x1, int y1, int x2, int y2, unsigned int color, bool dotted);
extern void       draw_rect(KWWnd_t *wp, int x, int y, int width, int height, unsigned int color, bool dotted);
extern void       fill_rect(KWWnd_t *wp, int x, int y, int width, int height, unsigned int color, bool dotted);

extern KWEvent_t* alloc_event(KWWnd_t *wp);
extern void       get_next_event(KWWnd_t *wp);

extern void       deliver_focus_in_event(KWWnd_t *wp);
extern void       deliver_focus_out_event(KWWnd_t *wp);
extern void       deliver_exposure_event(KWWnd_t *wp, int x, int y, int width, int height);

extern bool       init_region(KWRegion_t *region, int left, int top, int right, int bottom);
extern void       subtract_region(KWRegion_t *regD, KWRegion_t *regM, KWRegion_t *regS);

extern KWWnd_t*   find_visible_window(int x, int y);
extern void       set_toppest_window(KWWnd_t *wp);
		  
extern void       accept_window(int fd, KWNewWndReq_t *req);
extern void       accept_wmanager(int fd, KWNewWManagerReq_t *req);
extern KWWnd_t*   find_window_in_wndhash(unsigned int id);
extern void       map_window(KWWnd_t *wp);
extern void       move_window(KWWnd_t *wp, int x, int y);
extern KWWnd_t*   check_drag_window(KWWnd_t *wp, int x, int y);

extern void       get_read_permit(sync_t *sync);
extern void       release_read_permit(sync_t *sync);
extern void       get_write_permit(sync_t *sync);
extern void       release_write_permit(sync_t *sync);

#endif
