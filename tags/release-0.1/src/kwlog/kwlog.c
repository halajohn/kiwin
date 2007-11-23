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

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#include <kwlog/kwlog.h>

/* ========================
 *     static functions
 * ======================== */

static char*
timestring(void)
{
  static char TimeBuf[100];
  time_t t = time(NULL);
  struct tm tm;

  if (localtime_r(&t, &tm) == NULL) {
    snprintf(TimeBuf, sizeof(TimeBuf) - 1, "%d seconds since the Epoch", (int)t);
  } else {
    int len;

    /* from GNU Libc C info, to avoid treating empty result as a error */
    TimeBuf[0] = '\1';
    len = strftime(TimeBuf, 100, "%Y/%m/%d %H:%M:%S", &tm);
    if (len == 0 && TimeBuf[0] != '\0') {
      LOG(("%s\n", "<Error> fails on calling strftime()."));
      return NULL;
    }
  }

  return (TimeBuf);
}

/* ========================
 *     export functions
 * ======================== */

bool
_kw_loghdr(char *file, char *func, int line)
{
  int old_errno = errno;
  
  fprintf(stderr, "[%s] %s(%d):%s\n", timestring(), file, line, func);
  fflush(stderr);
  
  errno = old_errno;
  
  return true;
}

bool
_kw_log(char *format_str, ...)
{
  va_list ap;
  char space[2] = {0x20, 0x20};
  
  fwrite(space, 1, 2, stderr);
  
  va_start(ap, format_str);
  vfprintf(stderr, format_str, ap);
  fflush(stderr);
  va_end(ap);
  
  return true;
}
