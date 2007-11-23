/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * Copyright (C) <2000>  Greg Haerr <greg@censoft.com>
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

#include <stdlib.h>
#include <errno.h>

#include <kwcommon/kwcommon.h>
#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export functions
 * ======================== */

KWEvent_t*
alloc_event(KWWnd_t *wp)
{
  KWEventList_t *elp = NULL;  /* current element list */
  
  /* Get a new event structure from the free list, or else
   * allocate it using malloc.
   */
  sem_wait(&(wp->free_event_list_sem));
  elp = wp->free_event_list;
  
  if (elp != NULL) {
    wp->free_event_list = elp->next;
    
    sem_post(&(wp->free_event_list_sem));
  } else {
    sem_post(&(wp->free_event_list_sem));
    
    elp = (KWEventList_t *)malloc(sizeof(KWEventList_t));
    if (elp == NULL) {
      LOG(("<Server Error> window %d: allocate a new event list failed.\n", wp->id));
      return NULL;
    }
  }
  
  sem_wait(&(wp->eventqueue_sem));
  
  /* Add the event to the end of the event list */
  if (wp->eventhead != NULL) {
    wp->eventtail->next = elp;
  } else {
    wp->eventhead = elp;
  }
  wp->eventtail = elp;
  elp->next = NULL;
  
  sem_post(&(wp->eventqueue_sem));
  
  elp->event.type = EVENT_TYPE_NONE;
  
  return &elp->event;
}

void
get_next_event(KWWnd_t *wp)
{
  KWEvent_t evt;
  KWEventList_t *elp = NULL;
  
  sem_wait(&(wp->eventqueue_sem));
  
  elp = wp->eventhead;
  if (elp == NULL) {
    sem_post(&(wp->eventqueue_sem));
    return;
  }
  
  wp->eventhead = wp->eventhead->next;
  if (wp->eventtail == elp) {
    wp->eventtail = NULL;
  }
  sem_post(&(wp->eventqueue_sem));
  
  evt = elp->event;
  write_typed_data_to_cli(wp->socket_fd, KWGetNextEventReqNum);
  write_data_to_cli(wp->socket_fd, &evt, sizeof(evt));
  wp->waiting_for_event = false;
  
  /* add this event to the head of the free_event_list */
  sem_wait(&(wp->free_event_list_sem));
  elp->next = wp->free_event_list;
  wp->free_event_list = elp;
  sem_post(&(wp->free_event_list_sem));
}

void
deliver_focus_in_event(KWWnd_t *wp)
{
  KWEventFocusChange_t *ep = NULL;
  
  if ((wp->eventmask & EVENT_MASK_FOCUS_IN) == 0) {
    return;
  }
  
  ep = (KWEventFocusChange_t *)alloc_event(wp);
  if (ep == NULL) {
    LOG(("<Server Error> window %d: allocate a focus in event failed.\n", wp->id));
    return;
  }
  
  ep->type = EVENT_TYPE_FOCUS_IN;
  ep->wid = wp->id;
}

void
deliver_focus_out_event(KWWnd_t *wp)
{
  KWEventFocusChange_t *ep = NULL;
  
  if ((wp->eventmask & EVENT_MASK_FOCUS_OUT) == 0) {
    return;
  }
  
  ep = (KWEventFocusChange_t *)alloc_event(wp);
  if (ep == NULL) {
    LOG(("<Server Error> window %d: allocate a focus out event failed.\n", wp->id));
    return;
  }
  
  ep->type = EVENT_TYPE_FOCUS_OUT;
  ep->wid = wp->id;
}

void
deliver_button_press_event(KWWnd_t *wp, int x, int y)
{
  KWEventButton_t *ep = NULL;
  
  if ((wp->eventmask & EVENT_MASK_BUTTON_PRESS) == 0) {
    return;
  }
  
  ep = (KWEventButton_t *)alloc_event(wp);
  if (ep == NULL) {
    LOG(("<Server Error> window %d: allocate a button press event failed.\n", wp->id));
    return;
  }
  
  ep->type = EVENT_TYPE_BUTTON_PRESS;
  ep->wid = wp->id;
  ep->x = x;
  ep->y = y;
}

void
deliver_motion_event(KWWnd_t *wp, int x, int y)
{
  KWEventMotion_t *ep = NULL;
  
  if ((wp->eventmask & EVENT_MASK_MOTION) == 0) {
    return;
  }
  
  ep = (KWEventMotion_t *)alloc_event(wp);
  if (ep == NULL) {
    LOG(("<Server Error> window %d: allocate a motion event failed.\n", wp->id));
    return;
  }
  
  ep->type = EVENT_TYPE_MOTION;
  ep->wid = wp->id;
  ep->x = x;
  ep->y = y;
}

void
deliver_exposure_event(KWWnd_t *wp, int x, int y, int width, int height)
{
  KWEventExposure_t *ep = NULL;
  
  if ((wp->mapped == false) ||
      ((wp->eventmask & EVENT_MASK_EXPOSURE) == 0)) {
    return;
  }
  
  ep = (KWEventExposure_t *)alloc_event(wp);
  if (ep == NULL) {
    LOG(("<Server Error> window %d: allocate an exposure event failed.\n", wp->id));
    return;
  }
  
  ep->type = EVENT_TYPE_EXPOSURE;
  ep->wid = wp->id;
  ep->x = x;
  ep->y = y;
  ep->width = width;
  ep->height = height;
}
