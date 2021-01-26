#ifndef PRED_TREE_H
#define PRED_TREE_H

#include "defs.h"
#include "util_t.h"

/* See dissertation Section 7.4 */
struct pred_parser
{
  char* str0;
  bool reduced_pc;
  bool used_extended;
  
  enum ops
  {
    eq_,
    neq_,
    smeq_,
    sm_,
    greq_,
    gr_
  };

  struct extrema_rownum_lvl;

  
  char* get_str_pos();
  void free_pred_tree(pred_tree_elem*);
  
  pred_tree_elem *parse(char* str0, bool reduced, bool &used_extended, bool set_extr);
  void parse_seq(char* str0);

  pred_tree_elem *parse_or(extrema_rownum_lvl &extr_result);
  pred_tree_elem *parse_and(extrema_rownum_lvl &extr_result);
  pred_tree_elem *parse_not(extrema_rownum_lvl &extr_result);
  pred_tree_elem *parse_parenth(extrema_rownum_lvl &extr_result);
  pred_tree_elem *parse_term(extrema_rownum_lvl &extr_result);
  
  template <typename T_>
  void assign_op_funtion(ops op, bool (*&tmp_check_cond)(T_, T_))
  {
    switch (op)
    {
    case eq_:
      tmp_check_cond = fkt_eq; 
      break;

    case neq_:
      tmp_check_cond = fkt_neq;
      break;

    case smeq_:
      tmp_check_cond = fkt_sm_eq;
      break;

    case sm_:
      tmp_check_cond = fkt_sm;
      break;

    case greq_:
      tmp_check_cond = fkt_gr_eq;
      break;

    case gr_:
      tmp_check_cond = fkt_gr;
      break;
    }
  }
  
  void assign_op_funtion(ops op, bool (*&tmp_check_cond)(char*, char*, int))
  {
    switch (op)
    {
    case eq_:
      tmp_check_cond = fkt_eq; 
      break;

    case neq_:
      tmp_check_cond = fkt_neq;
      break;

    case smeq_:
      tmp_check_cond = fkt_sm_eq;
      break;

    case sm_:
      tmp_check_cond = fkt_sm;
      break;

    case greq_:
      tmp_check_cond = fkt_gr_eq;
      break;

    case gr_:
      tmp_check_cond = fkt_gr;
      break;
    }
  }

};

struct pred_parser::extrema_rownum_lvl
{
  uint4 min_rownum;
  uint4 max_rownum;
  int min_lvl;
  int max_lvl;
  bool changed_rownum;
  bool changed_lvl;

  static const uint4 min_rownum_init = 0 + 1;
  static const uint4 max_rownum_init = UINT_MAX - 1;
  static const int min_lvl_init = INT_MIN + 1;
  static const int max_lvl_init = INT_MAX - 1;

  void apply_or(extrema_rownum_lvl e)
  {
    if (changed_rownum && e.changed_rownum)
    {
      min_rownum = min1(min_rownum, e.min_rownum);
      max_rownum = max1(max_rownum, e.max_rownum);
    }
    else if (e.changed_rownum)
    {
      min_rownum = e.min_rownum;
      max_rownum = e.max_rownum;
      changed_rownum = true;
    }

    if (changed_lvl && e.changed_lvl)
    {
      min_lvl = min1(min_lvl, e.min_lvl);
      max_lvl = max1(max_lvl, e.max_lvl);
    }
    else if (e.changed_lvl)
    {
      min_lvl = e.min_lvl;
      max_lvl = e.max_lvl;
      changed_lvl = true;
    }
  }

  void apply_and(extrema_rownum_lvl e)
  {
    min_rownum = max1(min_rownum, e.min_rownum);
    max_rownum = min1(max_rownum, e.max_rownum);
    changed_rownum = changed_rownum || e.changed_rownum;
    
    min_lvl = max1(min_lvl, e.min_lvl);
    max_lvl = min1(max_lvl, e.max_lvl);
    changed_lvl = changed_lvl || e.changed_lvl;
  }

  void apply_not()
  {
    if (changed_rownum)
    {
      if (min_rownum == min_rownum_init)
        min_rownum = max_rownum + 1;
      else 
        min_rownum = min_rownum_init;

      if (max_rownum == max_rownum_init)
        max_rownum = min_rownum - 1;
      else 
        max_rownum = max_rownum_init;
    }

    if (changed_lvl)
    {      
      if (min_lvl == min_lvl_init)
        min_lvl = max_lvl + 1;
      else 
        min_lvl = min_lvl_init;

      if (max_lvl == max_lvl_init)
        max_lvl = min_lvl - 1;
      else 
        max_lvl = max_lvl_init;
    }
  }

  void update_rownum(uint4 rownum, ops op)
  {
    changed_rownum = true;

    switch (op)
    {
    case eq_:
      min_rownum = max_rownum = rownum; 
      break;

    case neq_:
      if (min_rownum == rownum)
        min_rownum++;
      if (max_rownum == rownum)
        max_rownum--;
      break;

    case smeq_:
      max_rownum = rownum;
      break;

    case sm_:
      max_rownum = rownum - 1;
      break;

    case greq_:
      min_rownum = rownum;
      break;

    case gr_:
      min_rownum = rownum + 1;
      break;
    }
  }

  void update_lvl(int lvl, ops op)
  {
    changed_lvl = true;

    switch (op)
    {
    case eq_:
      min_lvl = max_lvl = lvl; 
      break;

    case neq_:
      if (min_lvl == lvl)
        min_lvl++;
      if (max_lvl == lvl)
        max_lvl--;
      break;

    case smeq_:
      max_lvl = lvl;
      break;

    case sm_:
      max_lvl = lvl - 1;
      break;

    case greq_:
      min_lvl = lvl;
      break;

    case gr_:
      min_lvl = lvl + 1;
      break;
    }
  }

  extrema_rownum_lvl(): min_rownum (min_rownum_init), max_rownum (max_rownum_init), 
    min_lvl(min_lvl_init), max_lvl(max_lvl_init), changed_rownum(false), changed_lvl(false) {}
};


struct pred_tree_elem
{
public:
  pred_tree_elem* left;
  pred_tree_elem* right;
  virtual bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num = 0, int lvl = 0, bool is_cycle = false) = 0;
  pred_tree_elem(pred_tree_elem* l, pred_tree_elem* r) : left(l), right(r){};
};

// AND, OR, NOT

struct pred_tree_elem_and : public pred_tree_elem
{
  pred_tree_elem_and(pred_tree_elem* l, pred_tree_elem* r) : pred_tree_elem(l,r){};
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return left->evaluate(b, h, gid, row_num, lvl) && right->evaluate(b, h, gid, row_num, lvl);
  };
};

struct pred_tree_elem_or : public pred_tree_elem
{
  pred_tree_elem_or(pred_tree_elem* l, pred_tree_elem* r) : pred_tree_elem(l,r){};
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return left->evaluate(b, h, gid, row_num, lvl, is_cycle) || right->evaluate(b, h, gid, row_num, lvl, is_cycle);
  };
};

struct pred_tree_elem_not : public pred_tree_elem 
{
  pred_tree_elem_not(pred_tree_elem* r) : pred_tree_elem(NULL,r){};
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return !right->evaluate(b, h, gid, row_num, lvl, is_cycle);
  }
};

struct pred_tree_elem_TERM : public pred_tree_elem 
{
  pred_tree_elem_TERM(): pred_tree_elem(NULL, NULL){};
  int h_fix_fld;
  int h_var_fld;
  int field_id;
};

//////////////////////////////////////////////////////////////////////////

template <typename T_>
struct pred_tree_elem_num_bool : public pred_tree_elem_TERM
{
  pred_tree_elem_num_bool(T_ targ_): targ(targ_), pred_tree_elem_TERM(){};
  T_ targ;
  bool (*check_cond)(T_, T_);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
	return check_cond(*((T_*)(b + (get_header(h) + h_fix_fld))), targ);
  }
};

struct pred_tree_elem_fixchar : public pred_tree_elem_TERM
{
  pred_tree_elem_fixchar(char* targ_, int len_): targ(targ_), len(len_), pred_tree_elem_TERM(){};
  char* targ;
  int len;
  bool (*check_cond)(char*, char*, int);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return check_cond(b + (get_header(h + h_var_fld) + h_fix_fld), targ, len);
  }
};

struct pred_tree_elem_varchar : public pred_tree_elem_TERM
{
  pred_tree_elem_varchar(char* targ_): targ(targ_), pred_tree_elem_TERM(){};
  char* targ;
  bool (*check_cond)(char*, char*, int);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return check_cond(b + (get_header(h + h_var_fld) + h_fix_fld), targ, get_header(h + h_var_fld + GLOBALS::header_slt_len) - (get_header(h + h_var_fld) + h_fix_fld));
  }
};

struct pred_tree_elem_false : public pred_tree_elem_TERM
{
  pred_tree_elem_false (): pred_tree_elem_TERM(){};
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return false;
  };
};

struct pred_tree_elem_true : public pred_tree_elem_TERM
{
  pred_tree_elem_true (): pred_tree_elem_TERM(){};
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return true;
  };
};

struct pred_tree_elem_lvl : public pred_tree_elem_TERM
{
  pred_tree_elem_lvl (int targ_): targ(targ_), pred_tree_elem_TERM(){};
  int targ;
  bool (*check_cond)(int, int);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return check_cond(lvl, targ);
  };
};

struct pred_tree_elem_iscycle : public pred_tree_elem_TERM
{
  pred_tree_elem_iscycle (bool targ_): targ(targ_), pred_tree_elem_TERM(){};
  bool targ;
  bool (*check_cond)(bool, bool);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return check_cond(is_cycle, targ);
  };
};

struct pred_tree_elem_gid : public pred_tree_elem_TERM
{
  pred_tree_elem_gid (g_id targ_): targ(targ_), pred_tree_elem_TERM(){};
  g_id targ;
  bool (*check_cond)(g_id, g_id);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return check_cond(gid, targ);
  };
};

struct pred_tree_elem_rownum : public pred_tree_elem_TERM
{
  pred_tree_elem_rownum  (uint4 targ_): targ(targ_), pred_tree_elem_TERM(){};
  uint4 targ;
  bool (*check_cond)(uint4, uint4);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return check_cond(row_num, targ);
  };
};

struct pred_tree_elem_countedges : public pred_tree_elem_TERM
{
  pred_tree_elem_countedges (b_sc targ_): targ (targ_), pred_tree_elem_TERM(){};
  b_sc targ;
  bool (*check_cond)(b_sc, b_sc);
  bool evaluate(block1 b, header1 h, g_id gid, uint4 row_num, int lvl, bool is_cycle)
  {
    return check_cond((get_header(h + GLOBALS::ee_in_h) - get_header(h + GLOBALS::ie_in_h) - GLOBALS::ie_fix_off) / GLOBALS::ie_size + 
      (get_header(h + GLOBALS::ee_end_in_h) - get_header(h + GLOBALS::ee_in_h) ) / 4, targ);
  };
};

#endif