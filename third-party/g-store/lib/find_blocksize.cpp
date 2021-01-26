#include "find_blocksize.h"

#include "defs.h"
#include "util.h"
#include "block.h"
#include "parameters.h"

/* Askes the user for a block size in the storage algorithm */
//#include <g-store.h>

void determine_blocksize(bool display_prompt)
{
  DWORD sect_per_clust, bytes_per_sect, free_clust, total_clust;
  char drive[4];
  strncpy(drive,PARAM::working_dir,3);
  drive[3] = '\0';

  if (*(drive+1) != ':')
    error_exit("Parameter PARAM::working_dir must contain an absolute path to "
    "the working directory\nFor instance, 'C:/g-store/'");

  GetDiskFreeSpace (drive, &sect_per_clust, &bytes_per_sect, &free_clust, &total_clust);

  int sect_per_clust_int = int(sect_per_clust);
  int bytes_per_sect_int = int(bytes_per_sect);

  GLOBALS::blk_size = sect_per_clust_int * bytes_per_sect_int * PARAM::default_fsblks_per_page;

  //if (GLOBALS::blk_size > 65536)
  //  GLOBALS::blk_size = 65536;

  if (display_prompt && confirm_yn("Do you want to change the default page size (%d KB)?", GLOBALS::blk_size/1024))
  {
    print_ln("A sector in drive %s is %d bytes and a file system block is %d KB.",
      drive, bytes_per_sect_int, bytes_per_sect_int * sect_per_clust_int / 1024);
    
    for(;;)
    {
      print_ln("Please enter a page size in KB:");

      GLOBALS::blk_size = read_int()*1024;

      if (GLOBALS::blk_size <= 0 || GLOBALS::blk_size % bytes_per_sect_int != 0 ||  GLOBALS::blk_size > 16777216)
      {
        print_ln("Page size must be a positive multiple of sector size (%d bytes), and not larger than 16384 KB.", bytes_per_sect_int);
        continue;
      }

      if (GLOBALS::blk_size % (bytes_per_sect_int * sect_per_clust_int) != 0)
      {
        print_ln("In general, it is recommended to choose a multiple of file system block size.");
        if (confirm_yn("Do you want to use %d KB?", GLOBALS::blk_size/1024))
          break;
      }
      else
        break;
    }
  }
  
  if (GLOBALS::blk_size > 65536)
  {
    GLOBALS::header_slt_len = 3;
    GLOBALS::blk_writable = GLOBALS::blk_size - (4 + 3 + 2 + 2); //preliminary 
    get_header = get_header3;
    set_header = set_header3;
    inc_header = inc_header3;
  }
  else
  {
    GLOBALS::header_slt_len = 2;
    GLOBALS::blk_writable = GLOBALS::blk_size - (4 + 2 + 2 + 2); //preliminary 
    get_header = get_header2;
    set_header = set_header2;
    inc_header = inc_header2;
  }
}
