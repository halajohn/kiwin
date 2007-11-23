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
#include <sys/types.h>

#include <kwlog/kwlog.h>
#include <kwgsshr/kwgsshr.h>
#include <kwgscli/kwgscli.h>

/* ========================
 *     static functions
 * ======================== */

static bool
alloc_init_reqbuf(KWWnd_t *wp, int newsize)
{
  if (newsize < (int)REQBUF_INIT_SIZE) {
    newsize = REQBUF_INIT_SIZE;
  }
  
  wp->reqbuf.buffer = (unsigned char *)malloc(newsize);
  if (wp->reqbuf.buffer == NULL) {
    LOG (("<Client Error> client %d, window unit %d: can't allocate initial request buffer.", getpid(), wp->id));
    return false;
  }
  
  wp->reqbuf.bufptr = wp->reqbuf.buffer;
  wp->reqbuf.bufmax = wp->reqbuf.buffer + newsize;
  
  return true;
}

/* ========================
 *     export functions
 * ======================== */

/* Allocate a request of passed size and fill in header fields */
void*
alloc_req(KWWnd_t *wp, int type, int size, int extra)
{
  KWReqHeader_t *req = NULL;
  int total_size = size + extra;
  
  if (total_size > MAX_REQUEST_SIZE) {
    LOG(("<Client Error> client %d, window unit %d: can't allocate a request more than %d bytes.",
	 getpid(), wp->id, MAX_REQUEST_SIZE));
    return NULL;
  }
  
  /* flush buffer if required, and allocate larger one if required */
  if (wp->reqbuf.bufptr + total_size >= wp->reqbuf.bufmax) {
    flush_reqbuf(wp, total_size);
  }
  
  /* fill in request header */
  req = (KWReqHeader_t *)wp->reqbuf.bufptr;
  
  req->reqType  = (unsigned char)type;
  req->length   = (unsigned short)total_size;
  
  wp->reqbuf.bufptr += total_size;
  
  return req;
}

/* Flush request buffer if required, possibly reallocate buffer size */
bool
flush_reqbuf(KWWnd_t *wp, int size_needed)
{
  /* handle one-time initialization case */
  if (wp->reqbuf.buffer == NULL) {
    return alloc_init_reqbuf(wp, size_needed);
  }
  
  /* flush buffer if required */
  if (wp->reqbuf.bufptr > wp->reqbuf.buffer) {
    char *buf = wp->reqbuf.buffer;
    int	length = wp->reqbuf.bufptr - wp->reqbuf.buffer;
    
    /* standard socket transfer */
    write_data_to_srv(wp, buf, length);
    wp->reqbuf.bufptr = wp->reqbuf.buffer;
  }
  
  /* allocate larger buffer for current request, if needed */
  if (wp->reqbuf.bufptr + size_needed >= wp->reqbuf.bufmax) {
    wp->reqbuf.buffer = realloc(wp->reqbuf.buffer, size_needed);
    
    if (wp->reqbuf.buffer == NULL) {
      LOG(("<Client Error> client %d, window unit %d: can't reallocate request buffer.\n", getpid(), wp->id));
      return false;
    }
    
    wp->reqbuf.bufptr = wp->reqbuf.buffer;
    wp->reqbuf.bufmax = wp->reqbuf.buffer + size_needed;
  }
  
  return true;
}
