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
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

#include <kwlog/kwlog.h>
#include <gdbm/gdbm.h>

/* ========================
 *     static functions
 * ======================== */

static void
usage(char *s)
{
  LOG(("Usage: %s [-s file.txt]\n", s));
  
  exit (-1);
}

static unsigned int
get_src_file_next_word(FILE *fp, char *buf, unsigned int buflen)
{
  register unsigned int n = 0;
  int   c;
  
  while (((c = getc(fp)) != EOF) && ((c == '\n') || isspace(c)));
  
  while (!isspace(c) && c != '\n' && c != EOF && n < buflen) {
    *buf++ = c;
    n++;
    c = getc(fp);
  }
  
  ungetc(c, fp);
  
  return (n);
}

/* ========================
 *     export functions
 * ======================== */

int
main(int argc, char **argv)
{
  char *src_file_name = NULL;
  char *reg_file_name = NULL;
  FILE *src_file = NULL;
  gdbm_file_info *reg_file;
  int opt;
  int len;
  
  datum key_data;
  datum data_data;
  
#define KEY_LINE_LEN 100
#define DATA_LINE_LEN 100
  
  char key_line[KEY_LINE_LEN] = {0,};
  char data_line[DATA_LINE_LEN] = {0,};
  
  if (argc < 3) {
    usage(argv[0]);
  }
  
  opterr = 0;
  while ((opt = getopt(argc, argv, "s:d:")) != -1) {
    switch (opt) {
    case 's':
      src_file_name = (char *)strdup(optarg);
      break;
      
    case 'd':
      reg_file_name = (char *)strdup(optarg);
      break;
      
    default:
      usage(argv[0]);
    }
  }
  
  src_file = fopen(src_file_name, "r");
  
  reg_file = gdbm_open(reg_file_name, 0, GDBM_NEWDB, 00664, NULL);
  if (reg_file == NULL) {
    LOG(("%s\n", "gdbm_open() failed."));
    exit (-1);
  }
  
  key_data.dptr = key_line;
  data_data.dptr = data_line;
  
  while (1) {
    len = get_src_file_next_word(src_file, key_line, KEY_LINE_LEN);
    if (len == 0) {
      break;
    }
    key_line[len] = '\0';
    
    key_data.dsize = strlen(key_line) + 1;
    
    len = get_src_file_next_word(src_file, data_line, DATA_LINE_LEN);
    data_line[len] = '\0';
    data_data.dsize = strlen(data_line) + 1;
    
    if (gdbm_store(reg_file, key_data, data_data, GDBM_INSERT) != 0) {
      LOG(("%s\n", "item not inserted."));
    }
  }
  
  if (gdbm_reorganize(reg_file) == -1) {
    LOG(("%s\n", "Reorganization failed."));
  }
  gdbm_close(reg_file);
  
  return 0;
}
