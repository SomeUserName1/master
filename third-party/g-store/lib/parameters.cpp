#include "parameters.h"

#include "util_t.h"
#include "util.h"
#include "structs.h"
#include "read_schema.h"
#include "memory_mgr.h"
#include "block.h"

/* I/O for files _globals.g and _parameter.g*/
//#include <g-store.h>

namespace PARAM
{
  //char working_dir[] = "";  
  char working_dir[] = "C:/g-store/";
  char globals_filename[] = "_globals.g";
  char params_filename[] = "_parameter.g";     // this must be changed here
  //char params_filename[] = "C:/_parameter.g";
  char default_parts_filename[] = "_parts.g";     
  char default_gidmap_filename[] = "_gidmap.g";
  char null_filename[] = "";
  //char null_filename[] = "c:/g-store/q.txt";   //DEL
  char default_ora_export_filename[] = "ora_export.sql";
  char default_psql_export_filename[] = "psql_export.sql";
  char output_delimiter = ';';
  int8 max_disk_size = (((int8) 1 << 32)/1024)-1;
  //new
  char default_schema_filename[] = "_example_schema.g";

  uint4 query_memory = 16384;

  bool debug_mode = false;
  double alpha = 0.125;
  double beta = 1.0;
  double gamma = 8.0;
  int runs_a = 2;
  int runs_b = 3;
  double give_up_page_multiple = 0.2;
  int default_fsblks_per_page = 4;
  
  int max_traverse_depth = 10000;

  bool print_query_time = false;
  char query_time_filename[] = "qtimes.g";
  bool count_io = false;
}

namespace GLOBALS
{
  g_id   max_rec_p_page_1;        // defined in finalize
  uint1  max_rec_p_page_sh;      // defined in finalize
  FILE*  fp_org_size;
  int    max_c_level;
  double dbl_avg_c_ratio;
  int*   isle_map = NULL;                 //todo:to be freed

  int    blk_size;
  int    max_rec_p_page;    // defined in finalize
  int    num_parts;                // defined in finalize
  int    num_fields;
  int    header_slt_len;
  int    header_len;
  int    ie_size;
  int    blk_writable;
  int    blk_max_num_rec;

  int    num_vertices;       //todo:adjust all over the program
  int    num_edges; 

  b_sc  ie_in_h;
  b_sc  ie_fix_off;
  b_sc  ee_in_h;
  b_sc  ee_end_in_h;
  b_sc  len_fix_flds;
  b_sc* h_fix_off = NULL;
  b_sc* h_var_fld = NULL;
  uint4 blk_var_start;
  char* v_p_blk = NULL;

  rec_struct* rec_fields = NULL;
  char first_is_id;
  char graph_name[] = "";
  char graph_input_filename[] = "";

  int num_of_disks;
  int8 cnt_db_size;
  b_id cnt_blks;

  char* disk_files_ = NULL;
  char** disk_files = NULL;
  int8* disk_sizes = NULL;
  int* blocks_in_disk = NULL;
  b_id* disk_first_bid = NULL;
  void** disk_fps = NULL;
}

// RESET STACK ALLOCATION

void load_parameters()
{
  char filename[256];
  char last_var[64];
  char* line_ = malloc1<char>(MAXLINE, "");
  char* line = line_;
  bool found_eof = false;
  strcpy(filename, PARAM::params_filename);
  FILE* fp_params = fopen(filename, "r");

  if (fp_params == NULL)
  {
    PARAM::query_memory*=1024;   //todo: not very elegant and error prone, fix
    PARAM::max_disk_size*=1024;
    return; // use defaults
  }

  do 
    fgets(line, MAXLINE, fp_params);
  while ((*line == '#' || *line == '\n') && !feof(fp_params));

  using namespace PARAM;

  do
  {
    found_eof = (feof(fp_params) != 0);
    read_param_char (fp_params, line, "working_dir", working_dir, 220);
    read_param_char (fp_params, line, "globals_filename", globals_filename, 32);
    
    read_param_char (fp_params, line, "default_parts_filename", default_parts_filename, 32);
    read_param_char (fp_params, line, "default_gidmap_filename", default_gidmap_filename, 32);
    
    read_param_char (fp_params, line, "default_ora_export_filename", default_ora_export_filename, 256);
    read_param_char (fp_params, line, "default_psql_export_filename", default_psql_export_filename, 256);
    read_param      (fp_params, line, "default_fsblks_per_blk", default_fsblks_per_page);
    
    read_param_char (fp_params, line, "null_filename", null_filename, 256);
    read_param      (fp_params, line, "query_memory", query_memory);
    read_param      (fp_params, line, "output_delimiter", output_delimiter); //char
    read_param      (fp_params, line, "max_disk_size", max_disk_size);

    read_param      (fp_params, line, "debug_mode", debug_mode);
    read_param      (fp_params, line, "beta", beta);
    read_param      (fp_params, line, "alpha", alpha);
    read_param      (fp_params, line, "gamma", gamma);
    read_param      (fp_params, line, "runs_a", runs_a);
    read_param      (fp_params, line, "runs_b", runs_b);
    read_param      (fp_params, line, "give_up_blk_multiple", give_up_page_multiple);
    read_param      (fp_params, line, "max_traverse_depth", max_traverse_depth);

    read_param      (fp_params, line, "max_traverse_depth", max_traverse_depth);
    read_param      (fp_params, line, "print_query_time", print_query_time);
    read_param_char (fp_params, line, "query_time_filename", query_time_filename, 256);
    read_param      (fp_params, line, "count_io", count_io);

    if (!strisnew(line, last_var))
    {
      print_ln("Warning: Ignoring unknown parameter %s in file %s", last_var, filename);
      do 
        fgets(line, MAXLINE, fp_params);
      while ((*line == '#' || *line == '\n') && !feof(fp_params));
    }

  } while (*line != ';' && !found_eof);

  query_memory*=1024;   // NOW IN B!
  max_disk_size*=1024;

  fclose(fp_params);
  free(line_);
}

void save_db_globals()
{
  char filename[256];
  strcat1(filename, PARAM::working_dir, PARAM::globals_filename);
  FILE* fp_globals = fopen(filename, "wb");

  if (fp_globals==NULL)
    error_exit("cannot open globals file '%s' for write", filename);

  fprintf(fp_globals, "%s\n", "# This file may not be modified. It will destroy the binary encoded information.");

  using namespace GLOBALS;

  write_param      (fp_globals, "blk_size", blk_size);
  write_param      (fp_globals, "max_rec_p_page", max_rec_p_page);
  write_param      (fp_globals, "num_parts", num_parts);
  write_param      (fp_globals, "num_fields", num_fields);
  write_param      (fp_globals, "header_slt_len", header_slt_len);
  write_param      (fp_globals, "header_len", header_len);
  write_param      (fp_globals, "ie_size", ie_size);
  write_param      (fp_globals, "blk_writable", blk_writable);
  write_param      (fp_globals, "blk_max_num_rec", blk_max_num_rec);

  write_param      (fp_globals, "num_vertices", num_vertices);
  write_param      (fp_globals, "num_edges", num_edges);
  write_param      (fp_globals, "ie_in_h", ie_in_h);
  write_param      (fp_globals, "ie_fix_off", ie_fix_off);
  write_param      (fp_globals, "ee_in_h", ee_in_h);
  write_param      (fp_globals, "ee_end_in_h", ee_end_in_h);
  write_param      (fp_globals, "len_fix_flds", len_fix_flds);

  write_param_arr  (fp_globals, "h_fix_off", h_fix_off, sizeof(b_sc) * num_fields);
  write_param_arr  (fp_globals, "h_var_fld", h_var_fld, sizeof(b_sc) * num_fields);
  write_param_arr  (fp_globals, "v_p_blk", v_p_blk, ie_size * num_parts);
  
  write_param      (fp_globals, "num_of_disks", num_of_disks);
  write_param      (fp_globals, "cnt_blks", cnt_blks);
  write_param      (fp_globals, "cnt_db_size", cnt_db_size);
  
  write_param_arr  (fp_globals, "disk_sizes", disk_sizes, sizeof(int8) * num_of_disks);
  write_param_arr  (fp_globals, "blocks_in_disk", blocks_in_disk, sizeof(int) * num_of_disks);
  write_param_arr  (fp_globals, "disk_first_bid", disk_first_bid, sizeof(b_id) * num_of_disks);


  write_param_arr  (fp_globals, "disk_files_", disk_files_, (strlen(PARAM::working_dir) + 20) * num_of_disks);
  
  write_param_arr  (fp_globals, "rec_fields", rec_fields, sizeof(rec_struct) * num_fields);
  write_param      (fp_globals, "first_is_id", first_is_id);
  write_param_char (fp_globals, "graph_input_filename", graph_input_filename);
  write_param_char (fp_globals, "graph_name", graph_name);
  
  putc(';', fp_globals);
  
  fclose(fp_globals);
}


void load_db_globals()
{
  char filename[256];
  char last_var[64];
  bool found_eof = false;

  strcat1(filename, PARAM::working_dir, PARAM::globals_filename);
  FILE* fp_globals = fopen(filename, "rb");

  if (fp_globals == NULL)
    return;

  char* line_ = malloc1<char>(MAXLINE, "todo");
  char* line = line_;

  do
    fgets(line, MAXLINE, fp_globals);
  while ((*line == '#' || *line == '\n') && !feof(fp_globals));

  if (feof(fp_globals))
    error_exit("globals input file empty");

  using namespace GLOBALS;

  free( h_fix_off );
  free( h_var_fld );
  free( v_p_blk );
  free( rec_fields );
  free( disk_files_ );
  free( disk_files );
  free( disk_sizes );
  free( blocks_in_disk );
  free( disk_first_bid );
  free( disk_fps );

  do
  {
    found_eof = (feof(fp_globals) != 0);
    read_param           (fp_globals, line, "blk_size", blk_size);
    read_param           (fp_globals, line, "max_rec_p_page", max_rec_p_page);
    read_param           (fp_globals, line, "num_parts", num_parts);
    read_param           (fp_globals, line, "num_fields", num_fields);
    read_param           (fp_globals, line, "header_slt_len", header_slt_len);
    read_param           (fp_globals, line, "header_len", header_len);
    read_param           (fp_globals, line, "ie_size", ie_size);
    read_param           (fp_globals, line, "blk_writable", blk_writable);
    read_param           (fp_globals, line, "blk_max_num_rec", blk_max_num_rec);

    read_param           (fp_globals, line, "num_vertices", num_vertices);
    read_param           (fp_globals, line, "num_edges", num_edges);
    read_param           (fp_globals, line, "ie_in_h", ie_in_h);
    read_param           (fp_globals, line, "ie_fix_off", ie_fix_off);
    
    read_param           (fp_globals, line, "ee_in_h", ee_in_h);
    read_param           (fp_globals, line, "ee_end_in_h", ee_end_in_h);
    read_param           (fp_globals, line, "len_fix_flds", len_fix_flds);

    read_param_arr       (fp_globals, line, "h_fix_off", h_fix_off);
    read_param_arr       (fp_globals, line, "h_var_fld", h_var_fld);
    read_param_arr       (fp_globals, line, "v_p_blk", v_p_blk);

    read_param           (fp_globals, line, "num_of_disks", num_of_disks);
    read_param           (fp_globals, line, "cnt_blks", cnt_blks);
    read_param           (fp_globals, line, "cnt_db_size", cnt_db_size);

    read_param_arr       (fp_globals, line, "disk_sizes", disk_sizes);
    read_param_arr       (fp_globals, line, "blocks_in_disk", blocks_in_disk);
    read_param_arr       (fp_globals, line, "disk_first_bid", disk_first_bid);
    read_param_arr       (fp_globals, line, "disk_files_", disk_files_);

    read_param_arr       (fp_globals, line, "rec_fields", rec_fields);
    read_param           (fp_globals, line, "first_is_id", first_is_id);
    read_param_char      (fp_globals, line, "graph_name", graph_name, 32);
    read_param_char      (fp_globals, line, "graph_input_filename", graph_input_filename, 256);

    if (!strisnew(line, last_var))
      error_exit("found unknown variable %s in file %s", last_var, filename);

  } while (*line != ';' && !found_eof);

  assign_rec_field_fkts();
  mem_reinitialize();
  blk_init_fkts();

  free(line_);
  //fclose(fp_globals);  //TODO: I think this belongs here
}

