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
#include <unistd.h>     /* getpid() */
#include <pthread.h>
#include <sys/types.h>  /* getpid() */
#include <sys/socket.h>
#include <sys/un.h>

#include <kwgsshr/kwgsshr.h>
#include <kwgscli/kwgscli.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export functions
 * ======================== */

unsigned int
KRegisterWManager(unsigned int props)
{
  KWNewWManagerReq_t *req = NULL;
  KWWnd_t            *wmp = NULL;
  KWWnd_t            *wndlist = NULL;
  struct sockaddr_un  name;
  size_t              size;
  int                 tries;
  int                 ret;
  
  wndlist = pthread_getspecific(thread_specific_wndlist_key);
  
  wmp = (KWWnd_t *)malloc(sizeof(KWWnd_t));
  if (wmp == NULL) {
    LOG(("<Window Manager Error> window manager %d: allocate a new window manager structure failed.\n", getpid()));
    goto failed;
  } else {
    /* do some initialization */
    wmp->socket_fd = -1;
    wmp->reqbuf.buffer = NULL;
  }
  
  if ((wmp->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    LOG(("<Window Manager Error> window manager %d: can not create a new socket for the new window manager.\n", getpid()));
    goto failed;
  }
  
  /* try to connect to the kiwin server. */
  name.sun_family = AF_UNIX;
  strcpy(name.sun_path, KW_GS_NAMED_SOCKET);
  size = ((unsigned int)(((struct sockaddr_un *)0)->sun_path) + strlen(name.sun_path) + 1);
  
  for (tries = 1; tries <= 10; tries++) {
    struct timespec req;
    
    ret = connect(wmp->socket_fd, (struct sockaddr *)&name, size);
    if (ret >= 0) {
      break;
    }
    
    req.tv_sec = 0;
    req.tv_nsec = 100000000;
    nanosleep(&req, NULL);
    LOG(("<Window Manager Error> window manager %d: retry connecting to the kiwin server attempt %d\n", getpid(), tries));
  }
  
  if (ret == -1) {
    LOG(("<Window Manager Error> window manager %d: retry 10 times and client still can not connect to the kiwin server.\n",
	 getpid()));
    goto failed;
  }
  /* connect to kiwin server successfully */
  
  wmp->props = props;
  wmp->getevent_active = false;
  
  /* We have to call flush_reqbuf() first
   * to generate an initial request buffer,
   * otherwise, the following AllocReq() will be
   * segmentation fault !!!
   */
  if (flush_reqbuf(wmp, 0) == false) {
    LOG(("<Window Manager Error> window manager %d: create the initial request buffer failed.\n", getpid()));
    goto failed;
  }
  
  /* send req to the kiwin server. */
  req = AllocReq(wmp, NewWManager);
  if (req == NULL) {
    LOG(("<Window Manager Error> window manager %d: allocate a new window manager request failed.\n", getpid()));
    goto failed;
  }
  
  if (flush_reqbuf(wmp, 0) == false) {
    LOG(("<Window Manager Error> window manager %d: flush the request buffer failed.\n", getpid()));
    goto failed;
  }
  
  if (read_typed_data_from_srv(wmp, &(wmp->id), sizeof(wmp->id), KWNewWndReqNum) == false) {
    LOG(("<Window Manager Error> window manager %d: get the new window manager's id failed.\n", getpid()));
    goto failed;
  }
  
  if (wmp->id == BAD_WMANAGER_ID) {
    LOG(("<Window Manager Error> window manager %d: server response creating a new window manager failed.\n", getpid()));
    goto failed;
  }
  
  if (wndlist == NULL) {
    wmp->next = NULL;
  } else {
    wmp->next = wndlist;
  }
  pthread_setspecific(thread_specific_wndlist_key, wmp);
  
  return wmp->id;
  
 failed:
  if (wmp != NULL) {
    if (wmp->socket_fd != -1) {
      close(wmp->socket_fd);
    }
    if (wmp->reqbuf.buffer != NULL) {
      free(wmp->reqbuf.buffer);
    }
    
    free(wmp);
  }
  return BAD_WMANAGER_ID;
}
