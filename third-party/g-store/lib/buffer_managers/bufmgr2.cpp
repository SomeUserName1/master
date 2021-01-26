#include "bufmgr.h"

#include <assert.h>

#include "memory_mgr.h"
#include "util.h"
#include "block.h"

/* Extension of bufmgr1 for q_trav */
//#include <g-store.h>

bufmgr2::bufmgr2(int num_blks) : bufmgr1(num_blks, true)
{
  idq_space = malloc_b(idq.mem_requirements(num_blks), "todo324");
  idq.init(idq_space, num_blks);
}

bufmgr2::~bufmgr2()
{
  free(idq_space);
}

void bufmgr2::find_or_fill(block1 &b, b_id bid)
{
  if (bid_blk_map[bid] != -1)
  {
    b = mem_blks[bid_blk_map[bid]];
    assert(get_block_id(blk_get_first_gid(b)) == bid);
  }
  else
  {
    int num = 0;
    for (int i = bid; i < GLOBALS::num_parts && num < num_blks; i++, num++)
    {
      if (bid_blk_map[i] != -1)
        blk_bid_map[bid_blk_map[i]] = -1;

      if (blk_bid_map[num] != -1)
        bid_blk_map[blk_bid_map[num]] = -1;

      blk_bid_map[num] = i;
      bid_blk_map[i] = num;
    }
    b = mem;
    mem_read_blocks(b, bid, num);
    //test_integrity();
  }

}
void bufmgr2::test_integrity()
{
  /*for(int i = 0; i<GLOBALS::num_parts; i++)
  {
    if (bid_blk_map[i]!=-1)
      if (blk_bid_map[bid_blk_map[i]] != i)
        assert(false);
  }

  for(int i = 0; i<num_blks; i++)
  {
    if (blk_bid_map[i]!=-1)
      if (bid_blk_map[blk_bid_map[i]] != i)
        assert(false);
  }*/

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
void bufmgr2::in_mem_or_add_pin(block1 &b, b_id bid)
{
  if (bid_blk_map[bid] == -1)
  {
    int tmp = idq.pin_new();

    if (blk_bid_map[tmp] != -1)
      bid_blk_map[blk_bid_map[tmp]] = -1;

    // new
    //if (bid_blk_map[bid] != -1)
    //  blk_bid_map[bid_blk_map[bid]] = -1;
    // new end

    blk_bid_map[tmp] = bid;
    bid_blk_map[bid] = tmp;
    b = mem_blks[tmp];
    mem_read_block(b, bid);
    assert(get_block_id(blk_get_first_gid(b)) == bid);
  }
  else
  {
    idq.pin_existing(bid_blk_map[bid]);
    b = mem_blks[bid_blk_map[bid]];
    assert(get_block_id(blk_get_first_gid(b)) == bid);
  }
}

//returns true if changed position
bool bufmgr2::confirm_block(b_id bid, int &assumed_mem_slt, block1 &b, header1 &h, char* &ptr_in_b)
{
  if (blk_bid_map[assumed_mem_slt] == bid)
  {
    assert(mem_blks[bid_blk_map[bid]] == b && get_block_id(blk_get_first_gid(b)) == bid);
    return false;
  }

  else if(bid_blk_map[bid] != -1)
  {
    //assert(get_block_id(blk_get_first_gid(b)) == bid);
    int tmp = bid_blk_map[bid];
    if (tmp > assumed_mem_slt)
    {
      uint4 offset = (tmp - assumed_mem_slt) * GLOBALS::blk_size;
      b += offset;
      h += offset;
      ptr_in_b += offset;
    } 
    else
    {
      uint4 offset = (assumed_mem_slt - tmp) * GLOBALS::blk_size;
      b -= offset;
      h -= offset;
      ptr_in_b -= offset;
    }
    assumed_mem_slt = tmp;
    return true;
  }
  else
  {
    assumed_mem_slt = idq.pin_new();
    int offset_h = h - b;
    int offset_ptr = ptr_in_b - b;

    if (blk_bid_map[assumed_mem_slt] != -1)
      bid_blk_map[blk_bid_map[assumed_mem_slt]] = -1;

    // new
    //if (bid_blk_map[bid] != -1)
    //  blk_bid_map[bid_blk_map[bid]] = -1;
    // new end

    blk_bid_map[assumed_mem_slt] = bid;
    bid_blk_map[bid] = assumed_mem_slt;
    b = mem_blks[assumed_mem_slt];
    mem_read_block(b, bid);
    h = b + offset_h;
    ptr_in_b = b + offset_ptr;
    return true;
  }
}

void bufmgr2::pin(int mem_slt)
{ 
  assert (mem_slt != -1);
  idq.pin_existing(mem_slt);
}

void bufmgr2::unpin(int assumed_mem_slt)
{
  if (blk_bid_map[assumed_mem_slt] != -1)
    idq.unpin(assumed_mem_slt);
}

void bufmgr2::start_traverse(b_id bid)
{
  assert (bid_blk_map[bid] != -1);
  idq.reset(bid_blk_map[bid]);
}

int bufmgr2::get_mem_slot_num(b_id bid)
{
  assert (bid_blk_map[bid] != -1);
  return bid_blk_map[bid];

}