#ifndef DEFS_H
#define DEFS_H

//#include <limits.h>
//#include <stdio.h>
#include <vector>
#include <windows.h> // for LARGE_INTEGER type

typedef char int1;                // [-128; 128)
typedef short int2;               // [-32,768; 32,768)
typedef int int4;                 // [-2,147,483,648; 2,147,483,648)
typedef long long int8;
typedef unsigned char uint1;      // [0; 256)
typedef unsigned short uint2;     // [0; 65,536)
typedef unsigned int uint4;       // [0; 4,294,967,296)
typedef unsigned int uint;        // [0; 4,294,967,296)
typedef unsigned long long uint8;

const int4   INT4_MAX =  INT_MAX;
const int4   INT4_MIN =  INT_MIN;
const uint4 UINT4_MAX = UINT_MAX;

const int2   INT2_MAX =  SHRT_MAX;
const int2   INT2_MIN =  SHRT_MIN;
const uint2 UINT2_MAX = USHRT_MAX;

const int1   INT1_MAX =  CHAR_MAX;
const int1   INT1_MIN =  CHAR_MIN;
const uint1 UINT1_MAX = UCHAR_MAX;

//////////////////////////////////////////////////////////////////////////

typedef int4 b_id;

typedef uint4 g_id;
typedef uint1 ie_1;
typedef uint2 ie_2;

typedef uint2 b_sc;
typedef char* block1;
typedef char* header1;

typedef b_sc header2;
struct header3 {
  uint4 x : 24;
};

const int MAXLINE = 1280000;
const int MAXLINE_INT = 12;
const int EE_SIZE = 4;
const g_id INVALID_GID = UINT4_MAX;

const g_id PATH_NOT_SEEN = INVALID_GID;
const g_id PATH_FROM_GID = INVALID_GID - 1;
const g_id PATH_TO_GID = INVALID_GID - 2;
const g_id PATH_UNFIT = INVALID_GID - 3;
const g_id PATH_NOT_SEEN_THOUGH_OK = INVALID_GID - 4;

#ifndef PARAM_
#define PARAM_

namespace PARAM
{
  extern char working_dir[220];
  extern char globals_filename[32];
  extern char params_filename[32];
  extern char default_parts_filename[32];     
  extern char default_gidmap_filename[32];
  extern char null_filename[256];
  extern char default_ora_export_filename[256];
  extern char default_psql_export_filename[256];

  extern char output_delimiter;
  extern int max_traverse_depth;
  
  extern int8 max_disk_size;
  extern uint4 query_memory;

  extern bool debug_mode;
  extern double beta;
  extern double alpha;
  extern double gamma;
  extern int runs_a;
  extern int runs_b;

  extern double give_up_page_multiple;
  extern int default_fsblks_per_page;

  extern bool print_query_time;
  extern char query_time_filename[256];
  extern bool count_io;
}

#endif

// globals that need not be saved

struct rec_struct;

#ifndef GLOBALS_
#define GLOBALS_

namespace GLOBALS
{
  extern FILE*  fp_org_size;
  extern int    max_c_level;
  extern double dbl_avg_c_ratio;
  extern int*   isle_map;                 //todo:to be freed
  
  extern int    blk_size;
  extern int    max_rec_p_page;    // defined in finalize
  extern int    num_parts;         // defined in finalize
  extern int    num_fields;
  extern int    header_len;
  extern int    header_slt_len;
  extern int    ie_size;
  extern int    blk_writable;
  extern int    blk_max_num_rec;

  extern int  num_vertices;       //todo:adjust all over the program
  extern int  num_edges;          //only for DESCRIBE

  extern b_sc  ie_in_h;
  extern b_sc  ie_fix_off;
  extern b_sc  ee_in_h;
  extern b_sc  ee_end_in_h;
  extern b_sc  len_fix_flds;
  extern b_sc* h_fix_off;
  extern b_sc* h_var_fld;
  extern uint4 blk_var_start;
  extern char* v_p_blk;

  extern rec_struct* rec_fields;
  extern char first_is_id;
  extern char graph_name[32];
  extern char graph_input_filename[256];

  extern int num_of_disks;
  extern int8 cnt_db_size;
  extern b_id cnt_blks;

  extern char* disk_files_;
  extern char** disk_files;
  extern int8* disk_sizes;
  extern int* blocks_in_disk;
  extern b_id* disk_first_bid;
  extern void** disk_fps;
}

#endif

struct graph1;

//TODO: do we need to "extern" all of them?
//extern double SC_SM_BLOCK_MULT;
//extern double SC_GR_BLOCK_MULT;
//extern int8 MAX_PART_W;
extern bool FIRST_RUN;
extern char MY_NAME[64];
extern g_id MAX_REC_P_PAGE_1;
extern uint1 MAX_REC_P_PAGE_SH;
extern graph1* ORG_GRAPH;

enum data_types
{
  bool_,    // = 1
  int_,     // 1 2 4 8
  uint_,    // 1 2 4 8
  double_,  // = 8
  varchar_, // = var
  fixchar_, // >0
  skip_,

  sel_gid_,
  sel_count_edges_,
  sel_isleaf_,

  sel_rownum_,
  sel_lvl_,
  sel_iscycle_,

  sel_paste_edges_,
  sel_lpad_,
  sel_path_,
  sel_root_,
};

enum query_types
{
  q_vertex_,
  q_trav_,
  q_path_,
  q_spath_,
  q_spath_tree_,
  q_path_seq_
};

extern void (*ie_to_str_adv)(char*&, char*);
extern void (*set_ie)(char*, b_sc);
extern void (*set_ie_adv)(char*&, b_sc);
extern b_sc (*get_ie)(char*);
extern void (*adv_ie)(char*&);
extern b_sc (*get_ie_adv)(char*&);
extern char* (*ie_bs)(b_sc, char*, char*);
extern uint4 (*get_header)(header1);
extern void (*set_header)(header1, uint4);
extern void (*inc_header)(header1, uint4);

extern int (*get_lower_idx)(b_id key, b_id* x, int num_elem);

struct pred_tree_elem;
struct sel_path_struct;
struct sel_root_struct;
  
#ifndef QUERY_
#define QUERY_

namespace QUERY
{
  extern uint2 traverse_lvl;
  extern pred_tree_elem* pred_tree_where;
  extern pred_tree_elem* pred_tree_through;
  
  //new:
  extern std::vector<pred_tree_elem*> pred_tree_through_seq;
  extern int thr_seq_cnt;

  extern rec_struct* select_list;
  extern int select_list_len;
  extern uint4 buffer_memory;
  extern FILE* output_fp;
  extern bool must_close_output;

  extern char* sel_path;
  extern rec_struct* sel_path_ptr;
  extern char cnt_sel_root;
  extern char** sel_root_arr;
  extern bool sel_iscycle;

  extern bool nocycle;

  extern uint4 max_rownum;
  extern int last_lvl_q;
  extern int last_lvl_p;
  
  extern uint4 traverse_memory;
  extern pred_tree_elem* pred_tree_start_with;
  extern pred_tree_elem* pred_tree_end_with;
  extern query_types query_type;

  extern char* output_buffer;
  
  extern int xtra_path_len;
  extern LARGE_INTEGER xtra_start_print;
  extern int xtra_cnt_IO_single;
  extern int xtra_cnt_IO_cons;
}

#endif

struct one_byte;

#endif