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
#include <errno.h>
#include <pthread.h>
#include <unistd.h>     /* getpid() */
#include <sys/types.h>  /* getpid() */

#include <kwlog/kwlog.h>
#include <kwgsshr/kwgsshr.h>
#include <kwgscli/kwgscli.h>

/* ========================
 *     static variables
 * ======================== */

static pthread_once_t thread_specific_evQueue_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t  thread_specific_evQueue_key;

/* ========================
 *    static functions
 * ======================== */

static void
create_thread_specific_evQueue_key(void)
{
  pthread_key_create(&thread_specific_evQueue_key, NULL);
}

static void
create_thread_specific_evQueue(void)
{
  pthread_once(&thread_specific_evQueue_key_once, create_thread_specific_evQueue_key);
}

static void
get_next_queued_event(KWEventList_t *evQueue, KWEvent_t *ep)
{
  *ep = evQueue->event;
  pthread_setspecific(thread_specific_evQueue_key, evQueue->next);
}

static void
internal_event_processing(KWWnd_t *wp, KWEvent_t *ep)
{
  switch (ep->type) {
  }
}

/* ========================
 *     export functions
 * ======================== */

/* Queue an event in FIFO for later retrieval */
bool
queue_event(KWEvent_t *ep)
{
  KWEventList_t *eqp = NULL;
  KWEventList_t *preveqp = NULL;
  KWEventList_t *evQueue = NULL;
  
  evQueue = pthread_getspecific(thread_specific_evQueue_key);
  
  eqp = (KWEventList_t *)malloc(sizeof(KWEventList_t));
  if (eqp != NULL) {
    eqp->event = *ep;
    eqp->next = NULL;
    
    if (evQueue == NULL) {
      pthread_setspecific(thread_specific_evQueue_key, eqp);
      return true;
    }
    
    preveqp = evQueue;
    while (preveqp->next != NULL) {
      preveqp = preveqp->next;
    }
    preveqp->next = eqp;
  } else {
    LOG(("<Client Error> client %d: allocate a new event queue element failed.\n", getpid()));
    return false;
  }
  
  return true;
}

void 
KGetNextEvent(KWEvent_t *ep)
{
  fd_set                rfds;
  int                   setsize = 0;
  int                   e;
  KWWnd_t              *wp = NULL;
  KWGetNextEventReq_t  *req = NULL;
  KWWnd_t              *wndlist = NULL;
  KWEventList_t        *evQueue = NULL;
  
  wndlist = pthread_getspecific(thread_specific_wndlist_key);
  evQueue = (KWEventList_t *)pthread_getspecific(thread_specific_evQueue_key);
  
  if (evQueue != NULL) {
    get_next_queued_event(evQueue, ep);
    return;
  }
  
  FD_ZERO(&rfds);
  
  for (wp = wndlist; wp != NULL; wp = wp->next) {
    if (wp->getevent_active == false) {
      req = AllocReq(wp, GetNextEvent);
      if (req == NULL) {
	LOG(("<Client Error> client %d: allocate a new get event request failed.\n", getpid()));
	return;
      }
      
      flush_reqbuf(wp, 0);
      wp->getevent_active = true;
    }
    
    FD_SET(wp->socket_fd, &rfds);
    if (wp->socket_fd > setsize) {
      setsize = wp->socket_fd;
    }
  }
  
  if ((e = select(setsize + 1, &rfds, NULL, NULL, NULL)) > 0) {
    for (wp = wndlist; wp != NULL; wp = wp->next) {
      if (FD_ISSET(wp->socket_fd, &rfds)) {
	wp->getevent_active = false;
	read_typed_data_from_srv(wp, ep, sizeof(*ep), KWGetNextEventReqNum);
	
	internal_event_processing(wp, ep);
      }
    }
    return;
  } else {
    if (errno == EINTR) {
      LOG(("<Client Error> client %d: KGetNextEvent is interrupted by a signal.\n", getpid()));
      ep->type = EVENT_TYPE_NONE;
    } else {
      LOG(("<Client Error> client %d: KGetNextEvent failed.\n", getpid()));
      exit (-1);
    }
  }
}

bool
KSelectEvents(unsigned int wid, unsigned int eventmask)
{
  KWWnd_t *wp = NULL;
  KWSelectEventsReq_t *req = NULL;
  
  wp = find_window(wid);
  if (wp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding window (id = %d).\n", getpid(), wid));
    return false;
  }
  
  req = AllocReq(wp, SelectEvents);
  if (req == NULL) {
    LOG(("<Client Error> client %d: allocate a new select events request failed.\n", getpid()));
    return false;
  }
  
  req->eventmask = eventmask;
  
  return true;
}
