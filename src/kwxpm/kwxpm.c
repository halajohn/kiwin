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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <kwlog/kwlog.h>
#include <kwcommon/kwcommon.h>
#include <kwxpm/kwxpm.h>
#include <kwxpm/kwxcolor.h>
#include <kwcolor/kwcolor.h>

/* ========================
 *     static variables
 * ======================== */

static char *xpmColorKeys[] = {
  "s",	/* key #1: symbol         */
  "m",	/* key #2: mono visual    */
  "g4",	/* key #3: 4 grays visual */
  "g",	/* key #4: gray visual    */
  "c",	/* key #5: color visual   */
};

/* ========================
 *     static functions
 * ======================== */

static void  parse_xpm_file_comment(xpmData *data);

static bool
atoui(register char *p, unsigned int l, unsigned int *ui_return)
{
  register unsigned int n = 0;
  register unsigned int i = 0;
  
  for (i = 0; i < l; i++) {
    if (*p >= '0' && *p <= '9') {
      n = n * 10 + *p++ - '0';
    } else {
      break;
    }
  }
  
  if (i != 0 && i == l) {
    *ui_return = n;
    return true;
  } else {
    return false;
  }
}

/* skip whitespace and return the following word. */
static unsigned int
get_xpm_file_next_word(xpmData *data, char *buf, unsigned int buflen)
{
  register unsigned int n = 0;
  int   c;
  FILE *file = data->file;
  
  /* using to skip whitespace */
  while ((c = getc(file)) != EOF && isspace(c));
  
  while (!isspace(c) && c != data->Eos && c != EOF && n < buflen) {
    *buf++ = c;
    n++;
    c = getc(file);
  }
  
  /* from glibc info:
   * If c is `EOF', `ungetc' does nothing and just returns `EOF'.  This
   * lets you call `ungetc' with the return value of `getc' without
   * needing to check for an error from `getc'.
   */
  ungetc(c, file);
  
  return (n);
}

/* skip to the end of the current string and the beginning of the next one. */
int
goto_xpm_file_next_string_start(xpmData *data)
{
  register int c;
  FILE *file = data->file;
  
  /* get to the end of the current string */
  if (data->Eos != '\0') {
    while ((c = getc(file)) != data->Eos && c != EOF);
  }
  
  /* then get to the beginning of the next string looking for possible
   * comment.
   */
  if (data->Bos != '\0') {
    while ((c = getc(file)) != data->Bos && c != EOF) {
      if (c == '/') {
	parse_xpm_file_comment(data);
      }
    }
  }
  
  return 0;
}

/* skip whitespace and compute the following unsigned int,
 * returns true if one is found and false if not
 */
static bool
get_xpm_file_next_unsigned_int(xpmData *data, unsigned int *ui_return)
{
  char buf[BUFSIZ];
  int l;
  
  l = get_xpm_file_next_word(data, buf, BUFSIZ);
  return atoui(buf, l, ui_return);
}

#include "kwxpm_parse.c"

/* ========================
 *     export functions
 * ======================== */

bool
get_xpm_data(unsigned char *filename, int *width, int *height, int *x_hotspot, int *y_hotspot, unsigned char **data)
{
  xpmData  mdata;
  bool     result;
  FILE    *fp = NULL;
  
  if (filename == NULL) {
    LOG(("%s\n", "<Xpm Error> the image file name == NULL."));
    
    *width = 0;
    *height = 0;
    *x_hotspot = 0;
    *y_hotspot = 0;
    *data = NULL;
    
    return false;
  }
  
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    LOG(("%s\n", "<Xpm Error> can not open the specified xpm image file."));
    
    *width = 0;
    *height = 0;
    *x_hotspot = 0;
    *y_hotspot = 0;
    *data = NULL;
    
    return false;
  }
  
  mdata.file = fp;
  
  /* create the KWImageData from the XpmData */
  result = parse_xpm_file_content(&mdata, width, height, x_hotspot, y_hotspot, data);
  
  fclose(fp);
  
  return result;
}
