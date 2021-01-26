#include "read_to_blocks.h"

#include "structs.h"
#include "util_t.h"
#include "util.h"
#include "block.h"
#include "buffer_managers/bufmgr.h"

/* At the end of G-Store's storage algorithm, the graph is read to disk */ 
//#include <g-store.h>

void read_to_blocks(FILE* fp_in, FILE* fp_gid_map)
{
  rewind(fp_gid_map);
  rewind(fp_in);

  LARGE_INTEGER freq, start, finish;

  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start);

  //bufmgr1 bm(int4(PARAM::max_memory / GLOBALS::blk_size), true);
  bufmgr1 bm(-1, true);

  g_id* gid_map = (g_id*) malloc1<int>(GLOBALS::num_vertices,"");
  char* line = malloc1<char>(MAXLINE + 1, "");

  int i, adv = 0, progress = GLOBALS::num_vertices/20, cnt_x = 0;
  g_id me;
  b_id my_bid;
  b_sc my_slt;
  char* str0;

  block1 b;

  header1 h = malloc1<char>(GLOBALS::header_len, "todo");
  char* fields = malloc1<char>(GLOBALS::blk_writable,"todo");

  for (i = 0; i < GLOBALS::num_vertices; i++)
  {
    fgets(line, MAXLINE_INT, fp_gid_map);
    gid_map[i] = strtoul(line, NULL, 10);
  }

  do 
    fgets(line, MAXLINE, fp_in);
  while (*line == '#' && !feof(fp_in));

  printf("Reading vertices to blocks:     0 [                    ] %d", GLOBALS::num_vertices);
  put_mult_char('\b',len_num_str(GLOBALS::num_vertices) + 20 + 2);

  for (i = 0; i < GLOBALS::num_vertices; i++)
  {
    fgets(line, MAXLINE, fp_in);

    me = gid_map[strtoul(line, &str0, 10)];
    assert(gid_map[i]== me);

    my_bid = get_block_id(me);
    my_slt = get_slot(me);
    
    if (my_slt == 0)
    {
      bm.get_block_w(b, my_bid, true);
      blk_initialize(b, me);
    }
    else
       bm.get_block_w(b, my_bid,  false);

    if (GLOBALS::first_is_id == 1)
      parse_fields(me, my_bid, line, fields, h, blk_get_var_start(b), gid_map);
    else
      parse_fields(me, my_bid, str0, fields, h, blk_get_var_start(b), gid_map);
    
    /*if (b + h[GLOBALS::ee_in_h + 1] > b + (GLOBALS::blk_size - GLOBALS::header_len * (my_slt + 1)))
      do_nothing();*/

    //printf("%u ", blk_get_var_start(b));
    //print_arr_uint2(h, GLOBALS::header_len/2);
    //printf("%d\n", h[GLOBALS::header_len/2-1] - h[0]);

    blk_insert(b, my_slt, h, fields);   
    //print_arr_uint2((uint2*)h,6);
    //fwrite(fields, 50, 1, stdout);
    //printf("%u", get_header2(b+4));
    
    //blk_print(b);
    if (++adv == progress && (adv = 0) == 0 && cnt_x++ == cnt_x)
      putchar('=');
    
    //b->print_block();
    //printf("%d ", i);
    //if (i == 48915)
    //  print_endln();
  }

  printf("\n\n");

  //print_ln("Almost there...");

  bm.write_memory();

  QueryPerformanceCounter(&finish);
  free_all(4, gid_map, line, h, fields);
}

void parse_fields(g_id me, b_id my_bid, char* str0, char* fields, header1 h, uint4 var_start, g_id* gid_map)
{
  uint1  tmp_uint1;
  int1   tmp_int1;
  uint2  tmp_uint2;
  int2   tmp_int2;
  uint4  tmp_uint4;
  int4   tmp_int4;
  uint8  tmp_uint8;
  int8   tmp_int8;
  double tmp_d;
  bool dupl;

  b_sc cnt_fr_offset = 0;
  b_sc cnt_vr_offset = GLOBALS::len_fix_flds;
  b_sc cnt_str_len;
  b_sc cnt_header = GLOBALS::header_slt_len;
  char* str1, *str2, *str3, *str4;
  int i, ie_len, ee_len;
  
  g_id edge;
  b_sc edge_slt;
  char* ext_edges = malloc1<char>(GLOBALS::blk_writable, "todo");
  
  set_header(h, var_start);
  
  for (i = 0; i < GLOBALS::num_fields; i++)
  {
    while (*str0 == ' ')
      str0++;

    switch (GLOBALS::rec_fields[i].field_datatype)
    {
    case bool_:
      if (*str0 == '0' || *str0 == 'F' || *str0 == 'f')
        fields[cnt_fr_offset] = 0;
      else //(*str0 == '1' || *str0 == 'T' || *str0 == 't')
        fields[cnt_fr_offset] = 1;
      
      cnt_fr_offset++;
      while (*str0 != ' ' && *str0 != '\n' && *str0 != '\0')
        str0++;
      break;

    case int_:
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case 1:
        tmp_int1 = (int1) strtol(str0, &str1, 10);
        *((int1*) (fields + cnt_fr_offset)) = tmp_int1;
        cnt_fr_offset++;
        break;
      case 2:
        tmp_int2 = (int2) strtol(str0, &str1, 10);
        *((int2*) (fields + cnt_fr_offset)) = tmp_int2;
        cnt_fr_offset += 2;
        break;
      case 4:
        tmp_int4 = (int4) strtol(str0, &str1, 10);
        *((int4*) (fields + cnt_fr_offset)) = tmp_int4;
        cnt_fr_offset += 4;
        break;
      case 8:
        tmp_int8 = (int8) _strtoi64(str0, &str1, 10);
        *((int8*) (fields + cnt_fr_offset)) = tmp_int8;
        cnt_fr_offset += 8;
        break;
      }
      str0 = str1;
      break;

    case uint_:         
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case 1:
        tmp_uint1 = (uint1) strtoul(str0, &str1, 10);
        *((uint1*) (fields + cnt_fr_offset)) = tmp_uint1;
        cnt_fr_offset++;
        break;
      case 2:
        tmp_uint2 = (uint2) strtoul(str0, &str1, 10);
        *((uint2*) (fields + cnt_fr_offset)) = tmp_uint2;
        cnt_fr_offset += 2;
        break;
      case 4:
        tmp_uint4 = (uint4) strtoul(str0, &str1, 10);
        *((uint4*) (fields + cnt_fr_offset)) = tmp_uint4;
        cnt_fr_offset += 4;
        break;
      case 8:
        tmp_uint8 = (uint8) _strtoui64(str0, &str1, 10);
        *((uint8*) (fields + cnt_fr_offset)) = tmp_uint8;
        cnt_fr_offset += 8;
        break;
      }
      str0 = str1;
      break;

    case double_:
      tmp_d = strtod(str0, &str1);
      *((double*) (fields + cnt_fr_offset)) = tmp_d;
      cnt_fr_offset += 8;
      break;

    case fixchar_:
      str0++; // "
      str1 = fields + cnt_fr_offset;
      cnt_str_len = str_read_after_quote_adv(str0, str1);

      while (cnt_str_len < GLOBALS::rec_fields[i].field_length)
      {
        cnt_str_len++;
        *str1++ = '\0';
      }

      cnt_fr_offset += GLOBALS::rec_fields[i].field_length;
      break;

    case varchar_:
      
      str0++; // "      
      str1 = fields + cnt_vr_offset;

      cnt_str_len = str_read_after_quote_adv(str0, str1);
     
      set_header(h + cnt_header, get_header(h + cnt_header - GLOBALS::header_slt_len) + cnt_str_len 
                                 + (cnt_header == GLOBALS::header_slt_len ? GLOBALS::len_fix_flds : 0));
      cnt_header += GLOBALS::header_slt_len;
      cnt_vr_offset += cnt_str_len;
      break;
    
    case skip_:
      str_skip_adv(str0);
      break;
    }
  }

  assert(cnt_fr_offset == GLOBALS::len_fix_flds);
  assert(cnt_header == GLOBALS::ee_in_h);

  str2 = fields + cnt_vr_offset;
  str3 = ext_edges;
  
  for (;;) 
  {
    while (*str0 == ' ')
      str0++;
    
    if (*str0=='\n' || *str0=='\0')
      break;
    
    edge = gid_map[strtoul(str0, &str1, 10)];

    str0 = str1;
    
    if (edge == me)   //self-edge
      continue;

    dupl = false;
    if (get_block_id(edge) == my_bid)
    {
      edge_slt = get_slot(edge);
      
      str4 = fields + cnt_vr_offset;
      while(str4 != str2)
        if (get_ie_adv(str4) == edge_slt)
        { 
          dupl = true;
          break;
        }
       
      if (!dupl)
        set_ie_adv(str2, edge_slt);
    }
    else
    {
      str4 = ext_edges;
      while (str4 != str3)
        if (get_ee_adv(str4) == edge)
        { 
          dupl = true;
          break;
        }

      if (!dupl)
        set_ee_adv(str3, edge);
    }
  }

  ie_len = str2 - (fields + cnt_vr_offset);
  ee_len = str3 - ext_edges;

  if (GLOBALS::ie_size==1)
    qsort(fields + cnt_vr_offset, ie_len, 1, bs_comp_t<ie_1>);
  else
    qsort(fields + cnt_vr_offset, ie_len/2, 2, bs_comp_t<ie_2>);
  
  qsort(ext_edges, ee_len / 4, 4, bs_comp_t<g_id>);

  set_header(h + GLOBALS::ee_in_h, get_header(h + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off + ie_len);
  set_header(h + GLOBALS::ee_end_in_h, get_header(h + GLOBALS::ee_in_h) + ee_len);
  
  memcpy(str2, ext_edges, ee_len);
  
  free(ext_edges);
}