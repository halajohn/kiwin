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

#include <fcntl.h>
#include <unistd.h>  /* read() & close() */
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ========================
 *     static variables
 * ======================== */

static int             mouse_device_fd;
static int             packet_size;
static int             mouseDriver_previous_x, mouseDriver_previous_y;
static KWMouseDriver_t current_mouse;
static pthread_t       mouse_thread;
static int             mouse_socket_fd_send;  /* communicate to the graphic server */

/* ========================
 *     export variables
 * ======================== */

int  mouse_socket_fd_receive;
int  mouseDriver_current_x, mouseDriver_current_y;

#include "kwgssrv_mouseps2.c"

/* ========================
 *     static functions
 * ======================== */

static void
send_mouse_event(KWMouseEventTypeInternal_t type)
{
  KWMouseEventInternal_t event;
  char *buf = (char *)(&event);
  int   written;
  int   length = sizeof(KWMouseEventInternal_t);
  
  event.type = type;
  event.x = mouseDriver_current_x;
  event.y = mouseDriver_current_y;
  
  do {
    written = write(mouse_socket_fd_send, buf, length);
    if (written < 0) {
      if (errno == EAGAIN || errno == EINTR) {
	continue;
      }
      
      LOG(("%s\n", "<Server Error> send mouse event to graphic server failed."));
      return;
    }
    
    buf += written;
    length -= written;
  } while (length > 0);
  
  return;
}

static int
process_mouse_data(unsigned char *buf, int buf_size)
{
  int eaten;
  KWMouseEvent_t event;
  static int old_left, old_middle, old_right;
  
  eaten = current_mouse.packet_data(buf, buf_size, &event);
  
  if (event.updated == true) {
    mouseDriver_current_x += event.dx;
    if ( mouseDriver_current_x > (current_screen.width - 2) ) {
      mouseDriver_current_x = current_screen.width - 2;
    }
    if (mouseDriver_current_x < 0) {
      mouseDriver_current_x = 0;
    }
    
    mouseDriver_current_y += event.dy;
    if (mouseDriver_current_y > (current_screen.height - 2) ) {
      mouseDriver_current_y = current_screen.height - 2;
    }
    if (mouseDriver_current_y < 0) {
      mouseDriver_current_y = 0;
    }
    
    if (event.left == 1) {
      if (old_left == 0) {
	/* press _left_ button */
	send_mouse_event(LEFT_BUTTON_DOWN);
      } else if ((mouseDriver_current_x != mouseDriver_previous_x) ||
		 (mouseDriver_current_y != mouseDriver_previous_y)) {
	/* grab _left_ button */
	send_mouse_event(LEFT_BUTTON_DRAG);
      }
    } else if (old_left == 1) {
      /* release _left_button */
      send_mouse_event(LEFT_BUTTON_UP);
    }
    
    if (event.middle == 1) {
      if (old_middle == 0) {
	/* press _middle_ button */
	send_mouse_event(MIDDLE_BUTTON_DOWN);
      } else if ((mouseDriver_current_x != mouseDriver_previous_x) ||
		 (mouseDriver_current_y != mouseDriver_previous_y)) {
	/* grab _middle_ button */
	send_mouse_event(MIDDLE_BUTTON_DRAG);
      }
    } else if (old_middle == 1) {
      /* release _middle_ button */
    }
    
    if (event.right == 1) {
      if (old_right == 0) {
	/* press _right_ button */
	send_mouse_event(RIGHT_BUTTON_DOWN);
      } else if ((mouseDriver_current_x != mouseDriver_previous_x) ||
		 (mouseDriver_current_y != mouseDriver_previous_y)) {
	/* grab _right_ button */
	send_mouse_event(RIGHT_BUTTON_DRAG);
      }
    } else if (old_right == 1) {
      /* release _right_ button */
    }
    
    if ((event.left == 0) && (old_left == 0) &&
	(event.middle == 0) && (event.middle == 0) &&
	(event.right == 0) && (event.right == 0)) {
      send_mouse_event(MOUSE_MOVE);
    }
    
    old_left = event.left;
    old_right = event.right;
    old_middle = event.middle;
  }
  
  return eaten;
}

static bool
update_mouse_state(void)
{
  static unsigned char buf[1024];
  static int bytes_in_buffer = 0;
  int bytes_read;
  
  bytes_read = read(mouse_device_fd, &buf[bytes_in_buffer], sizeof(buf) - bytes_in_buffer);
  if (bytes_read <= 0) {
    return false;
  }
  
  bytes_in_buffer += bytes_read;
  
  while ( bytes_in_buffer && (bytes_read = process_mouse_data(buf, bytes_in_buffer)) ) {
    int i;
    bytes_in_buffer -= bytes_read;
    
    for (i = 0; i < bytes_in_buffer; i++) {
      buf[i] = buf[i + bytes_read];
    }
  }
  
  return true;
}

static KWMouseType_t
probe_mouse(void)
{
  return ps2mouse;
}

static void*
mouse_thread_func(void *unused)
{
  while (1) {
    update_mouse_state();
  }
}

/* ==========================
 *     exported functions
 * ========================== */

bool
mouse_init(void)
{
  bool    result;
  size_t  size;
  int     tries;
  int     ret = 0;
  struct sockaddr_un name;

  current_mouse.type = probe_mouse();
  
  if (current_mouse.type == ps2mouse) {
    result = ps2_mouse_init();
    if (result == false) {
      LOG(("%s\n", "<Server Error> can _not_ initilaize the ps/2 mouse."));
      return false;
    }
  }
  
  mouseDriver_current_x = current_screen.width / 2;
  mouseDriver_current_y = current_screen.height / 2;
  mouseDriver_previous_x = mouseDriver_current_x;
  mouseDriver_previous_y = mouseDriver_current_y;
  
  /* connect to the graphic server */
  mouse_socket_fd_send = socket(AF_UNIX, SOCK_STREAM, 0);
  if (mouse_socket_fd_send == -1) {
    LOG(("%s\n", "<Server Error> can not open the mouse socket fd."));
    return false;
  }
  
  name.sun_family = AF_UNIX;
  strcpy(name.sun_path, KW_GS_NAMED_SOCKET);
  size = ((unsigned int)(((struct sockaddr_un *)0)->sun_path) + strlen(name.sun_path) + 1);

  for (tries = 1; tries <= 10; tries++) {
    struct timespec req;
    
    ret = connect(mouse_socket_fd_send, (struct sockaddr *) &name, size);
    if (ret >= 0) {
      break;
    }
    req.tv_sec = 0;
    req.tv_nsec = 100000000;
    nanosleep(&req, NULL);
    LOG(("<Server Error> mouse driver retry connect to graphic server attempt %d\n", tries));
  }

  if (ret == -1) {
    close(mouse_socket_fd_send);
    LOG(("%s\n", "<Server Error> retry 10 times and mouse driver still can not connect to graphic server."));
    return false;
  }
  
  pthread_create(&mouse_thread, NULL, mouse_thread_func, NULL);
  
  return true;
}

bool
mouse_finalize(void)
{
  pthread_cancel(mouse_thread);
  pthread_join(mouse_thread, NULL);

  close(mouse_device_fd);
  close(mouse_socket_fd_send);

  return true;
}
