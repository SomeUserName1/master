#include "memory_mgr.h"

#include "util_t.h"
#include "util.h"
#include "read_query.h"
#include "parameters.h"

/* Access to the raw disk representation */
//#include <g-store.h>

int (*get_lower_idx)(b_id key, b_id* x, int num_elem);

void mem_create_disks_blks(int num_blocks, int8 max_disk_size_b /*= ((int8) 1 << 32)-1*/)
{
  mem_create_disks(int8(num_blocks) * int8(GLOBALS::blk_size), max_disk_size_b);
};

void mem_create_disks_g(double total_space_g, double max_disk_size_g /*= 4.0*/)
{
  const int8 gb = ((int8) 1 << 30);
  mem_create_disks(int8(total_space_g*gb), int8(max_disk_size_g*gb-1));
};

void mem_create_disks_m(double total_space_m, double max_disk_size_m /*= 4096.0*/)
{
  const int8 mb = ((int8) 1 << 20);
  mem_create_disks(int8(total_space_m*mb), int8(max_disk_size_m*mb-1));
};

void mem_create_disks(int8 space_b, int8 max_disk_size_b /*= ((int8) 1 << 32)-1*/)
{
  GLOBALS::num_of_disks = uint2(space_b / max_disk_size_b);

  int overlap_blks = int((space_b % max_disk_size_b) / GLOBALS::blk_size);
  
  if (overlap_blks > 0) 
    GLOBALS::num_of_disks += 1;
  
  GLOBALS::disk_files_     = new char[GLOBALS::num_of_disks * strlen(PARAM::working_dir) + 20];
  GLOBALS::disk_files      = new char*[GLOBALS::num_of_disks];
  GLOBALS::disk_sizes      = new int8[GLOBALS::num_of_disks];
  GLOBALS::blocks_in_disk  = new int[GLOBALS::num_of_disks];
  GLOBALS::disk_first_bid  = new b_id[GLOBALS::num_of_disks];
  GLOBALS::disk_fps        = new void*[GLOBALS::num_of_disks];
  //GLOBALS::disk_fps      = new FILE*[NUM_OF_DISKS];
  GLOBALS::cnt_db_size = 0;
  GLOBALS::cnt_blks = 0;

  for (int i = 0; i < GLOBALS::num_of_disks; i++)
  {
    GLOBALS::disk_files[i] = GLOBALS::disk_files_ + (strlen(PARAM::working_dir) + 20) * i;
    sprintf(GLOBALS::disk_files[i], "%s_disk%d.g", PARAM::working_dir, i);
    GLOBALS::blocks_in_disk[i] = 
      (i == GLOBALS::num_of_disks-1 && overlap_blks > 0) 
      ? overlap_blks 
      : int(max_disk_size_b/GLOBALS::blk_size);
    GLOBALS::disk_first_bid[i] = GLOBALS::cnt_blks;
    GLOBALS::cnt_blks += GLOBALS::blocks_in_disk[i];
    GLOBALS::disk_sizes[i] = int8(GLOBALS::blocks_in_disk[i]) * int8(GLOBALS::blk_size);
    GLOBALS::cnt_db_size += GLOBALS::disk_sizes[i];
                                                            //todo: may warn before overwrite;
    GLOBALS::disk_fps[i] = CreateFile(GLOBALS::disk_files[i], (GENERIC_WRITE | GENERIC_READ), FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING, NULL);
    //GLOBALS::disk_fps[i] = fopen(GLOBALS::disk_files[i], "wb+");
    if (GLOBALS::disk_fps[i] == INVALID_HANDLE_VALUE) 
    //if (GLOBALS::disk_fps[i] == NULL) 
      error_exit("cannot create disk file: %s", GLOBALS::disk_files[i]);
    
    if (SetFilePointerEx(GLOBALS::disk_fps[i], to_li(GLOBALS::disk_sizes[i]), NULL, FILE_BEGIN) == false)
    //if (_fseeki64(GLOBALS::disk_fps[i], DISK_SIZES[i], SEEK_SET) < 0)
      error_exit("cannot seek to disk file end: %s", GLOBALS::disk_files[i]);
    if (SetEndOfFile(GLOBALS::disk_fps[i]) == false) 
    //if (fwrite('\0', 1, 1, GLOBALS::disk_fps[i]) < 0) 
      error_exit("cannot set end of file: %s - disk full?", GLOBALS::disk_files[i]);
  }

  if (GLOBALS::num_of_disks == 1) 
    get_lower_idx = get_lower_idx_1;
  else
    get_lower_idx = get_lower_idx_n;

  mem_print_disks_info();
};

void mem_reinitialize()
{
  GLOBALS::disk_files = new char*[GLOBALS::num_of_disks];
  GLOBALS::disk_fps = new void*[GLOBALS::num_of_disks];

  for (int i = 0; i < GLOBALS::num_of_disks; i++)
  {
    GLOBALS::disk_files[i] = GLOBALS::disk_files_ + ((strlen(PARAM::working_dir) + 20) * i);
    GLOBALS::disk_fps[i] = CreateFile(GLOBALS::disk_files[i], (GENERIC_WRITE | GENERIC_READ), FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
    if (GLOBALS::disk_fps[i] == INVALID_HANDLE_VALUE) 
      //if (GLOBALS::disk_fps[i] == NULL) 
      error_exit("cannot open disk file: %s", GLOBALS::disk_files[i]);
  }

  if (GLOBALS::num_of_disks == 1) 
    get_lower_idx = get_lower_idx_1;
  else
    get_lower_idx = get_lower_idx_n;
}

void mem_write_blocks(block1 b, b_id bid, int num)
{
  DWORD tmp = 0;
  int disk = get_lower_idx(bid, GLOBALS::disk_first_bid, GLOBALS::num_of_disks);
  int block_offset = bid - GLOBALS::disk_first_bid[disk];
  //_fseeki64(GLOBALS::disk_fps[disk], int8(block_offset) * int8(BLOCK_SIZE), SEEK_SET);
  SetFilePointerEx(GLOBALS::disk_fps[disk], to_li(int8(block_offset) * GLOBALS::blk_size), NULL, FILE_BEGIN);

  if (num + block_offset <= GLOBALS::blocks_in_disk[disk])
    //fwrite(b, BLOCK_SIZE, num, GLOBALS::disk_fps[disk]);
    WriteFile(GLOBALS::disk_fps[disk], b, GLOBALS::blk_size*num, &tmp, NULL);
  else 
  {
    int write_cnt = GLOBALS::blocks_in_disk[disk] - block_offset;
    //fwrite(b, BLOCK_SIZE, write_cnt, GLOBALS::disk_fps[disk]);
    WriteFile(GLOBALS::disk_fps[disk], b, GLOBALS::blk_size*write_cnt, &tmp, NULL);
    int to_write = num - write_cnt;
    int i = disk + 1;
    while (to_write > 0)
    {
      SetFilePointerEx(GLOBALS::disk_fps[i], to_li(0), NULL, FILE_BEGIN);
      //rewind(GLOBALS::disk_fps[i]);
      WriteFile(GLOBALS::disk_fps[i], b + write_cnt, GLOBALS::blk_size * min1(GLOBALS::blocks_in_disk[i], to_write), &tmp, NULL);
      //fwrite(b + write_cnt, BLOCK_SIZE, std::min(BLOCKS_IN_DISK[i], to_write), GLOBALS::disk_fps[i]);
      to_write -= GLOBALS::blocks_in_disk[i];
      write_cnt += GLOBALS::blocks_in_disk[i];
      i++;
    }
  }
};

//todo: optimize it out for only one disk
void mem_write_block(block1 b, b_id bid)
{
  //if (bid == INVALID_BLOCK_ID)
  DWORD tmp = 0;
  int disk = get_lower_idx(bid, GLOBALS::disk_first_bid, GLOBALS::num_of_disks);
  SetFilePointerEx(GLOBALS::disk_fps[disk], to_li(int8(bid - GLOBALS::disk_first_bid[disk]) * GLOBALS::blk_size), NULL, FILE_BEGIN);
  //b->print_block();
  //_fseeki64(GLOBALS::disk_fps[disk], int8(bid - *it) * int8(BLOCK_SIZE), SEEK_SET);
  WriteFile(GLOBALS::disk_fps[disk], b, GLOBALS::blk_size, &tmp, NULL);
  //fwrite(b, BLOCK_SIZE, 1, GLOBALS::disk_fps[disk]);
};

void  mem_read_blocks(block1 b, b_id bid, int num)
{
  if (PARAM::count_io){
    QUERY::xtra_cnt_IO_single+=num;
    QUERY::xtra_cnt_IO_cons++;
  }
  int disk = get_lower_idx(bid, GLOBALS::disk_first_bid, GLOBALS::num_of_disks);
  int block_offset = bid - GLOBALS::disk_first_bid[disk];
  DWORD tmp = 0;
  HANDLE hFile = GLOBALS::disk_fps[disk];
  LARGE_INTEGER liDistanceToMove;
  liDistanceToMove.QuadPart = 1L * block_offset * GLOBALS::blk_size; // 64bit
  SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN);
  //SetFilePointerEx(GLOBALS::disk_fps[disk], to_li(int8(block_offset) * GLOBALS::blk_size), NULL, FILE_BEGIN);
  //_fseeki64(GLOBALS::disk_fps[disk], int8(block_offset) * int8(BLOCK_SIZE), SEEK_SET);

  if (num + block_offset <= GLOBALS::blocks_in_disk[disk]) 
    ReadFile(GLOBALS::disk_fps[disk], b, GLOBALS::blk_size * num, &tmp, NULL);
    //fread(b, BLOCK_SIZE, num, GLOBALS::disk_fps[disk]);
  else 
  {
    int read_count = GLOBALS::blocks_in_disk[disk] - block_offset;
    ReadFile(GLOBALS::disk_fps[disk], b, GLOBALS::blk_size * read_count, &tmp, NULL);
    //fread(b, BLOCK_SIZE, read_count, GLOBALS::disk_fps[disk]);
    int to_read = num - read_count;
    int i = disk + 1;
    while (to_read>0)
    {
      SetFilePointerEx(GLOBALS::disk_fps[i], to_li(0), NULL, FILE_BEGIN);
      //rewind(GLOBALS::disk_fps[i]);
      ReadFile(GLOBALS::disk_fps[i], b + read_count, GLOBALS::blk_size * min1(GLOBALS::blocks_in_disk[i], to_read), &tmp, NULL);
      //fread(b + read_count, BLOCK_SIZE, std::min(BLOCKS_IN_DISK[i], to_read), GLOBALS::disk_fps[i]);
      to_read -= GLOBALS::blocks_in_disk[i];
      read_count += GLOBALS::blocks_in_disk[i];      //todo:use feor and ferror to determine??
      i++;
    }
  }
};

void mem_read_block(block1 b, b_id bid)
{
  if (PARAM::count_io){
    QUERY::xtra_cnt_IO_single++;
    QUERY::xtra_cnt_IO_cons++;
  }
  int disk = get_lower_idx(bid, GLOBALS::disk_first_bid, GLOBALS::num_of_disks);
  DWORD tmp = 0;
  SetFilePointerEx(GLOBALS::disk_fps[disk], to_li(int8(bid - GLOBALS::disk_first_bid[disk]) * GLOBALS::blk_size), NULL, FILE_BEGIN);
  //_fseeki64(GLOBALS::disk_fps[disk], int8(bid - *it) * int8(BLOCK_SIZE), SEEK_SET);
  ReadFile(GLOBALS::disk_fps[disk], b, GLOBALS::blk_size, &tmp, NULL);
  //fread(b, BLOCK_SIZE, 1, GLOBALS::disk_fps[disk]);
};


void mem_print_disks_info()
{
  print_ln("\nStorage statistics:");
  print_ln("Total disks: %d", GLOBALS::num_of_disks);
  print_ln("Total pages: %u   (%d bytes per page)\n", GLOBALS::cnt_blks, GLOBALS::blk_size);
  print_ln("%-8s %7s %7s %7s  %10s  %s", "disk", "pages", "from", "to", "size (MB)", "file"); 
  print_ln("--------------------------------------------------------------------------");

  for (int i = 0; i < GLOBALS::num_of_disks; i++)
    print_ln("disk%-4d %7d %7d %7d  %10.3f  %s", 
      i, GLOBALS::blocks_in_disk[i], GLOBALS::disk_first_bid[i], GLOBALS::disk_first_bid[i] + GLOBALS::blocks_in_disk[i] - 1, 
      (double(GLOBALS::disk_sizes[i])/1048576), GLOBALS::disk_files[i]);

  print_endln();
};

void mem_close_disks()
{
  for (int i = 0; i < GLOBALS::num_of_disks; i++)
    CloseHandle(GLOBALS::disk_fps[i]);
    //fclose(GLOBALS::disk_fps[i]);
};

void mem_delete_disks()
{
  for (int i = 0; i < GLOBALS::num_of_disks; i++)
    remove(GLOBALS::disk_files[i]);
};
