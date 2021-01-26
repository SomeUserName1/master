#include "calc_header.h"

#include "util_t.h"
#include "util.h"

/* Calculates size and structure of a the header. See dissertation Section 6.7 */ 
//#include <g-store.h>

void calc_header()
{
  int i;
  b_sc cnt_fr_offset = 0, cnt_vp_offset = 0;

  GLOBALS::h_fix_off = malloc1<b_sc>(GLOBALS::num_fields + 2,"todo");
  GLOBALS::h_var_fld = malloc1<b_sc>(GLOBALS::num_fields + 2,"todo");

  for (i = 0; i < GLOBALS::num_fields; i++)
    switch (GLOBALS::rec_fields[i].field_datatype)
    {
    case bool_:
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case -1:
      case 0:
      case 1:
        GLOBALS::h_var_fld[i] = 0;
        GLOBALS::h_fix_off[i] = cnt_fr_offset++;
        break;
      default:
        print_ln("Warning: You specified %d for a bool record field - bool fields always have size 1", GLOBALS::rec_fields[i].field_length);
        break;
      }
      break;

    case uint_:         
    case int_:         
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case 1:
        GLOBALS::h_fix_off[i] = cnt_fr_offset++;
        break;
      case 2:
        GLOBALS::h_fix_off[i] = cnt_fr_offset;
        cnt_fr_offset += 2;
        break;
      case 4:
        GLOBALS::h_fix_off[i] = cnt_fr_offset;
        cnt_fr_offset += 4;
        break;
      case 8:
        GLOBALS::h_fix_off[i] = cnt_fr_offset;
        cnt_fr_offset += 8;
        break;
      default:
        error_exit("int/uint data types must have size 1 2 4 or 8");
        break;
      }
      GLOBALS::h_var_fld[i] = 0;
      break;

    case double_:
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case -1:
      case 0:
      case 8:
        GLOBALS::h_var_fld[i] = 0;
        GLOBALS::h_fix_off[i] = cnt_fr_offset;
        cnt_fr_offset += 8;
        break;
      default:
        print_ln("Warning: You specified %d for a double record field - "
          "double fields always have size 8", GLOBALS::rec_fields[i].field_length);
        break;
      }
      break;

    //fix for first is done below
    case varchar_:
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case -1:
      case 0:
        break;
      default:
        print_ln("Warning: You specified %d for a varchar record field - "
          "varchar fields have a variable size", GLOBALS::rec_fields[i].field_length);
        break;
      }
      GLOBALS::h_var_fld[i] = cnt_vp_offset;
      cnt_vp_offset += GLOBALS::header_slt_len;
      GLOBALS::h_fix_off[i] = 0;
      break;

    case fixchar_:
      if (GLOBALS::rec_fields[i].field_length <= 0)
        error_exit("Size of a fixchar field must be at least 1");
      GLOBALS::h_var_fld[i] = 0;
      GLOBALS::h_fix_off[i] = cnt_fr_offset;
      cnt_fr_offset += GLOBALS::rec_fields[i].field_length;
      break;

    case skip_:
      switch (GLOBALS::rec_fields[i].field_length)
      {
      case -1:
      case 0:
        break;
      default:
        print_ln("Warning: You specified %d for a skip record field - "
          "skip fields are ignored, no matter of their size", GLOBALS::rec_fields[i].field_length);
        break;
      }
      break;

    default:
      error_exit("unknown data type");
  }

  GLOBALS::ie_in_h = GLOBALS::h_var_fld[GLOBALS::num_fields] = cnt_vp_offset;       //edges
  GLOBALS::ee_in_h = GLOBALS::h_var_fld[GLOBALS::num_fields + 1] = GLOBALS::ie_in_h + GLOBALS::header_slt_len;
  GLOBALS::ee_end_in_h = GLOBALS::ie_in_h + GLOBALS::header_slt_len * 2;

  GLOBALS::h_fix_off[GLOBALS::num_fields] = GLOBALS::h_fix_off[GLOBALS::num_fields + 1] = 0;
  //GLOBALS::header_items = cnt_vp_offset + 3;         //begin int, end int/begin ext, end ext = 3
  GLOBALS::header_len = cnt_vp_offset + 3 * GLOBALS::header_slt_len;


  for (i = 0; i < GLOBALS::num_fields; i++)
    if (GLOBALS::rec_fields[i].field_datatype == varchar_ && GLOBALS::h_var_fld[i] == 0)
    {
      GLOBALS::h_fix_off[i] = cnt_fr_offset;
      break;
    } 

  if (GLOBALS::ie_in_h == 0)
    GLOBALS::ie_fix_off = cnt_fr_offset;
  else
    GLOBALS::ie_fix_off = 0;

  GLOBALS::len_fix_flds = cnt_fr_offset;
}