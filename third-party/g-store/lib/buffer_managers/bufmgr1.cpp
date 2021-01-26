#include "bufmgr.h"

#include <assert.h>

#include "memory_mgr.h"
#include "util_t.h"
#include "util.h"
#include "block.h"
#include "parameters.h"

/* The main buffer manager used for all queries except q_trav. It is also used 
with a massive buffer size when reading vertices to blocks in the 
storage algorithm */
//#include <g-store.h>

//Todo MAKE Block ID an integer
//TODO: GLOBALS::num_parts vs num_blks;

bufmgr1::bufmgr1(int num_blks_, bool use_map_) : num_blks(num_blks_), use_map(use_map_)
{
  int i;
      
  if (num_blks == -1) //fix to enable fast reading to blocks
  {             
    num_blks = int(1073741824 * 1.5) / GLOBALS::blk_size;    //1.2GB; 1073741824 bytes = 1GB
    mem = (char*) malloc(num_blks * GLOBALS::blk_size * sizeof(char));
    mem_blks = (block1*) malloc(num_blks * sizeof(block1));

    if (use_map)
    {
      blk_bid_map = (b_id*) malloc(num_blks * sizeof(b_id));
      bid_blk_map = (int4*) malloc(GLOBALS::num_parts * sizeof(int4));
    }
    else
    {
      blk_bid_map = NULL;
      bid_blk_map  = NULL;
    }

    while (mem == NULL || mem_blks == NULL || (use_map && (blk_bid_map == NULL || bid_blk_map == NULL)))
    {
      num_blks = int(num_blks * 0.92);        
      mem = (char*) malloc (num_blks * GLOBALS::blk_size);
      mem_blks = (block1*) malloc(num_blks * sizeof(block1));
      if (use_map)
      {
        blk_bid_map = (b_id*) malloc(num_blks * sizeof(b_id));
        bid_blk_map = (int4*) malloc(GLOBALS::num_parts * sizeof(int4));
      }
    }

    arr_set(blk_bid_map, -1, num_blks);
    arr_set(bid_blk_map, -1, GLOBALS::num_parts);
  } 
  else
  {
    mem = malloc1<char>(num_blks * GLOBALS::blk_size, "allocating buffer memory");
    mem_blks = malloc1<block1>(num_blks, "allocating buffer memory");
  
    if (use_map)
    {
      blk_bid_map = malloc1_set<b_id>(num_blks, -1, "todo");
      bid_blk_map = malloc1_set<int4>(GLOBALS::num_parts, -1, "todo");
    }
    else
    {
      blk_bid_map = NULL;
      bid_blk_map  = NULL;
    }
  }

  for (i = 0; i < num_blks; i++)
    mem_blks[i] = mem + i*GLOBALS::blk_size;

  last_blk = mem_blks[num_blks - 1];
  run0 = true;
  next = 0;
}

bufmgr1::~bufmgr1()
{
  free_all(3, mem, mem_blks, blk_bid_map, bid_blk_map);
}


void bufmgr1::fill_cons_from_start_no_map(block1 &b, b_id bid_start, int &num)
{
  int requested = GLOBALS::num_parts - bid_start;
  
  if (requested > num_blks)
    num = num_blks;
  else
    num = requested;

  mem_read_blocks(b = mem, bid_start, num);
}


void bufmgr1::fill_cons_from_start(block1 &b, b_id bid_start, int &num)
{
  num = 0;

  //invalidate earlier entry in map and add new;
  for (int i = bid_start; i < GLOBALS::num_parts && num < num_blks; i++)
  {
    if (blk_bid_map[num] != -1)
      bid_blk_map[blk_bid_map[num]] = -1;
    
    if (bid_blk_map[num] != -1) 
      blk_bid_map[bid_blk_map[num]] = -1;

    blk_bid_map[num] = i;
    bid_blk_map[i] = num++;
  }

  if (num == num_blks)
  {
    next = 0;
    run0 = false;
  }
  else
    next = num;

  b = mem;
  // actually read from disk
  mem_read_blocks(b, bid_start, num);
}

void bufmgr1::test_integrity1()
{
  for(int i = 0; i<GLOBALS::num_parts; i++)
  {
    if (bid_blk_map[i]!=-1)
      if (blk_bid_map[bid_blk_map[i]] != i)
        do_nothing();
  }

  for(int i = 0; i<num_blks; i++)
  {
    if (blk_bid_map[i]!=-1)
      if (bid_blk_map[blk_bid_map[i]] != i)
        do_nothing();
  }
  for (int i = 0; i < num_blks; i++)
    if(blk_bid_map[i] != -1)
      if (blk_bid_map[i]!=get_block_id(blk_get_first_gid(mem_blks[i])))
        do_nothing();
}

template <typename T_>
bool bufmgr1::fill_non_cons_bv_list(block1 &b, bv_list_begin<T_>* p_bv_list_begin, int &num)
{
  int i, j;
  num = 0;
  assert(next != num_blks);
  b = mem_blks[next];

  for (i = 0; i < GLOBALS::num_parts && num < num_blks; i++)
  {
    if (p_bv_list_begin[i].begin_v != NULL)
    {
      if (blk_bid_map[next] != -1)
        bid_blk_map[blk_bid_map[next]] = -1;

      blk_bid_map[next] = i;
      num++;

      if (bid_blk_map[i] != -1) 
      {
        blk_bid_map[bid_blk_map[i]] = -1;
        memcpy(mem_blks[next], mem_blks[bid_blk_map[i]], GLOBALS::blk_size);
        bid_blk_map[i] = next++;
        if (next == num_blks)
        {
          next = 0;
          run0 = false;
        }
        continue;
      }

      bid_blk_map[i] = next++;
      
      if (next == num_blks)
      {
        next = 0;
        run0 = false;
        mem_read_block(mem_blks[bid_blk_map[i]], i);
      }
      else
      {
        j = i + 1;
        while (j < GLOBALS::num_parts && p_bv_list_begin[j].begin_v != NULL && bid_blk_map[j] == -1 && num < num_blks)
        {
          if (blk_bid_map[next] != -1)
            bid_blk_map[blk_bid_map[next]] = -1;

          /*if (bid_blk_map[j] != -1) 
            blk_bid_map[bid_blk_map[j]] = -1;*/

          blk_bid_map[next] = j;
          bid_blk_map[j++] = next++;
          num++;
          if (next == num_blks)
          {
            next = 0;
            run0 = false;
            break;
          }
        }
    
        mem_read_blocks(mem_blks[bid_blk_map[i]], i, j - i);
        //num += j - i;
        i = j - 1; 
      }
    }
  }

  return num != 0;
}

template bool bufmgr1::fill_non_cons_bv_list<bv_list_slt>(block1 &b, bv_list_begin<bv_list_slt>* p_bv_list_begin, int &num);
template bool bufmgr1::fill_non_cons_bv_list<bv_list_slt_seq>(block1 &b, bv_list_begin<bv_list_slt_seq>* p_bv_list_begin, int &num);
template bool bufmgr1::fill_non_cons_bv_list<bv_list_slt_start_with>(block1 &b, bv_list_begin<bv_list_slt_start_with>* p_bv_list_begin, int &num);


void bufmgr1::get_next_blk(block1 &b)
{
  if (b == last_blk)
    b = mem;
  else 
    b += GLOBALS::blk_size;
}

bool bufmgr1::in_mem(block1 &b, b_id bid)
{
  if (bid_blk_map[bid] == -1)
    return false;
  else
  {
    b = mem_blks[bid_blk_map[bid]];
    if (get_block_id(blk_get_first_gid(b)) != bid)
      print_endln();
    int c = get_block_id(blk_get_first_gid(b));
    //assert(get_block_id(blk_get_first_gid(b)) == bid);
    return true;
  }
}

void bufmgr1::get_block(block1 &b, b_id bid)
{
  if (in_mem(b, bid))
    return;

  assert(next != num_blks);

  //we have bid_blk_map[bid] == -1
  if (blk_bid_map[next] != -1)
    bid_blk_map[blk_bid_map[next]] = -1;
  bid_blk_map[bid] = next;
  blk_bid_map[next] = bid;
  b = mem_blks[next];
  mem_read_block(b, bid);

  inc_next();
}

void bufmgr1::get_block_w(block1 &b, b_id bid, bool is_empty)
{
  if (in_mem(b, bid))
    return;

  if (run0)
  {
    b = mem_blks[next];
    bid_blk_map[bid] = next;
    blk_bid_map[next] = bid;
    if (!is_empty) 
      mem_read_block(b, bid);
  }
  else
  {
    b_id old_bid = blk_bid_map[next];
    bid_blk_map[old_bid] = -1;    
    bid_blk_map[bid] = next;
    blk_bid_map[next] = bid;
    b = mem_blks[next];

    mem_write_block(b, old_bid);
    if (!is_empty) 
      mem_read_block(b, bid);
  }
  inc_next();
}

void bufmgr1::inc_next()
{
  next++;
  if (next==num_blks)
  {
    next = 0;
    run0 = false;
  }
}

void bufmgr1::write_memory()
{
  uint4 i, mem_end;

  if (run0)
    if (next == 0)
      return;
    else 
      mem_end = next;
    else 
      mem_end = num_blks;

    for (i = 0; i < mem_end; i++)
      bid_blk_map[blk_bid_map[i]] = -1;
  
  for (i = 0; i < mem_end; i++)
    mem_write_block(mem_blks[i], blk_bid_map[i]);
    
  for (i = 0; i < mem_end; i++)
    blk_bid_map[i] = -1;
}
