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

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include <kwlog/kwlog.h>
#include <kwgsshr/kwgsshr.h>
#include <kwgscli/kwgscli.h>

/* =======================
 *    static functions
 * ======================= */

static bool
check_data_type(KWWnd_t *wp, short type)
{
  short      b;
  KWEvent_t  event;
  
  /* read 2 bytes from the server */
  while (read_data_from_srv(wp, &b, sizeof(b)) != false) {
    if (b == type) {
      return true;
    }
    
    if (b == KWGetNextEventReqNum) {
      /* if we read a KWGetNextEventReqNum interger,
       * this means the following data from the cli_socket_fd will be the
       * event from the server,
       * so we will read that event and queue it for later processing.
       */
      read_data_from_srv(wp, &event, sizeof(event));
      queue_event(&event);
    } else {
      LOG(("<Client Error> client %d, window %d: wrong data type %d (expected %d)\n", getpid(), wp->id, b, type));
    }
  }
  
  LOG(("<Client Error> client %d, window %d: read_data_from_srv() failed.\n", getpid(), wp->id));
  
  return false;
}

/* =======================
 *    export functions
 * ======================= */

bool
write_data_to_srv(KWWnd_t *wp, char *buf, int length)
{
  int written;
  
  do {
    written = write(wp->socket_fd, buf, length);
    
    if (written < 0) {
      if (errno == EAGAIN || errno == EINTR) {
	continue;
      }
      
      LOG(("<Client Error> client %d, window %d: write_data_to_srv() failed.\n", getpid(), wp->id));
      return false;
    }
    
    buf += written;
    length -= written;
  } while (length > 0);
  
  return true;
}

bool
read_data_from_srv(KWWnd_t *wp, void *b, int n)
{
  int i = 0;
  char *v = (char *)b;
  
  flush_reqbuf(wp, 0);
  
  while (v < ((char *)b + n)) {
    i = read(wp->socket_fd, v, ((char *)b + n - v));
    if (i <= 0) {
      if (i == 0) {
	/* read EOF */
	LOG(("<Client Error> client %d, window %d: lost connection to server.\n", getpid(), wp->id));
	return false;
      } else {
	if (errno == EINTR || errno == EAGAIN) {
	  continue;
	}
	
	LOG(("<Client Error> client %d, window %d: read_data_from_srv() failed.\n", getpid(), wp->id));
	return false;
      }
    }
    v += i;
  }
  
  return true;
}

bool
read_typed_data_from_srv(KWWnd_t *wp, void *b, int n, int type)
{
  bool result = check_data_type(wp, type);
  
  if (result == false) {
    return false;
  }
  
  return read_data_from_srv(wp, b, n);
}
