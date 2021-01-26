#include "block.h"

#include "util_t.h"
#include "util.h"
#include "structs.h"

/* See dissertation Section 6.7 
Defines functions for operations on block level. The function pointers 
declared right at the start are set to the appropriate function (depending on 
the size of an internal edge) in blk_init_fkts() */ 
//#include <g-store.h>

void (*ie_to_str_adv)(char*&, char*);
void (*set_ie)(char*, b_sc);
void (*set_ie_adv)(char*&, b_sc);
b_sc (*get_ie)(char*);
void (*adv_ie)(char*&);
b_sc (*get_ie_adv)(char*&);
char* (*ie_bs)(b_sc, char*, char*);
uint4 (*get_header)(header1);
void (*set_header)(header1, uint4);
void (*inc_header)(header1, uint4);

g_id MAX_REC_P_PAGE_1;
uint1 MAX_REC_P_PAGE_SH;

void ie_1_to_str_adv(char* &str, char* b_ptr){typetostr_adv<ie_1>(str, b_ptr);}
void ie_2_to_str_adv(char* &str, char* b_ptr){typetostr_adv<ie_2>(str, b_ptr);}
void ee_to_str_adv  (char* &str, char* b_ptr){typetostr_adv<g_id>(str, b_ptr);}

void set_ie_1(char* b_ptr, b_sc ie){*((ie_1*) b_ptr) = (ie_1) ie;}
void set_ie_2(char* b_ptr, b_sc ie){*((ie_2*) b_ptr) = (ie_2) ie;}
void set_ee  (char* b_ptr, g_id ee){*((g_id*) b_ptr) = ee;}

void set_ie_1_adv(char* &b_ptr, b_sc ie){*((ie_1*) b_ptr) = (ie_1) ie; b_ptr++;}
void set_ie_2_adv(char* &b_ptr, b_sc ie){*((ie_2*) b_ptr) = (ie_2) ie; b_ptr += 2;}
void set_ee_adv  (char* &b_ptr, g_id ee){*((g_id*) b_ptr) = ee; b_ptr += 4;}

b_sc get_ie_1(char* b_ptr){return *((ie_1*) b_ptr);}
b_sc get_ie_2(char* b_ptr){return *((ie_2*) b_ptr);}
g_id get_ee  (char* b_ptr){return *((g_id*) b_ptr);}

void adv_ie_1(char* &b_ptr){b_ptr++;}
void adv_ie_2(char* &b_ptr){b_ptr+=2;}
void adv_ee  (char* &b_ptr){b_ptr+=4;}

b_sc get_ie_1_adv(char* &b_ptr){return *((ie_1*) b_ptr++);}
b_sc get_ie_2_adv(char* &b_ptr){
	b_sc tmp = *((ie_2*) b_ptr); 
	b_ptr += 2; 
	return tmp;
}
g_id get_ee_adv  (char* &b_ptr){g_id tmp = *((g_id*) b_ptr); b_ptr += 4; return tmp;}

char* ie_1_bs(b_sc key, char* ie_begin, char* ie_end)
  {return (char*) bsearch(&key, ie_begin, (ie_end - ie_begin), 1, bs_comp_int<ie_1>);};
char* ie_2_bs(b_sc key, char* ie_begin, char* ie_end)
  {return (char*) bsearch(&key, ie_begin, (ie_end - ie_begin)/2, 2, bs_comp_int<ie_2>);};
char* ee_bs (g_id key, char* ee_begin, char* ee_end)
  {return (char*) bsearch(&key, ee_begin, (ee_end - ee_begin)/4, 4, bs_comp_int<g_id>);}; //!! CHANGED TO , 4, (TODO) 

uint4 get_header2(header1 h)
{
  return *((header2*)h);
}
uint4 get_header3(header1 h)
{
  return (*((header3*)h)).x;
}

void set_header2(header1 h, uint4 i)
{
  *((header2*)h) = i;
}
void set_header3(header1 h, uint4 i)
{
  (*((header3*)h)).x = i;
}

void inc_header2(header1 h, uint4 i)
{
  *((header2*)h) += i;
}
void inc_header3(header1 h, uint4 i)
{
  (*((header3*)h)).x += i;
}


void blk_init_fkts()
{
  if (GLOBALS::ie_size == 1)
  {
    set_ie = set_ie_1;
    set_ie_adv = set_ie_1_adv;
    get_ie = get_ie_1;
    get_ie_adv = get_ie_1_adv;
    adv_ie = adv_ie_1;
    ie_to_str_adv = ie_1_to_str_adv;
    ie_bs = ie_1_bs;
  }
  else
  {
    set_ie = set_ie_2;
    set_ie_adv = set_ie_2_adv;
    get_ie = get_ie_2;
    get_ie_adv = get_ie_2_adv;
    adv_ie = adv_ie_2;
    ie_to_str_adv = ie_2_to_str_adv;
    ie_bs = ie_1_bs;
  }

  if (GLOBALS::header_slt_len == 2)
  {
    get_header = get_header2;
    set_header = set_header2;
    inc_header = inc_header2;
  }
  else
  {
    get_header = get_header3;
    set_header = set_header3;
    inc_header = inc_header3;
  }
    
  int tmp = 0;
  int tmp2 = GLOBALS::max_rec_p_page;
  while (tmp2 >>= 1) ++tmp;
  
  MAX_REC_P_PAGE_SH = tmp;
  MAX_REC_P_PAGE_1 = (g_id) GLOBALS::max_rec_p_page - 1;
}

//TODO: check if not a casing mistake somewhere with e.g., g_id(int * int)
g_id get_first_gid(b_id bid)
{
  return g_id(bid) * GLOBALS::max_rec_p_page;
}

b_id get_block_id(g_id gid)
{
  return gid >> MAX_REC_P_PAGE_SH;
}

b_sc get_slot(g_id gid)
{
  return gid & MAX_REC_P_PAGE_1;
}

header1 blk_get_header(block1 b, b_sc slt) //HOK
{
  return b + (GLOBALS::blk_size - GLOBALS::header_len * (slt + 1));
}

void blk_initialize(block1 b, g_id first_v)
{
  memcpy(b, &first_v, 4);          // first gid 
  memcpy(b + 4, &GLOBALS::blk_var_start, GLOBALS::header_slt_len);  // page var start  // careful, copying part of an int
  set_ie(b + 4 + GLOBALS::header_slt_len , 0);
  set_ie(b + 4 + GLOBALS::header_slt_len + GLOBALS::ie_size, 0);
}

void blk_insert(block1 b, b_sc slt, header1 h, char* fields)
{
  uint4 len = get_header(h + GLOBALS::ee_in_h + GLOBALS::header_slt_len) - get_header(h);
  memcpy(blk_get_header(b, slt), h, GLOBALS::header_len);
  memcpy(b + blk_get_var_start(b), fields, len);
  blk_inc_var_start(b, len);
  blk_inc_rec_num(b);
}

// first_gid (4) | var_start (2) | max_rec | num_rec

g_id blk_get_first_gid(block1 b)
{
  return *((g_id*) b);
}

b_sc blk_get_rec_in_block(block1 b)
{
  return get_ie(b + 4 + GLOBALS::header_slt_len + GLOBALS::ie_size);
}

b_sc blk_get_max_slt(block1 b)
{
  return get_ie(b + 4 + GLOBALS::header_slt_len);
}

uint4 blk_get_var_start(block1 b)
{
  return get_header(b + 4);
}

void blk_inc_var_start(block1 b, uint4 len)
{
  inc_header(b + 4, len);
}

void blk_inc_rec_num(block1 b)
{
  set_ie(b + 4 + GLOBALS::header_slt_len, get_ie(b + 4 + + GLOBALS::header_slt_len) + 1);
  set_ie(b + 4 + GLOBALS::header_slt_len + GLOBALS::ie_size, get_ie(b + 4 + GLOBALS::header_slt_len + GLOBALS::ie_size) + 1);
}

void blk_extract_fld(block1 b, int fld, int slt, char* &dest, int &len) //not used
{
  header1 h = (b + (GLOBALS::blk_size - GLOBALS::header_len * (slt + 1)));
  len = get_header(h + GLOBALS::h_var_fld[fld] + GLOBALS::header_slt_len) - (get_header(h + GLOBALS::h_var_fld[fld]) + GLOBALS::h_fix_off[fld]);
  dest = b + (get_header(h + GLOBALS::h_var_fld[fld]) + GLOBALS::h_fix_off[fld]);

  /*switch (GLOBALS::rec_fields[fld].field_datatype)
  {
  case varchar_:
    
    break;
  case edges_:
    error_exit("cannot be asked here");
  default:
    dest = b + (*h_blk + GLOBALS::h_fix_off[fld]);
    break;    
  }*/
}

void blk_print(block1 b)
{
  g_id first_gid = blk_get_first_gid(b);
  uint4 tmp;
  b_sc num_max_rec = blk_get_max_slt(b);
  b_sc rec_in_block = blk_get_rec_in_block(b);
  b_sc i;
  int j;
  uint4 iu;
  header1 h;
  char* line = malloc1<char>(MAXLINE,"todo");
  char* str0;

  printf("PAGE %u | first_gid %u | var_start %u | num_max_rec %u | rec_in_block %u\n\n", get_block_id(first_gid), first_gid, blk_get_var_start(b), num_max_rec, rec_in_block);
  
  printf("__%s", "gid");

  for (i = 0; i < GLOBALS::num_fields; i++)
    printf(" %d_%s", i, GLOBALS::rec_fields[i].field_name);

  printf("\n\n");
  
  for (i = 0; i < num_max_rec; i++)
  {
    h = blk_get_header(b, i);
    
    str0 = line;
    inttostr_adv(str0, first_gid + i);
    
    if (get_header(h) == 0)      // invalid header
      continue;

    /*for (j = 0; GLOBALS::rec_fields[j].field_datatype != edges_; j++)
    {
      str_app_adv(str0, ' ');
      blk_extract_fld(b, j, i, str1, len);
      GLOBALS::rec_fields[j].append_to_str(str0, str1, len);
    }*/

    for (j = 0; j < GLOBALS::num_fields; j++)
    {
      str_app_adv(str0, ' ');
      (GLOBALS::rec_fields[j].*(GLOBALS::rec_fields[j].eff_append_to_str))(str0, b, h);
    }
    
    str_app_adv(str0, ' ');
    str_app_adv(str0, '|');

    tmp = get_header(blk_get_header(b, i) + GLOBALS::ee_in_h);
    for (iu = get_header(blk_get_header(b, i) + GLOBALS::ie_in_h) + GLOBALS::ie_fix_off; iu < tmp; iu += GLOBALS::ie_size)
    {
      str_app_adv(str0, ' ');
      ie_to_str_adv(str0, b + iu);
    }
    
    str_app_adv(str0, ' ');
    str_app_adv(str0, '|');

    tmp = get_header(blk_get_header(b, i) + GLOBALS::ee_end_in_h);
    for (iu = get_header(blk_get_header(b, i) + GLOBALS::ee_in_h); iu < tmp; iu += EE_SIZE)
    {
      str_app_adv(str0, ' ');
      ee_to_str_adv(str0, b + iu);
    }
    
    str_app_adv(str0, '\n');

    fwrite(line, str0-line, 1, stdout);
  }
  
  free(line);
  //    printf("          (%d+%d bytes)\n", HI_LEN, hip->off_ext_end - hip->off_rec);
  printf("___________\n\n");
};