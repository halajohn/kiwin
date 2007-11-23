#ifndef _KWGDBM_H_
#define _KWGDBM_H_

#include <fcntl.h>
#include <sys/file.h>

/* ==============
 *     macros
 * ============== */

#define  BUCKET_AVAIL   6
#define  SMALL          4

#define  GDBM_READER    0
#define  GDBM_WRITER    1
#define  GDBM_WRCREAT   2
#define  GDBM_NEWDB     3
#define  GDBM_OPENMASK  7

#define  GDBM_INSERT    0
#define  GDBM_REPLACE   1

/* ===============
 *     typedef
 * =============== */

typedef struct {
  char *dptr;
  int   dsize;
} datum;

typedef struct {
  int    av_size;
  off_t  av_adr;
} avail_elem;

typedef struct {
  int   size;
  int   count;
  off_t next_block;
  avail_elem av_table[1];
} avail_block;

typedef struct {
  int   header_magic;
  int   block_size;
  off_t dir;
  int   dir_size;
  int   dir_bits;
  int   bucket_size;
  int   bucket_elems;
  off_t next_block;
  avail_block avail;
} gdbm_file_header;

typedef struct {
  int   hash_value;
  char  key_start[SMALL];
  off_t data_pointer;
  int   key_size;
  int   data_size;
} bucket_element;

typedef struct {
  int             av_count;
  avail_elem      bucket_avail[BUCKET_AVAIL];
  int             bucket_bits;
  int             count;
  bucket_element  h_table[1];
} hash_bucket;

typedef struct {
  int   hash_val;
  int   data_size;
  int   key_size;
  char *dptr;
  int   elem_loc;
} data_cache_elem;

typedef struct {
  hash_bucket     *ca_bucket;
  off_t            ca_adr;
  char		   ca_changed;
  data_cache_elem  ca_data;
} cache_elem;

typedef struct {
  char *name;
  int read_write;
  int fast_write;
  int central_free;
  int coalesce_blocks;
  int file_locking;
  void (*fatal_err) ();
  int desc;
  gdbm_file_header *header;
  off_t *dir;
  cache_elem *bucket_cache;
  int cache_size;
  int last_read;
  hash_bucket *bucket;
  int bucket_dir;
  cache_elem *cache_entry;
  char  header_changed;
  char  directory_changed;
  char  bucket_changed;
  char  second_changed;
} gdbm_file_info;

/* ========================
 *     export functions
 * ======================== */

extern gdbm_file_info*  gdbm_open(char *file, int block_size, int flags, int mode, void (*fatal_func)());
extern void             gdbm_close(gdbm_file_info *dbf);
extern datum            gdbm_fetch(gdbm_file_info *dbf, datum key);
extern int              gdbm_delete(gdbm_file_info *dbf, datum key);
extern int              gdbm_exists(gdbm_file_info *dbf, datum key);
extern int              gdbm_reorganize(gdbm_file_info *dbf);
extern datum            gdbm_firstkey(gdbm_file_info *dbf);
extern datum            gdbm_nextkey(gdbm_file_info *dbf, datum key);
extern int              gdbm_setopt(gdbm_file_info *dbf, int optflag, int *optval, int optlen);
extern int              gdbm_store(gdbm_file_info *dbf, datum key, datum content, int flags);
extern void             gdbm_sync(gdbm_file_info *dbf);

extern void             _gdbm_get_bucket(gdbm_file_info *dbf, int dir_index);

#endif
