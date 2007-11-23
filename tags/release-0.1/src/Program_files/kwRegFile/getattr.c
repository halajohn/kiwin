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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#include <kwlog/kwlog.h>
#include <gdbm/gdbm.h>

/* ========================
 *     static variables
 * ======================== */

static gdbm_file_info *reg_file = NULL;

/* ========================
 *     static functions
 * ======================== */

static void
usage(char *s)
{
  LOG(("Usage: %s [-s file.reg]\n", s));
  
  exit (-1);
}

static void
print_bucket(hash_bucket *bucket, char *mesg)
{
  int  index;
  int  i = 0;
  
  printf("********** %s **********\n\n", mesg);
  printf("bucket bits = %d\n", bucket->bucket_bits);
  printf("have %d elements\n", reg_file->header->bucket_elems);
  printf("use %d elements\n\n", bucket->count);
  printf("Hash Table:\n");
  printf("     #    hash value     key size    data size     data adr  home\n");
  
  for (index = 0; index < reg_file->header->bucket_elems; index++) {
    if ((i != 0) && (i % 30) == 0) {
      printf("\npress enter to continue\n");
      while (getchar() != '\n') ;
    }
    
    printf("  %4d  %12x  %11d  %11d  %11d %5d\n", index,
	   bucket->h_table[index].hash_value,
	   bucket->h_table[index].key_size,
	   bucket->h_table[index].data_size,
	   (int)bucket->h_table[index].data_pointer,
	   bucket->h_table[index].hash_value % reg_file->header->bucket_elems);
    i++;
  }
  
  printf("\nAvail count = %1d\n", bucket->av_count);
  printf("Avail  adr     size\n");
  for (index = 0; index < bucket->av_count; index++) {
    printf("%9d%9d\n", (int)bucket->bucket_avail[index].av_adr,
	   bucket->bucket_avail[index].av_size);
  }
}

static void
gdbm_print_avail_list(gdbm_file_info *dbf)
{
  int temp;
  int size;
  avail_block *av_stk;
  
  /* Print the the header avail block.  */
  printf ("\nheader avail block:\nhave %d avail elems\nuse %d avail elems\n",
	  dbf->header->avail.size, dbf->header->avail.count);
  for (temp = 0; temp < dbf->header->avail.count; temp++) {
    printf ("  %15d   %10d \n", dbf->header->avail.av_table[temp].av_size,
	    (int)dbf->header->avail.av_table[temp].av_adr);
  }
  
  /* Initialize the variables for a pass throught the avail stack. */
  temp = dbf->header->avail.next_block;
  size = (((dbf->header->avail.size * sizeof (avail_elem)) >> 1) + sizeof (avail_block));
  av_stk = (avail_block *)malloc(size);
  if (av_stk == NULL) {
    printf("Out of memory\n");
    exit (2);
  }
  
  /* Print the stack. */
  while (false) {
    lseek(dbf->desc, temp, L_SET);
    read(dbf->desc, av_stk, size);
    
    /* Print the block! */
    printf("\nblock = %d\nsize  = %d\ncount = %d\n", temp,
	   av_stk->size, av_stk->count);
    for (temp = 0; temp < av_stk->count; temp++) {
      printf ("  %15d   %10d \n", av_stk->av_table[temp].av_size,
	      (int)av_stk->av_table[temp].av_adr);
    }
    temp = av_stk->next_block;
  }
}

/* ========================
 *     export functions
 * ======================== */

int
main(int argc, char **argv)
{
  char   *reg_file_name = NULL;
  int     opt;
  char    cmd_ch;
  datum   key_data;
  datum   return_data;
  bool    done = false;
  char    key_line[500];
  int     len;
  
  if (argc < 3) {
    usage(argv[0]);
  }
  
  opterr = 0;
  while ((opt = getopt(argc, argv, "s:")) != -1) {
    switch (opt) {
    case 's':
      reg_file_name = (char *)strdup(optarg);
      break;
      
    default:
      usage(argv[0]);
    }
  }
  
  reg_file = gdbm_open(reg_file_name, 0, GDBM_WRITER, 00664, NULL);
  if (reg_file == NULL) {
    LOG(("%s\n", "gdbm_open() failed."));
    exit (-1);
  }
  
  while (done == false) {
    printf("command -> ");
    cmd_ch = getchar();
    if (cmd_ch != '\n') {
      char temp;
      
      do {
	temp = getchar();
      } while (temp != '\n' && temp != EOF);
    }
    
    if (cmd_ch == EOF) {
      cmd_ch = 'q';
    }
    
    switch (cmd_ch) {
    case '\n':
      printf("\n");
      break;
      
    case 'c':
      {
	int count = 0;
	
	if (key_data.dptr != NULL) {
	  free(key_data.dptr);
	}
	
	return_data = gdbm_firstkey(reg_file);
	while (return_data.dptr != NULL) {
	  count++;
	  key_data = return_data;
	  return_data = gdbm_nextkey(reg_file, key_data);
	  free(key_data.dptr);
	}
	
	printf("There are %d items in the database.\n\n", count);
	key_data.dptr = NULL;
      }
      break;
      
    case 'f':
      if (key_data.dptr != NULL) {
	free(key_data.dptr);
      }
      
      printf ("key -> ");
      fgets(key_line, 500, stdin);
      len = strlen(key_line);
      key_line[len - 1] = '\0';
      
      key_data.dptr = key_line;
      key_data.dsize = strlen(key_line) + 1;
      return_data = gdbm_fetch(reg_file, key_data);
      
      if (return_data.dptr != NULL) {
	printf ("data is ->%s\n\n", return_data.dptr);
	free (return_data.dptr);
      } else {
	printf ("No such item found.\n\n");
      }
      
      key_data.dptr = NULL;
      break;
      
    case 'q':
      done = true;
      break;
      
    case '1':
      if (key_data.dptr != NULL) {
	free(key_data.dptr);
      }
      
      key_data = gdbm_firstkey(reg_file);
      if (key_data.dptr != NULL) {
	printf ("key is  ->%s\n", key_data.dptr);
	return_data = gdbm_fetch(reg_file, key_data);
	printf ("data is ->%s\n\n", return_data.dptr);
	free (return_data.dptr);
      } else {
	printf ("No such item found.\n\n");
      }
      break;
      
    case '2':
      return_data = gdbm_nextkey(reg_file, key_data);
      if (return_data.dptr != NULL) {
	free(key_data.dptr);
	key_data = return_data;
	
	printf("key is  ->%s\n", key_data.dptr);
	return_data = gdbm_fetch(reg_file, key_data);
	printf("data is ->%s\n\n", return_data.dptr);
	free(return_data.dptr);
      } else {
	printf("No such item found.\n\n");
      }
      break;
      
    case 'r':
      if (gdbm_reorganize(reg_file) == -1) {
	printf("Reorganization failed.\n\n");
      } else {
	printf ("Reorganization succeeded.\n\n");
      }
      break;
      
    case 'A':
      gdbm_print_avail_list(reg_file);
      printf("\n");
      break;
      
    case 'B':
      {
	int temp;
	char number[80];
	
	printf("bucket? ");
	fgets(number, 80, stdin);
	sscanf(number, "%d", &temp);
	
	if (temp >= reg_file->header->dir_size / 4) {
	  printf("Not a bucket.\n\n");
	  break;
	}
	_gdbm_get_bucket(reg_file, temp);
	printf("Your bucket is now %d\n\n", temp);
      }
      
    case 'C':
      if (reg_file->bucket != NULL) {
	print_bucket(reg_file->bucket, "Current bucket");
	printf ("\n current directory entry = %d.\n", reg_file->bucket_dir);
	printf (" current bucket address  = %d.\n\n",
		(int)reg_file->cache_entry->ca_adr);
      } else {
	printf("current bucket isn't defined\n");
      }
      break;
      
    case 'D':
      printf("Hash table directory.\n");
      printf("  Size =  %d.  Bits = %d. \n\n", reg_file->header->dir_size,
	     reg_file->header->dir_bits);
      {
	int temp;
	
	for (temp = 0; temp < reg_file->header->dir_size / 4; temp++) {
	  printf ("  %10d:  %12d\n", temp, (int)reg_file->dir[temp]);
	  if ((temp + 1) % 20 == 0 && isatty(0)) {
	    printf("\npress enter to continue\n");
	    while (getchar () != '\n') /* Do nothing. */;
	  }
	}
      }
      printf ("\n");
      break;
      
    case 'F':
      printf ("\nFile Header: \n\n");
      printf ("  table        = %d\n", (int)reg_file->header->dir);
      printf ("  table size   = %d\n", reg_file->header->dir_size);
      printf ("  table bits   = %d\n", reg_file->header->dir_bits);
      printf ("  block size   = %d\n", reg_file->header->block_size);
      printf ("  bucket elems = %d\n", reg_file->header->bucket_elems);
      printf ("  bucket size  = %d\n", reg_file->header->bucket_size);
      printf ("  header magic = %x\n", reg_file->header->header_magic);
      printf ("  next block   = %d\n", (int)reg_file->header->next_block);
      printf ("  avail size   = %d\n", reg_file->header->avail.size);
      printf ("  avail count  = %d\n", reg_file->header->avail.count);
      printf ("  avail nx blk = %d\n", (int)reg_file->header->avail.next_block);
      printf ("\n");
      break;
      
    case '?':
      printf("c - count (number of entries)\n");
      printf("f - fetch\n");
      printf("q - quit\n");
      printf("1 - firstkey\n");
      printf("2 - nextkey on last key (from n, 1 or 2)\n\n");
      
      printf("r - reorganize\n");
      printf("A - print avail list\n");
      printf("B - get and print current bucket n\n");
      printf("C - print current bucket\n");
      printf("D - print hash directory\n");
      printf("F - print file header\n");
      break;
      
    default:
      printf ("what? \n\n");
      break;
    }
  }
  
  exit (0);
}
