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

#ifndef _KWXPM_H_
#define _KWXPM_H_

#include <stdio.h>

/* ==============
 *     macros
 * ============== */

#define USE_HASHTABLE       (cpp > 2 && ncolors > 4)
#define HASH_FUNCTION 	    hash = (hash << 5) - hash + *hp++;
#define INITIAL_HASH_SIZE   256  /* should be enough for colors */
#define HASH_TABLE_GROWS    size = size * 2;

/* number of xpmColorKeys */
#define NKEYS               5

#define XCOLOR_NUM          752

/* =========================
 *     struct defination
 * ========================= */

typedef struct xpmData_t {
  FILE         *file;
  char          Bos;
  char          Eos;
} xpmData;

typedef struct XpmColor_t {
  char          *string;     /* characters string             */
  char          *symbolic;   /* symbolic name                 */
  char          *m_color;    /* monochrom default             */
  char          *g4_color;   /* 4 level grayscale default     */
  char          *g_color;    /* other level grayscale default */
  char          *c_color;    /* color default                 */
} XpmColor;

typedef struct _xpmHashAtom {
  char *name;
  void *data;
} *xpmHashAtom;

typedef struct xpmHashTable_t {
  int size;
  int limit;
  int used;
  xpmHashAtom *atomTable;
} xpmHashTable;

/* ========================
 *     export functions
 * ======================== */

extern bool  get_xpm_data(unsigned char *filename, int *width, int *height,
			  int *x_hotspot, int *y_hotspot, unsigned char **data);

#endif
