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

/* ========================
 *     static functions
 * ======================== */

#include "kwxpm_hash.c"

static bool
parse_xpm_file_header(xpmData *data)
{
  char buf[BUFSIZ];
  int  l = 0;
  
  data->Bos = '\0';
  data->Eos = '\n';
  
  l = get_xpm_file_next_word(data, buf, BUFSIZ);
  /* skip the first word, get the second one, and see if this is XPM version 3 */
  l = get_xpm_file_next_word(data, buf, BUFSIZ);
  
  /* strncmp() function is the similar to `strcmp', except that no more than
   * 3rd argument wide characters are compared.  In other words, if the two
   * strings are the same in their first 3rd argument wide characters, the
   * return value is zero.
   */
  if (l != 3 || (strncmp("XPM", buf, 3) != 0)) {
    /* this is not an XPM version 3 file */
    LOG(("%s\n", "<Xpm Error> KiWin xpm library now _only_ support XPM version 3 file."));
    return false;
  }
  
  data->Bos  = '"';
  data->Eos  = '\0';
  /* get to the beginning of the first string */
  goto_xpm_file_next_string_start(data);
  data->Eos  = '"';
  
  return true;
}

static bool
parse_xpm_file_values(xpmData *data,
		      unsigned int *width, unsigned int *height, unsigned int *ncolors, unsigned int *cpp,
		      unsigned int *x_hotspot, unsigned int *y_hotspot)
{
  unsigned int l;
  char buf[BUFSIZ];
  
  /* read values: width, height, ncolors, chars_per_pixel. */
  if ((get_xpm_file_next_unsigned_int(data, width) == false) ||
      (get_xpm_file_next_unsigned_int(data, height) == false) ||
      (get_xpm_file_next_unsigned_int(data, ncolors) == false) ||
      (get_xpm_file_next_unsigned_int(data, cpp) == false)) {
    return false;
  }
  
  /* read optional information (hotspot) if any. */
  l = get_xpm_file_next_word(data, buf, BUFSIZ);
  if (l != 0) {
    atoui(buf, l, x_hotspot);
    get_xpm_file_next_unsigned_int(data, y_hotspot);
  } else {
    x_hotspot = 0;
    y_hotspot = 0;
  }
  
  return true;
}

/* Free the computed color table. */
static void
free_xpm_color_table(XpmColor *colorTable, int ncolors)
{
  int a, b;
  XpmColor *color;
  char **sptr;
  
  if (colorTable != NULL) {
    for (a = 0, color = colorTable; a < ncolors; a++, color++) {
      for (b = 0, sptr = (char **) color; b <= NKEYS; b++, sptr++) {
	if (*sptr != NULL) {
	  free(*sptr);
	}
      }
    }
    
    free(colorTable);
  }
}

static bool
parse_xpm_file_colors(xpmData *data, unsigned int ncolors, unsigned int cpp,
		      XpmColor **colorTablePtr, xpmHashTable *hashtable)
{
  unsigned int   key, l, a, b;
  unsigned int   curkey;            /* current color key */
  bool           lastwaskey;        /* key read          */
  char           buf[BUFSIZ];
  char           curbuf[BUFSIZ];    /* current buffer    */
  char         **sptr = NULL;
  char          *s = NULL;
  XpmColor      *color = NULL;
  XpmColor      *colorTable = NULL;
  char         **defaults = NULL;
  bool           result;
  
  colorTable = (XpmColor *)calloc(ncolors, sizeof(XpmColor));
  if (colorTable == NULL) {
    LOG(("%s\n", "<Xpm Error> malloc() failed."));
    return false;
  }
  
  for (a = 0, color = colorTable; a < ncolors; a++, color++) {
    goto_xpm_file_next_string_start(data);  /* skip the line */
    
    /* read pixel value */
    color->string = (char *)malloc(cpp + 1); /* +1 due to the last NULL terminator */
    if (color->string == NULL) {
      free_xpm_color_table(colorTable, ncolors);
      return false;
    }
    
    for (b = 0, s = color->string; b < cpp; b++, s++) {
      /* from glibc info:
       * ----------------
       * getc() reads the next character as an `unsigned char' from
       * the argument stream and returns its value, converted to an `int'.
       * If an end-of-file condition or read error occurs, `EOF' is
       * returned instead.
       */
      *s = getc(data->file);
    }
    *s = '\0';
    
    /* store the string in the hashtable with its color index number */
    if (USE_HASHTABLE) {
      result = xpmHashIntern(hashtable, color->string, (void *)a);
      if (result == false) {
	free_xpm_color_table(colorTable, ncolors);
	return false;
      }
    }
    
    /* read color keys and values */
    defaults = (char **)color;
    curkey = 0;
    lastwaskey = false;
    *curbuf = '\0';  /* init curbuf */
    
    while ((l = get_xpm_file_next_word(data, buf, BUFSIZ)) != 0) {
      if (lastwaskey == false) {
	for (key = 0, sptr = xpmColorKeys; key < NKEYS; key++, sptr++) {
	  if ((strlen(*sptr) == l) && ((strncmp(*sptr, buf, l)) == 0)) {
	    break;
	  }
	}
      }
      
      if ((lastwaskey == false) && (key < NKEYS)) {  /* this means that we are openning a new key */
	if (curkey != 0) {  /* flush string */
	  if (defaults[curkey] != NULL) {
	    free(defaults[curkey]);
	  }
	  
	  defaults[curkey] = (char *)malloc(strlen(curbuf) + 1);
	  if (defaults[curkey] == NULL) {
	    free_xpm_color_table(colorTable, ncolors);
	    return false;
	  }
	  
	  strcpy(defaults[curkey], curbuf);
	}
	
	curkey = key + 1;   /* set new key  */
	*curbuf = '\0';     /* reset curbuf */
	lastwaskey = true;
      } else {
	if (curkey == 0) {  /* key without value */
	  free_xpm_color_table(colorTable, ncolors);
	  return false;
	}
	
	if (lastwaskey == false) {
	  strcat(curbuf, " ");	/* append space */
	}
	
	buf[l] = '\0';
	strcat(curbuf, buf);  /* append buf */
	lastwaskey = false;
      }
    }
    
    if (curkey == 0) {  /* key without value */
      free_xpm_color_table(colorTable, ncolors);
      return false;
    }
    
    if (defaults[curkey] != NULL) {
      free(defaults[curkey]);
    }
    defaults[curkey] = (char *)malloc(strlen(curbuf) + 1);
    if (defaults[curkey] == NULL) {
      free_xpm_color_table(colorTable, ncolors);
      return false;
    }
    
    strcpy(defaults[curkey], curbuf);
  }
  
  *colorTablePtr = colorTable;
  return true;
}

static int
parse_xpm_file_pixels(xpmData *data,
		      unsigned int width, unsigned int height, unsigned int ncolors, unsigned int cpp,
		      XpmColor *colorTable, xpmHashTable *hashtable, unsigned int **pixels)
{
  unsigned int *iptr = NULL;
  unsigned int *iptr2 = NULL;
  unsigned int  a, x, y;
  
  iptr2 = (unsigned int *)malloc(sizeof(unsigned int) * width * height);
  if (iptr2 == NULL) {
    LOG(("%s\n", "<Xpm Error> malloc() failed."));
    return false;
  }
  iptr = iptr2;
  
  switch (cpp) {
  case 1:  /* Optimize for single character colors */
    {
      unsigned short colidx[256];
      unsigned int   colval[256];
      
      bzero((char *)colidx, 256 * sizeof(short));
      
      for (a = 0; a < ncolors; a++) {
	if (colorTable[a].c_color != NULL) {
	  if (colorTable[a].c_color[0] == '#') {
	    unsigned int red, green, blue, color = 0;
	    
	    atoui(&colorTable[a].c_color[1], 2, &red);
	    atoui(&colorTable[a].c_color[3], 2, &green);
	    atoui(&colorTable[a].c_color[5], 2, &blue);
	    
	    color = CREATE_KWCOLOR(0xff, red, green, blue);
	    
	    colval[(unsigned char)colorTable[a].string[0]] = color;
	  } else {
	    if ((strlen(colorTable[a].c_color) == 4) &&
		((strcmp(colorTable[a].c_color, "None") == 0) ||
		 (strcmp(colorTable[a].c_color, "none") == 0))) {
	      colval[(unsigned char)colorTable[a].string[0]] = 0;
	    } else {
	      colval[(unsigned char)colorTable[a].string[0]] = find_x_color(colorTable[a].c_color);
	    }
	  }
	  
	  colidx[(unsigned char)colorTable[a].string[0]] = a + 1;
	} else {
	  LOG(("%s\n", "<Xpm Error> KiWin xpm library now _only_ support XPM version 3 file."));
	  free(iptr2);
	  return false;
	}
      }
      
      for (y = 0; y < height; y++) {
	goto_xpm_file_next_string_start(data);
	for (x = 0; x < width; x++, iptr++) {
	  int c = getc(data->file);
	  
	  if (c > 0 && c < 256 && colidx[c] != 0) {
	    /* the reason why there is a minus 1 here are there is a plus 1 upthere is that
	     * we use colidx[c] != 0 as a condition statement.
	     */
	    *iptr = colval[c];
	  } else {
	    /* there is at least one pixel which have no color attributes corresponding to it,
	     * so this is not a correct XPM version 3 image file,
	     * free the local malloc resources, and just return.
	     */
	    free(iptr2);
	    return false;
	  }
	}
      }
    }
    break;
    
  case (2):  /* Optimize for double character colors */
    {
      /* free all allocated pointers at all exits */
      /* array of pointers malloced by need */
      unsigned short *cidx[256];
      int char1;
      
      bzero((char *)cidx, 256 * sizeof(unsigned short *)); /* init */
      for (a = 0; a < ncolors; a++) {
	char1 = colorTable[a].string[0];
	
	if (cidx[char1] == NULL) { /* get new memory */
	  cidx[char1] = (unsigned short *)calloc(256, sizeof(unsigned short));
	  if (cidx[char1] == NULL) { /* new block failed */
	    int f;
	    
	    for (f = 0; f < 256; f++) {
	      if (cidx[f]) {
		free(cidx[f]);
	      }
	    }
	    free(iptr2);
	    return false;
	  }
	}
	cidx[char1][(unsigned char)colorTable[a].string[1]] = a + 1;
      }
      
      for (y = 0; y < height; y++) {
	goto_xpm_file_next_string_start(data);
	
	for (x = 0; x < width; x++, iptr++) {
	  int cc1 = getc(data->file);
	  if (cc1 > 0 && cc1 < 256) {
	    int cc2 = getc(data->file);
	    if (cc2 > 0 && cc2 < 256 &&
		cidx[cc1] && cidx[cc1][cc2] != 0) {
	      *iptr = cidx[cc1][cc2] - 1;
	    } else {
	      int f;
	      for (f = 0; f < 256; f++) {
		if (cidx[f]) {
		  free(cidx[f]);
		}
	      }
	      free(iptr2);
	      return false;
	    }
	  } else {
	    {
	      int f;
	      for (f = 0; f < 256; f++) {
		if (cidx[f]) {
		  free(cidx[f]);
		}
	      }
	    }
	    free(iptr2);
	    return false;
	  }
	}
      }
      {
	int f;
	for (f = 0; f < 256; f++) {
	  if (cidx[f]) {
	    free(cidx[f]);
	  }
	}
      }
    }
    break;
    
  default:  /* Non-optimized case of long color names */
    {
      char *s;
      char buf[BUFSIZ];
      
      buf[cpp] = '\0';
      if (USE_HASHTABLE) {
	xpmHashAtom *slot;
	
	for (y = 0; y < height; y++) {
	  goto_xpm_file_next_string_start(data);
	  
	  for (x = 0; x < width; x++, iptr++) {
	    for (a = 0, s = buf; a < cpp; a++, s++) {
	      *s = getc(data->file);
	    }
	    
	    slot = xpmHashSlot(hashtable, buf);
	    if (!*slot) {	/* no color matches */
	      free(iptr2);
	      return false;
	    }
	    *iptr = (unsigned int)((*slot)->data);
	  }
	}
      } else {
	for (y = 0; y < height; y++) {
	  goto_xpm_file_next_string_start(data);
	  
	  for (x = 0; x < width; x++, iptr++) {
	    for (a = 0, s = buf; a < cpp; a++, s++) {
	      *s = getc(data->file);
	    }
	    
	    for (a = 0; a < ncolors; a++) {
	      if (!strcmp(colorTable[a].string, buf)) {
		break;
	      }
	    }
	    
	    if (a == ncolors) {	/* no color matches */
	      free(iptr2);
	      return false;
	    }
	    *iptr = a;
	  }
	}
      }
    }
    break;
  }
  
  *pixels = iptr2;
  return true;
}

/* This function parses an Xpm file and store the found informations
 * in an an KWImageData structure which is returned.
 */
static bool
parse_xpm_file_content(xpmData *mdata, int *width_ptr, int *height_ptr,
		       int *x_hotspot_ptr, int *y_hotspot_ptr, unsigned char **data)
{
  /* variables to return */
  unsigned int   width, height, ncolors, cpp;
  unsigned int   x_hotspot, y_hotspot;
  XpmColor      *colorTable = NULL;
  unsigned int  *pixelindex = NULL;
  xpmHashTable   hashtable;
  bool           result;
  
  /* parse the header. */
  if (parse_xpm_file_header(mdata) == false) {
    return false;
  }
  
  /* read values */
  if (parse_xpm_file_values(mdata, &width, &height, &ncolors, &cpp,
			    &x_hotspot, &y_hotspot) == false) {
    return false;
  }
  
  /* init the hastable */
  if (USE_HASHTABLE) {
    result = xpmHashTableInit(&hashtable);
    if (result == false) {
      return false;
    }
  }
  
  /* read colors */
  result = parse_xpm_file_colors(mdata, ncolors, cpp, &colorTable, &hashtable);
  if (result == false) {
    if (USE_HASHTABLE) {
      xpmHashTableFree(&hashtable);
    }
    if (colorTable != NULL) {
      free_xpm_color_table(colorTable, ncolors);
    }
    
    return false;
  }
  
  /* read pixels and index them on color number. */
  result = parse_xpm_file_pixels(mdata, width, height, ncolors, cpp, colorTable,
				 &hashtable, &pixelindex);
  
  /* free the hastable */
  if (USE_HASHTABLE) {
    xpmHashTableFree(&hashtable);
  }
  /* free the colortable */
  if (colorTable != NULL) {
    free_xpm_color_table(colorTable, ncolors);
  }
  if (result == false) {
    if (pixelindex != NULL) {
      free(pixelindex);
    }
    
    return false;
  }
  
  *width_ptr     = width;
  *height_ptr    = height;
  *data          = (unsigned char *)pixelindex;
  *x_hotspot_ptr = x_hotspot;
  *y_hotspot_ptr = y_hotspot;
  
  return true;
}

static void
parse_xpm_file_comment(xpmData *data)
{
  FILE *file = data->file;
  register int c;
  bool notend;
  
  /* skip the string beginning comment */
  c = getc(file);
  if (c != '*') {
    /* this wasn't the beginning of a comment,
     * put characters back.
     */
    ungetc(c, file);
    
    return;
  }
  
  notend = true;
  c = getc(file);
  while (notend == true) {
    while (c != '*' && c != EOF) {
      c = getc(file);
    }
    
    c = getc(file);
    if (c == '/') {
      /* this is the end of the comment */
      notend = false;
    }
  }
  
  return;
}
