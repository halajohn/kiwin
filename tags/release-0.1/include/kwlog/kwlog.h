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

#ifndef _KWLOG_H_
#define _KWLOG_H_

#include <kwcommon/kwcommon.h>

/* ==============
 *     macros
 * ============== */

#define FILE_MACRO        (__FILE__)
#define FUNCTION_MACRO    (__FUNCTION__)
#define LINE_MACRO        (__LINE__)

#define LOG(body)   (void)((_kw_loghdr(FILE_MACRO, FUNCTION_MACRO, LINE_MACRO)) && (_kw_log body))
#define LOGC(body)  (void)(_kw_log body)

/* ========================
 *     export functions
 * ======================== */

/* to define _kw_log() rather than log(),
 * because log() is already used in standard libc header file - math.h.
 */
extern bool _kw_log(char *format_str, ...);
extern bool _kw_loghdr(char *, char *, int);

#endif
