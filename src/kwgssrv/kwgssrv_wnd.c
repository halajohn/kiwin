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
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <semaphore.h>

#include <kwgsshr/kwgsshr.h>
#include <kwgssrv/kwgssrv.h>
#include <kwregfile/kwregfile.h>
#include <kwlog/kwlog.h>

/* =======================
 *     static variable
 * ======================= */

static void  reqHandler_new_window(KWWnd_t *wp, void *r);
static void  reqHandler_draw_point(KWWnd_t *wp, void *r);
static void  reqHandler_draw_line(KWWnd_t *wp, void *r);
static void  reqHandler_draw_rect(KWWnd_t *wp, void *r);
static void  reqHandler_fill_rect(KWWnd_t *wp, void *r);
static void  reqHandler_get_next_event(KWWnd_t *wp, void *r);
static void  reqHandler_select_events(KWWnd_t *wp, void *r);
static void  reqHandler_map_window(KWWnd_t *wp, void *r);

/* !!! Important !!!
 * if we have modified this structure,
 * we must modify the #defines macros in include/kwgsshr/kwreq.h.
 * Such that the ReqNum value can match this structure.
 */
static KWReqHandler_t reqHandlers[] = {
  /*  00 */ reqHandler_new_window,
  /*  01 */ reqHandler_draw_point,
  /*  02 */ reqHandler_draw_line,
  /*  03 */ reqHandler_draw_rect,
  /*  04 */ reqHandler_fill_rect,
  /*  05 */ reqHandler_get_next_event,
  /*  06 */ reqHandler_select_events,
  /*  07 */ reqHandler_map_window
};

static KWWndList_t    *free_wnd_list = NULL;

static sem_t           wndhash_sem;
static KWWnd_t        *wndhash[WNDHASH_SZ];

static sem_t           wndlist_sem;
static KWWnd_t        *wndlist = NULL;

static sem_t           cache_window_sem;
static KWWnd_t        *cache_window_ptr = NULL;
static unsigned int    cache_window_id = 0;

/* =======================
 *    export variables
 * ======================= */

sync_t          wnd_stack_sync;

KWWnd_t        *root_window_ptr = NULL;
KWWnd_t        *focus_window_ptr = NULL;
KWWnd_t        *drag_window_ptr = NULL;

/* =======================
 *    static functions
 * ======================= */

static bool
check_overlap_geometry(KWWnd_t *wp, int x, int y, int width, int height)
{
  int minx1, miny1, maxx1, maxy1;
  int minx2, miny2, maxx2, maxy2;
  int width1, height1;
  
  if (wp->mapped == false) {
    return false;
  } else {
    minx1 = wp->x;
    miny1 = wp->y;
    width1 = wp->width;
    height1 = wp->height;
    maxx1 = minx1 + width1 - 1;
    maxy1 = miny1 + height1 - 1;
    
    minx2 = x;
    maxx2 = x + width - 1;
    miny2 = y;
    maxy2 = y + height - 1;
    
    if ((minx1 > maxx2) || (minx2 > maxx1) ||
	(miny1 > maxy2) || (miny2 > maxy1)) {
      return false;
    } else {
      return true;
    }
  }
}

/* Check to see if the first window overlaps the second window */
static bool
check_overlap_wnd(KWWnd_t *topwp, KWWnd_t *botwp)
{
  int  minx1, miny1, maxx1, maxy1;
  int  minx2, miny2, maxx2, maxy2;
  int  width1, height1, width2, height2;
  
  if ((topwp->mapped == false) || (botwp->mapped == false)) {
    return false;
  } else {
    minx1 = topwp->x;
    miny1 = topwp->y;
    width1 = topwp->width;
    height1 = topwp->height;
    maxx1 = minx1 + width1 - 1;
    maxy1 = miny1 + height1 - 1;
    
    minx2 = botwp->x;
    miny2 = botwp->y;
    width2 = botwp->width;
    height2 = botwp->height;
    maxx2 = minx2 + width2 - 1;
    maxy2 = miny2 + height2 - 1;
    
    if ((minx1 > maxx2) || (minx2 > maxx1) ||
	(miny1 > maxy2) || (miny2 > maxy1)) {
      return false;
    } else {
      return true;
    }
  }
}

void
get_read_permit(sync_t *sync)
{
  sem_wait(&(sync->readWriteMutex));
  
  if ((sync->aw + sync->ww) == 0) {
    sem_post(&(sync->OkToRead));
    sync->ar = sync->ar + 1;
  } else {
    sync->wr = sync->wr + 1;
  }
  
  sem_post(&(sync->readWriteMutex));
  
  sem_wait(&(sync->OkToRead));
}

void
release_read_permit(sync_t *sync)
{
  sem_wait(&(sync->readWriteMutex));
  
  sync->ar = sync->ar - 1;
  if (sync->ar == 0 && sync->ww > 0) {
    sem_post(&(sync->OkToWrite));
    sync->aw = sync->aw + 1;
    sync->ww = sync->ww - 1;
  }
  
  sem_post(&(sync->readWriteMutex));
}

void
get_write_permit(sync_t *sync)
{
  sem_wait(&(sync->readWriteMutex));
  
  if ((sync->aw + sync->ar + sync->ww) == 0) {
    sem_post(&(sync->OkToWrite));
    sync->aw = sync->aw + 1;
  } else {
    sync->ww = sync->ww + 1;
  }
  
  sem_post(&(sync->readWriteMutex));
  
  sem_wait(&(sync->OkToWrite));
}

void
release_write_permit(sync_t *sync)
{
  sem_wait(&(sync->readWriteMutex));
  
  sync->aw = sync->aw - 1;
  if (sync->ww > 0) {
    sem_post(&(sync->OkToWrite));
    sync->aw = sync->aw + 1;
    sync->ww = sync->ww - 1;
  } else while (sync->wr > 0) {
    sem_post(&(sync->OkToRead));
    sync->ar = sync->ar + 1;
    sync->wr = sync->wr - 1;
  }
  
  sem_post(&(sync->readWriteMutex));
}

static KWWndList_t*
alloc_wndlist(KWWnd_t *wp)
{
  KWWndList_t *wlp = NULL;
  
  wlp = free_wnd_list;
  
  if (wlp != NULL) {
    free_wnd_list = wlp->next;
  } else {
    wlp = (KWWndList_t *)malloc(sizeof(KWWndList_t));
    if (wlp == NULL) {
      LOG(("%s\n", "<Server Error> allocate a new window list failed."));
      return NULL;
    }
  }
  
  wlp->wp = wp;
  wlp->next = NULL;
  
  return wlp;
}

static void
free_wndlist(KWWndList_t *wlp)
{
  KWWndList_t *saved_ptr = NULL;
  
  while (wlp != NULL) {
    saved_ptr = wlp->next;
    wlp->next = free_wnd_list;
    free_wnd_list = wlp;
    wlp = saved_ptr;
  }
}

static void handle_client_window_req(KWWnd_t *wp);

static void*
wnd_thread_func(void *window_ptr)
{
  KWWnd_t        *wp = window_ptr;
  fd_set          rset;
  fd_set          allset;
  int             maxfd = wp->socket_fd + 1;
  int             e;
  
  FD_ZERO(&allset);
  FD_SET(wp->socket_fd, &allset);
  
  while (1) {
    if (wp->waiting_for_event == true) {
      /* We have at least one pending event,
       * and the client window indeed requires one event.
       * Thus we pass the pending event to that client window.
       */
      get_next_event(wp);
    }
    
    /* Reset the read fdset used in select() system call,
     * because the last select() will alter the read fdset.
     */
    rset = allset;
    
    if ((e = select(maxfd, &rset, NULL, NULL, NULL)) > 0) {
      if (FD_ISSET(wp->socket_fd, &rset)) {
	handle_client_window_req(wp);
      }
    } else {
      if (errno != EINTR) {
	LOG(("<Server Error> window %d: select() failed.\n", wp->id));
      }
    }
  }
}

static bool
init_sync(sync_t *sync)
{
  sync->aw = 0;
  sync->ww = 0;
  sync->ar = 0;
  sync->wr = 0;
  
  if (sem_init(&(sync->readWriteMutex), 0, 1) == -1) {
    LOG(("%s\n", "<Server Error> readWriteMutex init failed."));
    return false;
  }
  
  if (sem_init(&(sync->OkToRead), 0, 0) == -1) {
    LOG(("%s\n", "<Server Error> OkToRead init failed."));
    sem_destroy(&(sync->readWriteMutex));
    return false;
  }
  
  if (sem_init(&(sync->OkToWrite), 0, 0) == -1) {
    LOG(("%s\n", "<Server Error> OkToWrite init failed."));
    sem_destroy(&(sync->readWriteMutex));
    sem_destroy(&(sync->OkToRead));
    return false;
  }
  
  return true;
}

static void
destroy_sync(sync_t *sync)
{
  sem_destroy(&(sync->readWriteMutex));
  sem_destroy(&(sync->OkToRead));
  sem_destroy(&(sync->OkToWrite));
}

static void
wndlist_add_all_children(KWWnd_t *wp, KWWndList_t **startp, KWWndList_t **endp)
{
  KWWndList_t  *tp = NULL;
  KWWnd_t      *childwp = NULL;
  
  tp = alloc_wndlist(wp);
  
  if ((*startp) == NULL) {
    (*startp) = tp;
    (*endp) = tp;
  } else {
    (*endp)->next = tp;
    (*endp) = tp;
  }
  
  for (childwp = wp->children; childwp != NULL; childwp = childwp->siblings) {
    wndlist_add_all_children(childwp, startp, endp);
  }
}

static bool
find_window_in_temp_wndlist(KWWndList_t **startp, KWWnd_t *wp)
{
  KWWndList_t *twlp = NULL;
  
  for (twlp = (*startp); ((twlp != NULL) && (twlp->wp != wp)); twlp = twlp->next);
  
  if (twlp == NULL) {
    return false;
  } else {
    return true;
  }
}

static void
wndlist_add_overlap_siblings_and_children(KWWnd_t *sibwp, KWWndList_t **startp, KWWndList_t **endp, KWWnd_t *wp, bool check_exist)
{
  KWWndList_t *tp = NULL;
  
  if (check_overlap_wnd(sibwp, wp) == true) {
    if ((check_exist == false) ||
	((check_exist == true) && (find_window_in_temp_wndlist(startp, sibwp) == false))) {
      tp = alloc_wndlist(sibwp);
      
      if ((*startp) == NULL) {
	(*startp) = tp;
	(*endp) = tp;
      } else {
	(*endp)->next = tp;
	(*endp) = tp;
      }
    }
  }
  
  for (sibwp = sibwp->children; sibwp != NULL; sibwp = sibwp->siblings) {
    wndlist_add_overlap_siblings_and_children(sibwp, startp, endp, wp, check_exist);
  }
}

static void
wndlist_add_all_overlap_siblings_and_children(KWWnd_t *wp, KWWndList_t **startp, KWWndList_t **endp, bool check_exist)
{
  KWWnd_t *sibwp = NULL;
  
  for (sibwp = wp->siblings; sibwp != NULL; sibwp = sibwp->siblings) {
    wndlist_add_overlap_siblings_and_children(sibwp, startp, endp, wp, check_exist);
  }
}

static void
offset_wnd_tree(KWWnd_t *wp, int x_offset, int y_offset)
{
  KWWnd_t *childwp = NULL;
  
  wp->x += x_offset;
  wp->y += y_offset;
  
  for (childwp = wp->children; childwp != NULL; childwp = childwp->siblings) {
    offset_wnd_tree(childwp, x_offset, y_offset);
  }
}

static void
map_wnd_tree(KWWnd_t *wp)
{
  KWWnd_t *childwp = NULL;
  
  wp->mapped = true;
  
  for (childwp = wp->children; childwp != NULL; childwp = childwp->siblings) {
    map_wnd_tree(childwp);
  }
}

static void
insert_wndhash(KWWnd_t *wp)
{
  KWWnd_t **htable = &wndhash[wnd_hashfn(wp->id)];
  
  sem_wait(&(wndhash_sem));
  
  wp = *htable;
  if (wp == NULL) {
    *htable = wp;
    sem_post(&(wndhash_sem));
    return;
  }
  
  for (; wp->wndhash_next != NULL; wp = wp->wndhash_next);
  
  wp->wndhash_next = wp;
  sem_post(&(wndhash_sem));
}

static void
remove_wndhash(unsigned int id)
{
  KWWnd_t  *wp = NULL;
  KWWnd_t  *prevwp = NULL;
  KWWnd_t **htable = &wndhash[wnd_hashfn(id)];
  
  sem_wait(&(wndhash_sem));
  
  for (wp = *htable; ((wp != NULL) && (wp->id != id)); prevwp = wp, wp = wp->wndhash_next);
  
  if (wp != NULL) {
    if (prevwp == NULL) {
      *htable = wp->wndhash_next;
      sem_post(&(wndhash_sem));
      return;
    }
    
    prevwp->wndhash_next = wp->wndhash_next;
  }
  
  sem_post(&(wndhash_sem));
}

static void
insert_wndlist(KWWnd_t *wp)
{
  sem_wait(&(wndlist_sem));
  
  if (wndlist == NULL) {
    root_window_ptr = wp;
  }
  
  wp->wndlist_next = wndlist;
  wndlist = wp;
  
  sem_post(&(wndlist_sem));
}

static void
remove_wndlist(unsigned int id)
{
  KWWnd_t *wp = NULL;
  KWWnd_t *prevwp = NULL;
  
  sem_wait(&(wndlist_sem));
  
  for (wp = wndlist; ((wp != NULL) && (wp->id != id)); prevwp = wp, wp = wp->wndlist_next);
  
  if (wp != NULL) {
    if (prevwp == NULL) {
      /* the window list is empty */
      sem_post(&(wndlist_sem));
      return;
    }
    
    prevwp->wndlist_next = wp->wndlist_next;
  }
  
  sem_post(&(wndlist_sem));
}

/* ===============================
 *     static request handlers
 * =============================== */

static void
reqHandler_new_window(KWWnd_t *wp, void *r)
{
  return;
}

static void
reqHandler_draw_point(KWWnd_t *wp, void *r)
{
  KWDrawPointReq_t *req = r;
  
  get_read_permit(&wnd_stack_sync);
  
  if (wp->need_recalc_clip == true) {
    calc_window_clip(wp);
    wp->need_recalc_clip = false;
  }
  
  draw_point(wp, req->x, req->y, req->color);
  
  release_read_permit(&wnd_stack_sync);
}

static void
reqHandler_draw_line(KWWnd_t *wp, void *r)
{
  KWDrawLineReq_t *req = r;
  
  get_read_permit(&wnd_stack_sync);
  
  if (wp->need_recalc_clip == true) {
    calc_window_clip(wp);
    wp->need_recalc_clip = false;
  }
  
  draw_line(wp, req->x1, req->y1, req->x2, req->y2, req->color, req->dotted);
  
  release_read_permit(&wnd_stack_sync);
}

static void
reqHandler_draw_rect(KWWnd_t *wp, void *r)
{
  KWDrawRectReq_t *req = r;
  
  get_read_permit(&wnd_stack_sync);
  
  if (wp->need_recalc_clip == true) {
    calc_window_clip(wp);
    wp->need_recalc_clip = false;
  }
  
  draw_rect(wp, req->x, req->y, req->width, req->height, req->color, req->dotted);
  
  release_read_permit(&wnd_stack_sync);
}

static void
reqHandler_fill_rect(KWWnd_t *wp, void *r)
{
  KWFillRectReq_t *req = r;
  
  get_read_permit(&wnd_stack_sync);
  
  if (wp->need_recalc_clip == true) {
    calc_window_clip(wp);
    wp->need_recalc_clip = false;
  }
  
  fill_rect(wp, req->x, req->y, req->width, req->height, req->color, req->dotted);
  
  release_read_permit(&wnd_stack_sync);
}

static void
reqHandler_get_next_event(KWWnd_t *wp, void *r)
{
  wp->waiting_for_event = true;
}

static void
reqHandler_select_events(KWWnd_t *wp, void *r)
{
  KWSelectEventsReq_t *req = r;
  
  wp->eventmask = req->eventmask;
}

static void
reqHandler_map_window(KWWnd_t *wp, void *r)
{
  map_window(wp);
}

static void
handle_client_window_req(KWWnd_t *wp)
{
  KWReqHeader_t *req = NULL;
  long           len;
  char           buf[MAX_REQUEST_SIZE];
  
  /* read request header */
  if (read_data_from_cli(wp->socket_fd, buf, sizeof(KWReqHeader_t)) == false) {
    LOG(("<Server Error> window %d: read request header failed.\n", wp->id));
    return;
  }
  
  len = ((KWReqHeader_t *)&buf[0])->length;
  if (len > MAX_REQUEST_SIZE) {
    LOG(("<Server Error> window %d: request size is too large: %d bytes > %d bytes\n", wp->id, len, MAX_REQUEST_SIZE));
    return;
  }
  
  /* read additional request data */
  if (read_data_from_cli(wp->socket_fd, &buf[sizeof(KWReqHeader_t)], len - sizeof(KWReqHeader_t)) == false) {
    LOG(("%s\n", "<Server Error> window %d: read additional request data failed.\n", wp->id));
    return;
  }
  
  req = (KWReqHeader_t *)&buf[0];
  
  if (req->reqType < TotalReqNum) {
    (reqHandlers[req->reqType])(wp, req);
  } else {
    LOG(("<Server Error> window %d: unknown request type: %d\n", wp->id, req->reqType));
  }
}

/* =======================
 *    export functions
 * ======================= */

bool
wnd_prework_init(void)
{
  int i;
  
  for (i = 0; i < WNDHASH_SZ; i++) {
    wndhash[i] = NULL;
  }
  
  if (sem_init(&(wndhash_sem), 0, 1) == -1) {
    LOG(("%s\n", "<Server Error> window hash table semaphore init failed."));
    return false;
  }
  
  if (init_sync(&(wnd_stack_sync)) == false) {
    LOG(("%s\n", "<Server Error> window stack sync semaphore collection failed."));
    return false;
  }
  
  return true;
}

void
wnd_prework_finalize(void)
{
  sem_destroy(&(wndhash_sem));
  destroy_sync(&(wnd_stack_sync));
}

bool
rootWnd_init(void)
{
  datum  key_data;
  datum  return_data;
  char  *argv[1];
#ifdef LOG_NONE_ERROR
  char  *debug_rootWnd_file_name = NULL;
#endif
  
  key_data.dptr = "root_wnd_path";
  key_data.dsize = strlen("root_wnd_path") + 1;
  
  return_data = gdbm_fetch(reg_file, key_data);
  
  if (return_data.dptr == NULL) {
    LOG(("%s\n", "<Server Error> kiwin can't get the root window file path from the regfile."));
    return false;
  }
  
  if (fork() == 0) {
    /* This is inside the child process */
    argv[0] = NULL;
    
#ifdef LOG_NONE_ERROR
    debug_rootWnd_file_name = (char *)malloc(return_data.dsize + 2);
    strcpy(debug_rootWnd_file_name, return_data.dptr);
    strcat(debug_rootWnd_file_name + return_data.dsize - 1, "_g");
    debug_rootWnd_file_name[return_data.dsize + 1] = '\0';
    
    if (execv(debug_rootWnd_file_name, argv) == -1) {
      LOG(("%s\n", "<Server Error> failed to execv() the root window executable."));
      free(return_data.dptr);
      free(debug_rootWnd_file_name);
      exit (-1);
    }
#else
    if (execv(return_data.dptr, argv) == -1) {
      LOG(("%s\n", "<Server Error> failed to execv() the root window executable."));
      free(return_data.dptr);
      exit (-1);
    }
#endif
  } else {
    /* This is inside the parent process (kiwin) */
    free(return_data.dptr);
  }
  
  return true;
}

/* This function is used to accept a connnection from a client window */
void
accept_window(int fd, KWNewWndReq_t *req)
{
  KWWnd_t            *wp = NULL;
  int                 bad_window_id = BAD_WND_ID;
  unsigned int        wm_props = 0;
  bool                eventqueue_sem_has_inited = false;
  bool                free_event_list_sem_has_inited = false;
  
  if ((wp = (KWWnd_t *)malloc(sizeof(KWWnd_t))) == NULL) {
    LOG(("%s\n", "<Server Error> allocate a new window failed."));
    goto failed;
  }
  
  wp->x              = req->x;
  wp->y              = req->y;
  wp->width          = req->width;
  wp->height         = req->height;
  wp->props          = req->props;
  
  if (wp->x > current_screen.width) {
    wp->x = 0;
  }
  if (wp->y > current_screen.height) {
    wp->y = 0;
  }
  
  wp->waiting_for_event = false;
  wp->eventmask = 0;
      
  if (sem_init(&(wp->eventqueue_sem), 0, 1) == -1) {
    LOG(("%s\n", "<Server Error> window's event queue semaphore init failed."));
    goto failed;
  } else {
    eventqueue_sem_has_inited = true;
  }
  wp->eventhead = NULL;
  wp->eventtail = NULL;
  
  if (sem_init(&(wp->free_event_list_sem), 0, 1) == -1) {
    LOG(("%s\n", "<Server Error> window's free event list semaphore init failed."));
    goto failed;
  } else {
    free_event_list_sem_has_inited = true;
  }
  wp->free_event_list = NULL;
  
  wp->id = (unsigned int)wp;
  wp->socket_fd = fd;
  
  wp->mapped = false;
  
  wp->clip.region.rects          = NULL;
  wp->clip.region.rects_have     = 0;
  wp->clip.region.rects_use      = 0;
  wp->clip.region.extents.left   = 0;
  wp->clip.region.extents.right  = 0;
  wp->clip.region.extents.top    = 0;
  wp->clip.region.extents.bottom = 0;
  wp->clip.clipminx = MIN_COORD;
  wp->clip.clipminy = MIN_COORD;
  wp->clip.clipmaxx = MAX_COORD;
  wp->clip.clipmaxy = MAX_COORD;
  wp->clip.clipresult = false;
  
  wp->children = NULL;
  
  if (req->parentid == 0) {
    wp->parent = root_window_ptr;
  } else {
    KWWnd_t *pwp = find_window_in_wndhash(req->parentid);
    if (pwp == NULL) {
      goto failed;
    }
    
    wp->parent = pwp;
  }
  
  wp->siblings = wp->parent->children;
  
  insert_wndhash(wp);
  insert_wndlist(wp);
  
  get_write_permit(&wnd_stack_sync);
  wp->parent->children = wp;
  release_write_permit(&wnd_stack_sync);
  
  /* Inform the client window its window id. */
  write_typed_data_to_cli(wp->socket_fd, KWNewWndReqNum);
  write_data_to_cli(wp->socket_fd, &(wp->id), sizeof(wp->id));
  
  pthread_create(&wp->wnd_thread, NULL, wnd_thread_func, wp);
  
  if (wp->props | WNDPROPS_HAVE_BORDER) {
    wm_props |= WNDPROPS_HAVE_BORDER;
  }
  if (wp->props | WNDPROPS_HAVE_CAPTION) {
    wm_props |= WNDPROPS_HAVE_CAPTION;
  }
  
  get_read_permit(&wmp_sync);
  if (wmp != NULL) {
    if (wm_props != 0) {
      KWEventInitWMComponent_t *ep = NULL;
      
      ep = (KWEventInitWMComponent_t *)alloc_event(wmp);
      if (ep == NULL) {
	LOG(("<Server Error> window %d: allocate window manager component failed.\n", wp->id));
      }
      
      ep->type = EVENT_TYPE_INIT_WMCOMPONENT;
      ep->parentid = wp->id;
      ep->width = wp->width;
      ep->height = wp->height;
      ep->props = wm_props;
    }
  }
  release_read_permit(&wmp_sync);
  return;
  
 failed:
  /* inform the client window this bad news. */
  write_typed_data_to_cli(fd, KWNewWndReqNum);
  write_data_to_cli(fd, &(bad_window_id), sizeof(bad_window_id));
  
  /* close the connection. */
  close(fd);
  
  if (eventqueue_sem_has_inited == true) {
    sem_destroy(&(wp->eventqueue_sem));
  }
  
  if (free_event_list_sem_has_inited == true) {
    sem_destroy(&(wp->free_event_list_sem));
  }
  
  if (wp != NULL) {
    free(wp);
  }
  return;
}

/* Return a pointer to the window structure with the specified window id.
 * Returns NULL if the window does not exist.
 */
KWWnd_t*
find_window_in_wndhash(unsigned int id)
{
  KWWnd_t  *wp = NULL;
  KWWnd_t **htable = NULL;
  
  sem_wait(&(cache_window_sem));
  if ((id == cache_window_id) && (id != 0)) {
    sem_post(&(cache_window_sem));
    return cache_window_ptr;
  }
  sem_post(&(cache_window_sem));
  
  htable = &wndhash[wnd_hashfn(id)];
  
  sem_wait(&(wndhash_sem));
  
  /* No, search for it and cache it for future calls */
  for (wp = *htable; ((wp != NULL) && (wp->id != id)); wp = wp->wndhash_next);
  
  sem_post(&(wndhash_sem));
  
  if (wp != NULL) {
    sem_wait(&(cache_window_sem));
    cache_window_ptr = wp;
    cache_window_id = wp->id;
    sem_post(&(cache_window_sem));
  }
  
  return wp;
}

void
move_window(KWWnd_t *wp, int x, int y)
{
  int          oldx, oldy;
  KWWndList_t *startp = NULL;
  KWWndList_t *endp = NULL;
  KWWndList_t *tp = NULL;
  KWWnd_t     *temp_wp = NULL;
  
  if (wp == root_window_ptr) {
    return;
  }
  
  get_write_permit(&wnd_stack_sync);
  
  wndlist_add_all_children(wp, &startp, &endp);
  wndlist_add_all_overlap_siblings_and_children(wp, &startp, &endp, false);
  
  oldx = wp->x;
  oldy = wp->y;
  offset_wnd_tree(wp, x - wp->x, y - wp->y);
  
  wndlist_add_all_overlap_siblings_and_children(wp, &startp, &endp, true);
  
  for (tp = startp; tp != NULL; tp = tp->next) {
    temp_wp = tp->wp;
    
    deliver_exposure_event(temp_wp, oldx - temp_wp->x, oldy - temp_wp->y, wp->width, wp->height);
    temp_wp->need_recalc_clip = true;
  }
  
  free_wndlist(startp);
  
  release_write_permit(&wnd_stack_sync);
}

void
map_window(KWWnd_t *wp)
{
  KWWndList_t *startp = NULL;
  KWWndList_t *endp = NULL;
  KWWndList_t *tp = NULL;
  KWWnd_t     *temp_wp = NULL;
  
  if (wp->mapped == true) {
    return;
  }
  
  get_write_permit(&wnd_stack_sync);
  
  map_wnd_tree(wp);
  
  wndlist_add_all_children(wp, &startp, &endp);
  wndlist_add_all_overlap_siblings_and_children(wp, &startp, &endp, false);
  
  for (tp = startp; tp != NULL; tp = tp->next) {
    temp_wp = tp->wp;
    
    deliver_exposure_event(temp_wp, 0, 0, temp_wp->width, temp_wp->height);
    temp_wp->need_recalc_clip = true;
  }
  
  free_wndlist(startp);
  
  release_write_permit(&wnd_stack_sync);
}

KWWnd_t*
find_visible_window(int x, int y)
{
  KWWnd_t  *wp = NULL;
  KWWnd_t  *pwp = NULL;
  int       wx, wy, wwidth, wheight;
  
  get_read_permit(&wnd_stack_sync);
  
  wp = root_window_ptr;
  
  while (wp != NULL) {
    wx = wp->x;
    wy = wp->y;
    wwidth = wp->width;
    wheight = wp->height;
    
    if ((wp->mapped == true) &&
	(x >= wx) && (y >= wy) &&
	(x < wx + wwidth) && (y < wy + wheight)) {
      pwp = wp;
      wp = wp->children;
    } else {
      wp = wp->siblings;
    }
  }
  
  release_read_permit(&wnd_stack_sync);
  
  return pwp;
}
