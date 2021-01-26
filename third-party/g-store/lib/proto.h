
//////////////////////////////////////////////////////////////////////////block.cpp
//void blk_init_fkts();
//g_id get_first_gid(b_id bid);
//b_id get_block_id(g_id gid);
//b_sc get_slot(g_id gid);
//void ie_1_to_str_adv(char* &str, char* b_ptr);
//void ie_2_to_str_adv(char* &str, char* b_ptr);
//void ee_to_str_adv  (char* &str, char* b_ptr);
//void set_ie_1(char* b_ptr, b_sc ie);
//void set_ie_2(char* b_ptr, b_sc ie);
//void set_ee  (char* b_ptr, g_id ee);
//void set_ie_1_adv(char* &b_ptr, b_sc ie);
//void set_ie_2_adv(char* &b_ptr, b_sc ie);
//void set_ee_adv  (char* &b_ptr, g_id ee);
//b_sc get_ie_1(char* b_ptr);
//b_sc get_ie_2(char* b_ptr);
//g_id get_ee  (char* b_ptr);
//void adv_ie_1(char* &b_ptr);
//void adv_ie_2(char* &b_ptr);
//void adv_ee  (char* &b_ptr);
//b_sc get_ie_1_adv(char* &b_ptr);
//b_sc get_ie_2_adv(char* &b_ptr);
//g_id get_ee_adv  (char* &b_ptr);
//char* ee_bs (g_id key, char* ee_begin, char* ee_end);
//char* ie_1_bs(b_sc key, char* ie_begin, char* ie_end);
//char* ie_2_bs(b_sc key, char* ie_begin, char* ie_end);
//uint4 get_header2(header1 h);
//uint4 get_header3(header1 h);
//void set_header2(header1 h, uint4 i);
//void set_header3(header1 h, uint4 i);
//void inc_header2(header1 h, uint4 i);
//void inc_header3(header1 h, uint4 i);
//header1 blk_get_header(block1 b, b_sc slt);
//void blk_initialize(block1 b, g_id first_v);
//void blk_insert(block1 b, b_sc slt, header1 header, char* fields);
//g_id blk_get_first_gid(block1 b);
//b_sc blk_get_rec_in_block(block1 b);
//b_sc blk_get_max_slt(block1 b);
//uint4 blk_get_var_start(block1 b);
//void blk_inc_var_start(block1 b, uint4 len);
//void blk_inc_rec_num(block1 b);
//void blk_extract_fld(block1 b, int fld, int slt, char* &dest, uint2 &len);
//void blk_print(block1 b);

////////////////////////////////////////////////////////////////////////// calc_header.cpp
//void calc_header();

////////////////////////////////////////////////////////////////////////// cmd_input.cpp
/*void read_from_input();
void read_menu();
void evaluate_menu(char* line);
void read_file(FILE* fp);
void read_from_input_query();
void cmd_misc();
void read_argument(char* line);
void display_help();
void display_describe(); */ 

////////////////////////////////////////////////////////////////////////// coarsen.cpp
//int coarsen(graph1 *graph);
//void coarsen_two(graph1 *graph, int max_part_w, double &c_ratio);
//void coarsen_n(graph1* graph, int num_v_matched, double &c_ratio);
//void create_c_graph(graph1* graph, int new_v, int* v_per_p, int* v_per_p_begin);

//////////////////////////////////////////////////////////////////////////db_help
void show_help_cmd_arguments();

//////////////////////////////////////////////////////////////////////////db_exit
void destroy_db();


//////////////////////////////////////////////////////////////////////////determine_blocksize
//void determine_blocksize(bool display_prompt);

//////////////////////////////////////////////////////////////////////////entry_points.cpp
//void create_db_new(FILE* fp_in);
//void create_db_part(FILE* fp_in, FILE* fp_parts, char flat);
//void gstore_init();
//void close_down();

//////////////////////////////////////////////////////////////////////////evaluate.cpp
//void evaluate(graph1* graph, int* v_per_p, int* v_per_p_begin, char* text = "");

//////////////////////////////////////////////////////////////////////////finalize.cpp
//void finalize(graph1* graph, int* &v_per_p, int* &v_per_p_begin);

//////////////////////////////////////////////////////////////////////////graph_to_block.cpp
//void read_to_blocks(FILE* fp_in, FILE* fp_gid_map);
//void parse_fields(g_id me, b_id my_bid, char* str0, char* fields, header1 header, uint4 var_start, g_id* gid_map);

//make_graph.cpp

//////////////////////////////////////////////////////////////////////////memory_mgr.cpp
//void mem_create_disks_blks(int num_blocks, int8 max_disk_size_b = ((int8) 1 << 32)-1);;
//void mem_create_disks_g(double total_space_g, double max_disk_size_g = 4.0);
//void mem_create_disks_m(double total_space_m, double max_disk_size_m = 4096.0);
//void mem_create_disks(int8 space_b, int8 max_disk_size_b = ((int8) 1 << 32)-1);
//void mem_reinitialize();
//void mem_write_blocks(block1 b, b_id bid, int num);
//void mem_write_block(block1 b, b_id bid);
//void mem_read_blocks(block1 b, b_id bid, int num);
//void mem_read_block(block1 b, b_id bid);
//void mem_print_disks_info();
//void mem_close_disks();
//void mem_delete_disks();

//////////////////////////////////////////////////////////////////////////move_to_disk.cpp
//int move_graph_to_disk(int mode);
//void read_graph_from_disk(graph1 *graph);

//////////////////////////////////////////////////////////////////////////parameters.cpp
//void load_parameters();
//void save_db_globals();
//void load_db_globals();

//////////////////////////////////////////////////////////////////////////part.cpp
//void part_graph1(graph1 *graph);


//////////////////////////////////////////////////////////////////////////print_graph.cpp
//void print_graph1(graph1 *graph, char* text, bool print_coarser);
//void print_graph2(graph1 *graph, char* text, bool print_coarser);
//void print_graph3(graph1 *graph, int* v_per_p, int* v_per_p_begin, char* text);
//void print_part_gid(graph1 *graph, FILE* part_file, FILE* gid_map_file);


//////////////////////////////////////////////////////////////////////////project.cpp
//void project(graph1* graph, int* v_per_p, int* v_per_p_begin, std::vector<bool>* part_type, int &max_p1);

//////////////////////////////////////////////////////////////////////////query_shared.cpp
//void query_vertex();
//void print_query_header();
//template <typename T_>
//void print_found_path(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<T_>* bm3, g_id* p_v_offset, g_id* v_found_paths);
//void print_found_path_seq(b_id to_bid, b_sc to_slt, bufmgr1* bm1, bufmgr3<bv_list_slt_seq>* bm3, g_id* p_v_offset, g_id** v_found_paths, int seq);
//template <typename T_>
//void print_found_path_common(bufmgr1* bm1, bufmgr3<T_>* bm3);
//void print_query_rec_new(block1 b, header1 h, uint4 recnum, g_id gid, int lvl, int &sel_path_len, bool iscycle);
//void add_to_sel_root_arr(block1 b, header1 h);
//void add_to_sel_path(block1 b, header1 h, int &sel_path_len);
//void push_on_output(char* str1);

//////////////////////////////////////////////////////////////////////////read_input.cpp
//void read_graph(graph1* graph, FILE* fp_in);
//void read_parts_into_graph(graph1* graph, FILE* fp_parts);
//void generate_parts_into_graph(graph1* graph);
//void generate_rnd_parts_into_graph(graph1* graph);
//int get_rec_len(char* &str0, int line_id);

//////////////////////////////////////////////////////////////////////////read_query.cpp
//void read_query(char* line, bool first_q = true);

//////////////////////////////////////////////////////////////////////////read_schema.cpp
//void read_schema(char* line);
//void assign_rec_field_fkts();

//////////////////////////////////////////////////////////////////////////refine.cpp
//void refine(graph1* graph, int* v_per_p, int* v_per_p_begin);

////////////////////////////////////////////////////////////////////////reorder.cpp
//void reorder(graph1* graph, int* v_per_p, int* v_per_p_begin, std::vector<bool>* part_type, int &max_p1);

////////////////////////////////////////////////////////////////////////uncoarsen.cpp
//void turn_around(graph1 *graph);
//int uncoarsen(graph1 *graph, double &t1, double &t2, double &t3, double &t4);

////////////////////////////////////////////////////////////////////////util.cpp
//void error_exit(char* str0, ...);
//void print_byte(one_byte* ob, int num);
//void print_int(int i);
//void print_arr(int* arr, int n);
//void print_arr_uint2(uint2* arr, int n);
//void print_endln();
//void do_nothing();
//void print_nl(char* str0, ...);
//void print_ln(char* str0, ...);
//void print_debug_ln(char* str0, ...);
//void print_debug(char* str0, ...);
//void free_all(int num, ...);
//void* malloc_b(int n, char *msg, int mode = 0);               //TODO: Depreciate
//void* realloc_b(void* &x, int n, char *msg, int mode = 0);    //TODO: Depreciate
//int strcpystr (char* &str_source, char* str_dest);
//void rnd_string(char* s, int len);
//double rnd_normal(double mean, double stddev);
//void permute(int *perm, int num);
//void test_heap();
//void strcat1(char* dest, char* a, char* b);
//void change_p_lists(int v_id, int from_p, int to_p, int* v_per_p, int* v_per_p_begin);
//void combine_p_lists(int first_p, int last_p, int new_p_num, int* v_per_p, int* v_per_p_begin);
//void str_app_adv(char* &s, char c);
//void str_app_adv(char* &s, char* t);
//graph1* malloc_graph1_init(char *msg);
//void free_graph1(graph1* graph);
//int read_int();
//bool confirm_yn(char* str, ...);
//bool str_tok_cmp(char* &str0, char* tok, char mode = 0, char* str_error="");
//bool str_tok_cmp_adv(char* &str0, char* tok, char mode = 0, char* str_error="");
//bool str_tok_cmp_adv_qd(char* &str0, char* tok, char mode = 0);
//bool str_tok_cmp_adv_sd(char* &str0, char* tok, char mode = 0);
//bool str_tok_cmp_adv_menu(char* &str0, char* tok, char mode = 0);
//bool str_tok_cmp_qd(char* &str0, char* tok, char mode = 0);
//bool str_tok_cmp_sd(char* &str0, char* tok, char mode = 0);
//bool str_tok_cmp_menu(char* &str0, char* tok, char mode = 0);
//void str_eat_ws(char* &str0);
//void str_eat_ws_err(char* &str0, char* str_error);
//void str_eat_ws_qd(char* &str0);
//void str_eat_ws_sd(char* &str0);
//void str_eat_ws_tok_err(char* &str0, char tok, char* str_error);
//void str_eat_ws_tok_qd(char* &str0, char tok);
//void str_eat_ws_tok_sd(char* &str0, char tok);
//void str_eat_ws_tok_menu(char* &str0, char tok);
//bool str_is_ws_err(char* str0, char* str_error, char alt_sym1 = ' ', char alt_sym2 = ' ');
//bool str_is_ws_qd(char* str0, char alt_sym1 = ' ', char alt_sym2 = ' ');
//bool str_is_ws_sd(char* str0, char alt_sym1 = ' ', char alt_sym2 = ' ');
//void write_param_arr(FILE* fp, char* str0, void* arr, int len);
//void write_param_char(FILE* fp, char* str0, char* val);
//bool read_param_char(FILE* fp, char* &str0, char* str1, char* val, int max_len);
//
//bool strisnew(char* str_new, char* str_last);
//char mystrncmp(char* str0, char* str1, int len);
//void str_cpy_until_wsnl_adv(char* &str0_in, char* &str0_out);
//int str_read_after_quote_adv(char* &str_in, char* &str_out);
//void str_skip_adv(char* &str0);
//int parse_field_id_adv(char* &str0);
//LARGE_INTEGER to_li(int8 i);
//int8 get_threshold(int c_level);
//char* replace_if_reserved(char* str0, char* reserved_list[]);
//char mytoupper(char c);
//
//bool fkt_eq(char* a, char* b, int len);
//bool fkt_neq(char* a, char* b, int len);
//bool fkt_sm_eq(char* a, char* b, int len);
//bool fkt_sm(char* a, char* b, int len);  
//bool fkt_gr_eq(char* a, char* b, int len);
//bool fkt_gr(char* a, char* b, int len);
//void put_mult_char(char c, int num);
//int len_num_str(int i);

////////////////////////////////////////////////////////////////////////read_to_psql
//void read_to_ora_postgres(FILE* fp_in, FILE* fp_out, bool ora);
//void read_to_postgres_copy(FILE* fp_in, FILE* fp_out, FILE* fp_out_v, FILE* fp_out_e);