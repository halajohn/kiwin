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
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <kwgsshr/kwgsshr.h>
#include <kwgscli/kwgscli.h>
#include <kwregfile/kwregfile.h>
#include <kwlog/kwlog.h>
#include <kwcolor/kwcolor.h>

#include <gdbm/gdbm.h>

/* ========================
 *     static variables
 * ======================== */

static pthread_once_t  thread_specific_wndlist_key_once = PTHREAD_ONCE_INIT;

/* ========================
 *     export variables
 * ======================== */

pthread_key_t   thread_specific_wndlist_key;

/* ========================
 *     static functions
 * ======================== */

static void
create_thread_specific_wndlist_key(void)
{
  pthread_key_create(&thread_specific_wndlist_key, NULL);
}

static void
create_thread_specific_wndlist(void)
{
  pthread_once(&thread_specific_wndlist_key_once, create_thread_specific_wndlist_key);
}

/* ========================
 *     export functions
 * ======================== */

unsigned int
KCreateWindow(unsigned int parent, int x, int y, int width, int height,
	      KWColor_t bg_color, unsigned int props)
{
  KWNewWndReq_t      *req = NULL;
  KWWnd_t            *wp = NULL;
  KWWnd_t            *wndlist = NULL;
  struct sockaddr_un  name;
  size_t              size;
  int                 tries;
  int                 ret;
  
  if (width <= 0) {
    LOG(("<Client Warning> client %d: try to create a window with a negative width.\n", getpid()));
    return BAD_WND_ID;
  }
  
  if (height <= 0) {
    LOG(("<Client Warning> client $d: try to create a window with a negative height.\n", getpid()));
    return BAD_WND_ID;
  }
  
  wndlist = pthread_getspecific(thread_specific_wndlist_key);
  
  wp = (KWWnd_t *)malloc(sizeof(KWWnd_t));
  if (wp == NULL) {
    LOG(("<Client Error> client %d: allocate a new client window structure failed.\n", getpid()));
    goto failed;
  } else {
    /* do some initialization */
    wp->socket_fd = -1;
    wp->reqbuf.buffer = NULL;
  }
  
  if ((wp->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    LOG(("<Client Error> client %d: can not create a new socket for the new client window.\n", getpid()));
    goto failed;
  }
  
  /* try to connect to the kiwin server. */
  name.sun_family = AF_UNIX;
  strcpy(name.sun_path, KW_GS_NAMED_SOCKET);
  size = ((unsigned int)(((struct sockaddr_un *)0)->sun_path) + strlen(name.sun_path) + 1);
  
  for (tries = 1; tries <= 10; tries++) {
    struct timespec req;
    
    ret = connect(wp->socket_fd, (struct sockaddr *)&name, size);
    if (ret >= 0) {
      break;
    }
    
    req.tv_sec = 0;
    req.tv_nsec = 100000000;
    nanosleep(&req, NULL);
    LOG(("<Client Error> client %d: retry connecting to the kiwin server attempt %d\n", getpid(), tries));
  }
  
  if (ret == -1) {
    LOG(("<Client Error> client %d: retry 10 times and client still can not connect to the kiwin server.\n",
	 getpid()));
    goto failed;
  }
  /* connect to kiwin server successfully */
  
  wp->bg_color = bg_color;
  wp->props = props;
  wp->getevent_active = false;
  
  /* We have to call flush_reqbuf() first
   * to generate an initial request buffer,
   * otherwise, the following AllocReq() will be
   * segmentation fault !!!
   */
  if (flush_reqbuf((KWWnd_t *)wp, 0) == false) {
    LOG(("<Client Error> client %d: create the initial request buffer failed.\n", getpid()));
    goto failed;
  }
  
  /* send req to the kiwin server. */
  req = AllocReq((KWWnd_t *)wp, NewWnd);
  if (req == NULL) {
    LOG(("<Client Error> client %d: allocate a new window request failed.\n", getpid()));
    goto failed;
  }
  
  req->parentid = parent;
  req->x = x;
  req->y = y;
  req->width = width;
  req->height = height;
  req->props = wp->props;
  
  if (flush_reqbuf((KWWnd_t *)wp, 0) == false) {
    LOG(("<Client Error> client %d: flush the request buffer failed.\n", getpid()));
    goto failed;
  }
  
  if (read_typed_data_from_srv((KWWnd_t *)wp, &(wp->id), sizeof(wp->id), KWNewWndReqNum) == false) {
    LOG(("<Client Error> client %d: get the new window's id failed.\n", getpid()));
    goto failed;
  }
  
  if (wp->id == BAD_WND_ID) {
    LOG(("<Client Error> client %d: server response creating a new window failed.\n", getpid()));
    goto failed;
  }
  
  if (wndlist == NULL) {
    wp->next = NULL;
  } else {
    wp->next = wndlist;
  }
  pthread_setspecific(thread_specific_wndlist_key, wp);
  
  return wp->id;
  
 failed:
  if (wp != NULL) {
    if (wp->socket_fd != -1) {
      close(wp->socket_fd);
    }
    if (wp->reqbuf.buffer != NULL) {
      free(wp->reqbuf.buffer);
    }
    
    free(wp);
  }
  return BAD_WND_ID;
}

void
KMapWindow(unsigned int wid)
{
  KWWnd_t       *wp = NULL;
  KWMapWndReq_t *req = NULL;
  
  wp = find_window(wid);
  if (wp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding window (id = %d).\n", getpid(), wid));
    return;
  }
  
  req = AllocReq((KWWnd_t *)wp, MapWnd);
  if (req == NULL) {
    LOG(("<Client Error> client %d: allocate a map window request failed.\n", getpid()));
    return;
  }
}

KWWnd_t*
find_window(unsigned int id)
{
  KWWnd_t *wp = NULL;
  KWWnd_t *wndlist = NULL;
  
  wndlist = (KWWnd_t *)pthread_getspecific(thread_specific_wndlist_key);
  
  for (wp = wndlist; wp != NULL; wp = wp->next) {
    if (wp->id == id) {
      return wp;
    }
  }
  
  return NULL;
}
