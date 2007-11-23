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
#include <pthread.h>

#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ========================
 *     export variables
 * ======================== */

sync_t   wmp_sync;
KWWnd_t *wmp = NULL;

/* ========================
 *     static functions
 * ======================== */

static void*
wmanager_thread_func(void* unused)
{
  while (1) {
    if (wmp->waiting_for_event == true) {
      get_next_event(wmp);
    }
  }
}

/* ========================
 *     export functions
 * ======================== */

void
accept_wmanager(int fd, KWNewWManagerReq_t *req)
{
  int                 bad_wmanager_id = BAD_WMANAGER_ID;
  bool                eventqueue_sem_has_inited = false;
  bool                free_event_list_sem_has_inited = false;
  
  get_write_permit(&wmp_sync);
  if (wmp != NULL) {
    LOG(("%s\n", "<Server Error> We already had a window manager."));
    goto failed;
  }
  
  if ((wmp = (KWWnd_t *)malloc(sizeof(KWWnd_t))) == NULL) {
    LOG(("%s\n", "<Server Error> allocate a new window manager failed."));
    goto failed;
  }
  
  wmp->waiting_for_event = false;
  
  if (sem_init(&(wmp->eventqueue_sem), 0, 1) == -1) {
    LOG(("%s\n", "<Server Error> window manager's event queue semaphore init failed."));
    goto failed;
  } else {
    eventqueue_sem_has_inited = true;
  }
  wmp->eventhead = NULL;
  wmp->eventtail = NULL;
  
  if (sem_init(&(wmp->free_event_list_sem), 0, 1) == -1) {
    LOG(("%s\n", "<Server Error> window's free event list semaphore init failed."));
    goto failed;
  } else {
    free_event_list_sem_has_inited = true;
  }
  wmp->free_event_list = NULL;
  
  wmp->id = (unsigned int)wmp;
  wmp->socket_fd = fd;
  
  /* Inform the client window its window manager id. */
  write_typed_data_to_cli(wmp->socket_fd, KWNewWManagerReqNum);
  write_data_to_cli(wmp->socket_fd, &(wmp->id), sizeof(wmp->id));
  
  pthread_create(&wmp->wnd_thread, NULL, wmanager_thread_func, NULL);
  release_write_permit(&wmp_sync);
  return;
  
 failed:
  /* inform the client window this bad news. */
  write_typed_data_to_cli(fd, KWNewWndReqNum);
  write_data_to_cli(fd, &(bad_wmanager_id), sizeof(bad_wmanager_id));
  
  /* close the connection. */
  close(fd);
  
  if (eventqueue_sem_has_inited == true) {
    sem_destroy(&(wmp->eventqueue_sem));
  }
  
  if (free_event_list_sem_has_inited == true) {
    sem_destroy(&(wmp->free_event_list_sem));
  }
  
  if (wmp != NULL) {
    free(wmp);
    wmp = NULL;
  }
  
  release_write_permit(&wmp_sync);
  return;
}
