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

/* ps/2 mouse protocol:
 *
 *                                 sign bits
 *   header byte          **          **        *          ***
 *                    overflow flags                current button state
 *
 *   x deltas bytes       **** ****
 *   y deltas bytes       **** ****
 *
 * The two data bytes are the X and Y deltas; prepend the 
 * corresponding sign bit to get a nine-bit two's complement number.
 */
static int
ps2_packet(unsigned char *buf, int buf_size, KWMouseEvent_t *event)
{
  event->updated = false;
  
  if (buf_size < packet_size) {
    return 0;  /* not enough data, spit it out for now */
  }
  
  /* if data is invalid, return no motion and all buttons released */
  event->left = event->right = event->middle = event->dx = event->dy = 0;
  event->updated = true;
  
  /* because the overflow bits in the header byte used rarely,
   * I test these 2 bits for a valid header byte.
   */
  if ((buf[0] & 0xc0) != 0) {
    event->updated = false;
    return 1; /* invalid byte, eat it */
  }
  
  /* data is valid, process it */
  event->left   = !!(buf[0] & 1);
  event->right  = !!(buf[0] & 2);
  event->middle = !!(buf[0] & 4);
  
  event->dx = buf[1];
  event->dy = buf[2];
  
  if (buf[1] != 0) {
    event->dx = (buf[0] & 0x10) ? buf[1] - 256 : buf[1];
    if ((event->dx > 10) || (event->dx < (-10))) {
      event->dx *= 2;
    }
  } else {
    event->dx = 0;
  }

  if (buf[2] != 0) {
    event->dy = -( (buf[0] & 0x20) ? buf[2] - 256 : buf[2] );
    if ((event->dy > 10) || (event->dy < (-10))) {
      event->dy *= 2;
    }
  } else {
    event->dy = 0;
  }

  event->updated = true;

  return packet_size;
}

static bool
ps2_mouse_init(void)
{
  fd_set set;
  int result;
  struct timeval tv;
  char buf[1];
  
  mouse_device_fd = open("/dev/psaux", O_RDONLY);
  if (mouse_device_fd < 0) {
    LOG(("%s\n", "<Server Error> unable to open /dev/psaux."));
    return false;
  }
  
  /* discard any garbage */
  do {
    FD_ZERO(&set);
    FD_SET(mouse_device_fd, &set);
    tv.tv_sec = tv.tv_usec = 0;
    result = select(FD_SETSIZE, &set, NULL, NULL, &tv);
    if (result > 0) {
      read(mouse_device_fd, buf, 1);
    }
  } while (result > 0);
  
  packet_size = 3;
  
  current_mouse.packet_data = ps2_packet;
  
  return true;
}
