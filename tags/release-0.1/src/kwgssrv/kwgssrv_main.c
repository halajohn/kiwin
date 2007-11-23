/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * Copyright (C) <1999, 2000, 2001>  Greg Haerr <greg@censoft.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <kwlog/kwlog.h>
#include <kwgssrv/kwgssrv.h>
#include <kwregfile/kwregfile.h>

/* ========================
 *     static variables
 * ======================== */

static bool mouse_driver_init = false;
static bool kbd_driver_init = false;

/* ========================
 *     static functions
 * ======================== */

static void
main_select(void)
{
  fd_set  rfds;
  int     e;
  int     setsize = 0;
  struct  sockaddr_un sckt;
  size_t  size = sizeof(sckt);
  
  FD_ZERO(&rfds);
  
  if (mouse_driver_init == true) {
    /* handle events from mouse socket */
    FD_SET(mouse_socket_fd_receive, &rfds);
    if (mouse_socket_fd_receive > setsize) {
      setsize = mouse_socket_fd_receive;
    }
  }
  
  if (kbd_driver_init == true) {
    /* handle events from keyboard socket */
    FD_SET(kbd_socket_fd_receive, &rfds);
    if (kbd_socket_fd_receive > setsize) {
      setsize = kbd_socket_fd_receive;
    }
  }
  
  /* handle client socket connections */
  FD_SET(srv_socket_fd, &rfds);
  if (srv_socket_fd > setsize) {
    setsize = srv_socket_fd;
  }
  
  if ((e = select(setsize + 1, &rfds, NULL, NULL, NULL)) > 0) {
    if (mouse_driver_init == true) {
      if (FD_ISSET(mouse_socket_fd_receive, &rfds)) {
	process_mouse_event();
      }
    }
    
    if (kbd_driver_init == true) {
      if (FD_ISSET(kbd_socket_fd_receive, &rfds)) {
	process_kbd_event();
      }
    }
    
    if (mouse_driver_init == false) {
      mouse_socket_fd_receive = accept(srv_socket_fd, (struct sockaddr *)&sckt, &size);
      if (mouse_socket_fd_receive == -1) {
	LOG(("<Server Error> accept the mouse driver socket failed, errno = %d\n", errno));
	return;
      }
      
      mouse_driver_init = true;
    } else if (kbd_driver_init == false) {
      kbd_socket_fd_receive = accept(srv_socket_fd, (struct sockaddr *)&sckt, &size);
      if (kbd_socket_fd_receive == -1) {
	LOG(("<Server Error> accept the keyboard driver socket failed, errno = %d\n", errno));
	return;
      }
      
      kbd_driver_init = true;
    } else {
      /* we can only accept a client window unitl mouse and keyboard socket are connected.
       * If a client window is trying to connect, accept it.
       */
      if (FD_ISSET(srv_socket_fd, &rfds)) {
	int                 fd;
	fd_set              rfds;
	int                 setsize = 0;
	struct sockaddr_un  sckt;
	size_t              size = sizeof(sckt);
	char                buf[MAX_REQUEST_SIZE] = {0,};
	KWReqHeader_t      *req = NULL;
	int                 bad_window_id = BAD_WND_ID;
	
	/* try to build the connection between the client window and us. */
	if ((fd = accept(srv_socket_fd, (struct sockaddr *)&sckt, &size)) == -1) {
	  LOG(("%s\n", "<Server Error> accept a client failed."));
	  return;
	}
	/* the connection has been built. */
	
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	if (fd > setsize) {
	  setsize = fd;
	}
	
	/* We try to receive the first request from the client window through the connection just built,
	 * and this request has to be the KWNewWndReq_t.
	 */
	if ((e = select(setsize + 1, &rfds, NULL, NULL, NULL)) > 0) {
	  if (FD_ISSET(fd, &rfds)) {
	    if (read_data_from_cli(fd, buf, sizeof(KWReqHeader_t)) == false) {
	      /* We can not get an exact KWNewWndReq_t or KWNewWManagerReq_t from the client window
	       * through the connection, so we inform this bad news to the client.
	       */
	      write_typed_data_to_cli(fd, KWNewWndReqNum);
	      write_data_to_cli(fd, &(bad_window_id), sizeof(bad_window_id));
	      close(fd);
	      return;
	    }
	    
	    req = (KWReqHeader_t *)&buf[0];
	    switch (req->reqType) {
	    case KWNewWndReqNum:
	      accept_window(fd, (KWNewWndReq_t *)req);
	      break;
	      
	    case KWNewWManagerReqNum:
	      accept_wmanager(fd, (KWNewWManagerReq_t *)req);
	      break;
	    }
	  }
	} else {
	  LOG(("<Server Error> select() call in new window function failed, errno = %d.\n", errno));
	}
      }
    }
  } else {
    if (errno != EINTR) {
      LOG(("%s\n", "<Server Error> main loop failed.\n"));
    } else {
      LOG(("%s\n", "<Server Error> main loop is interrupted by a signal."));
    }
  }
}

static void
kw_init(void)
{
  if (screen_init() == false) {
    goto failed;
  }
  
  if (gs_init() == false) {
    goto failed;
  }
  
  /* the mouse driver must be initialized before the keyboard one,
   * because the server first accept mouse_socket_fd_receive,
   * and then kbd_socket_fd_receive,
   * so the mouse connect request must send to srv_socket_fd first.
   */
  if (mouse_init() == false) {
    goto failed;
  }
  
  if (kbd_init() == false) {
    goto failed;
  }
  
  if (regfile_init() == false) {
    goto failed;
  }
  
  if (cursor_init() == false) {
    goto failed;
  }
  
  if (wnd_prework_init() == false) {
    goto failed;
  }
    
  if (rootWnd_init() == false) {
    goto failed;
  }
  return;
  
 failed:
  cursor_finalize();
  regfile_finalize();
  kbd_finalize();
  mouse_finalize();
  gs_finalize();
  screen_finalize();
  wnd_prework_finalize();
}

static void
kw_finalize(void)
{
  cursor_finalize();
  regfile_finalize();
  kbd_finalize();
  mouse_finalize();
  gs_finalize();
  screen_finalize();
  wnd_prework_finalize();
}
 
/* ========================
 *     export functions
 * ======================== */

int
main(int argc, char** argv)
{
  kw_init();
  
  while (1) {
    main_select();
  }
  
  kw_finalize();
  
  fprintf(stderr, "%s\n", "");
  fprintf(stderr, "%s\n", " **************************************************************************");
  fprintf(stderr, "%s\n", " *************         Thank You for Using KiWin              *************");
  fprintf(stderr, "%s\n", " *************         If you have any problem,               *************");
  fprintf(stderr, "%s\n", " *************         you may contact the author by          *************");
  fprintf(stderr, "%s\n", " *************         e-mail: wei.hu.tw@gmail.com            *************");
  fprintf(stderr, "%s\n", " **************************************************************************");
  fprintf(stderr, "%s\n", "");
  
  exit (0);
}
