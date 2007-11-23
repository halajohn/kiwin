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
#include <pthread.h>
#include <unistd.h>     /* getpid() */
#include <sys/types.h>  /* getpid() */

#include <kwlog/kwlog.h>
#include <kwcolor/kwcolor.h>
#include <kwgscli/kwgscli.h>

/* ========================
 *     static variables
 * ======================== */

static pthread_once_t thread_specific_gclist_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t  thread_specific_gclist_key;

/* ========================
 *     static functions
 * ======================== */

static void
create_thread_specific_gclist_key(void)
{
  pthread_key_create(&thread_specific_gclist_key, NULL);
}

static void
create_thread_specific_gclist(void)
{
  pthread_once(&thread_specific_gclist_key_once, create_thread_specific_gclist_key);
}

/* ========================
 *     export functions
 * ======================== */

KWGc_t*
find_gc(unsigned int gcid)
{
  KWGc_t   *gcp = NULL;
  KWGc_t   *gclist = NULL;
  
  gclist = pthread_getspecific(thread_specific_gclist_key);
  
  for (gcp = gclist; gcp != NULL; gcp = gcp->next) {
    if (gcp->id == gcid) {
      return gcp;
    }
  }
  
  return NULL;
}

unsigned int
KCreateGC(void)
{
  KWGc_t *gcp = NULL;
  KWGc_t *gclist = NULL;
  
  gcp = (KWGc_t *)malloc(sizeof(KWGc_t));
  if (gcp == NULL) {
    LOG(("<Client Error> client %d: allocate a new graphic context failed.\n", getpid()));
    return BAD_GC_ID;
  }
  
  gcp->id = (unsigned int)gcp;
  gcp->foreground = BLACK;
  gcp->background = WHITE;
  
  gclist = pthread_getspecific(thread_specific_gclist_key);
  
  gcp->next = gclist;
  pthread_setspecific(thread_specific_gclist_key, gcp);
  
  return gcp->id;
}

void
KSetGCFg(unsigned int gcid, KWColor_t foreground)
{
  KWGc_t *gcp = find_gc(gcid);
  
  if (gcp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding graphic context (id = %d).\n", getpid(), gcid));
    return;
  }
  
  gcp->foreground = foreground;
}

void
KSetGCBg(unsigned int gcid, KWColor_t background)
{
  KWGc_t *gcp = find_gc(gcid);
  
  if (gcp == NULL) {
    LOG(("<Client Error> client %d: can not find the corresponding graphic context (id = %d).\n", getpid(), gcid));
    return;
  }
  
  gcp->background = background;
}
