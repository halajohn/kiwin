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

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <kwcommon/kwcommon.h>
#include <kwgsshr/kwgsshr.h>
#include <kwgssrv/kwgssrv.h>
#include <kwlog/kwlog.h>

/* ======================== 
 *     export variables
 * ======================== */

int srv_socket_fd = -1;    /* the server socket descriptor */

/* ======================== 
 *     static functions
 * ======================== */

/* This function is used to bind to the named socket which is used to
 * accept connections from the clients.
 */
static bool
open_server_socket(void)
{
  struct sockaddr_un sckt;
  
  if (access(KW_GS_NAMED_SOCKET, F_OK) == 0) {
    /* if the named socket file exist */
    if (unlink(KW_GS_NAMED_SOCKET) != 0) {
      LOG(("<Server Error> delete the server named socket %s failed, errno = %d\n", KW_GS_NAMED_SOCKET, errno));
      goto failed;
    }
  }
  
  /* create the socket */
  if ((srv_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    LOG(("<Server Error> can not create the server named socket, errno = %d\n", errno));
    goto failed;
  }
  
  /* bind a name to the socket */
  sckt.sun_family = AF_UNIX;
  strncpy(sckt.sun_path, KW_GS_NAMED_SOCKET, sizeof(sckt.sun_path));
  
  if (bind(srv_socket_fd, (struct sockaddr *)&sckt,
	   ((size_t)(((struct sockaddr_un *)0)->sun_path) + strlen((&sckt)->sun_path)) ) < 0) {
    LOG(("<Server Error> assign a name to the server named socket failed, errno = %d\n", errno));
    goto failed;
  }
  
  /* start listening on the socket */
  if (listen(srv_socket_fd, 5) == -1) {
    LOG(("<Server Error> enable the server named socket to accept client connections failed, errno = %d\n", errno));
    goto failed;
  }
  
  return true;
  
 failed:
  if (srv_socket_fd != -1) {
    close(srv_socket_fd);
  }
  return false;
}

static void
close_server_socket(void)
{
  if (srv_socket_fd != -1) {
    close(srv_socket_fd);
  }
  
  if (access(KW_GS_NAMED_SOCKET, F_OK) == 0) {
    unlink(KW_GS_NAMED_SOCKET);
  }
}

/* ======================== 
 *     export functions
 * ======================== */

bool
gs_init(void)
{
  if (open_server_socket() == false) {
    LOG(("<Server Error> can not open the server named socket, errno = %d\n", errno));
    return false;
  }
  
  return true;
}

void
gs_finalize(void)
{
  close_server_socket();
}
