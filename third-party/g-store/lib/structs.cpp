#include "structs.h"

/* Implementation of indexed_dequeue, used in bufmgr2. 
See also dissertation Listing 4. */
//#include <g-store.h>

// TODO: indexed_dequeue appears to work only with an even size. Check/Fix!
//this file currently contains only the implementation for indexed_dequeue

int indexed_dequeue::mem_requirements(int size_)
{
  return (sizeof(ideq_elem) + sizeof(ideq_elem*)) * size_;
}

void indexed_dequeue::init(void* mem, int size_)
{
  size = size_;
  ideq = (ideq_elem*) mem;
  pos = (ideq_elem**) (ideq + size);
}

void indexed_dequeue::reset(int cur_pos)
{
  int i;
  int center = cur_pos - size / 2;
  if (center < 0)
    center += size;

  ideq[0].pin_cnt = 0;
  ideq[0].left = NULL;

  int tmp;
  for (i = 1; i < size; i++)
  {
    ideq[i].left = &ideq[i-1];
    ideq[i-1].right = &ideq[i]; 
    ideq[i].pin_cnt = 0;
  }
  ideq[i-1].right = NULL;

  pos[center] = ideq;
  ideq[0].idx = center;
  for (i = 1; i < size/2; i++)
  {
    pos[tmp = ((center + i) % size)] = ideq + (i * 2);
    ideq[i * 2].idx = tmp;
    pos[tmp = (center - i >= 0 ? center - i : size - (i - center))] = ideq + (i * 2 - 1);
    ideq[i * 2 - 1].idx = tmp;
  }
  pos[tmp = (center - i >= 0 ? center - i : size - (i - center))] = ideq + (i * 2 - 1);
  ideq[i * 2 - 1].idx = tmp;

  ptr_begin = ideq;
  ptr_last_up = ptr_back = ideq + (size - 1);
}

// enqueues random-access element
void indexed_dequeue::pin_existing(int idx)
{
  ideq_elem* tmp = pos[idx];

  if (ptr_last_up == tmp)
    ptr_last_up = tmp->left;

  if (tmp == ptr_begin)
  {
    take_out_begin();
    insert_as_back(tmp);
  } 
  else if (tmp != ptr_back)
  {
    take_out_middle(tmp);
    insert_as_back(tmp);
  }

  tmp->pin_cnt++;
}

// enqueues leftmost element, returns its memory slot position
int indexed_dequeue::pin_new()
{
  ideq_elem* tmp = ptr_begin;

  if (tmp == ptr_last_up)
    ptr_last_up = NULL;

  take_out_begin();
  insert_as_back(tmp);

  tmp->pin_cnt = 1;
  return ptr_back->idx;
}

// decreases pin count and move if pincount == 0
void indexed_dequeue::unpin(int idx)
{
  ideq_elem* tmp = pos[idx];

  //this is possible, because pinned pages can be thrown out of the buffer
  if (tmp->pin_cnt <= 0)
    return;

  if (--(tmp->pin_cnt) == 0)
  {
    assert (tmp != ptr_last_up);
    if (ptr_last_up == NULL)
    {
      ptr_last_up = tmp;

      if (tmp == ptr_back)
      {
        take_out_back();
        insert_as_begin(tmp);
      }
      else if (tmp != ptr_begin)
      {
        take_out_middle(tmp);
        insert_as_begin(tmp);
      } 
    }
    else
    {
      assert (tmp != ptr_begin && ptr_last_up != ptr_back);
      if (tmp == ptr_back)
        take_out_back();
      else 
        take_out_middle(tmp);

      insert_after(tmp, ptr_last_up);
      ptr_last_up = tmp;
    }
  }
}

void indexed_dequeue::print_list_idx()
{
  ideq_elem* tmp = ptr_begin; 
  while (tmp != ptr_back)
  {
    printf("%d(%d) ", tmp->idx, tmp->pin_cnt);
    tmp = tmp->right;
  }

  printf("%d(%d)\n", ptr_back->idx, tmp->pin_cnt);
}

void indexed_dequeue::insert_as_back(ideq_elem* tmp)
{
  ptr_back->right = tmp;
  tmp->left = ptr_back;
  tmp->right = NULL;
  ptr_back = tmp;
}

void indexed_dequeue::insert_as_begin(ideq_elem* tmp)
{
  ptr_begin->left = tmp;
  tmp->right = ptr_begin;
  tmp->left = NULL;
  ptr_begin = tmp;
}

//this function may not be called with after_what = ptr_back
void indexed_dequeue::insert_after(ideq_elem* tmp, ideq_elem* after_what)
{
  if (after_what == ptr_back)
  {
    insert_as_back(tmp);
  }
  else
  {
    tmp->right = after_what->right;
    after_what->right->left = tmp;

    after_what->right = tmp;
    tmp->left = after_what;
  }
}

void indexed_dequeue::take_out_begin()
{
  ptr_begin = ptr_begin->right;
  ptr_begin->left = NULL;
}

void indexed_dequeue::take_out_back()
{
  ptr_back = ptr_back->left;
  ptr_back->right = NULL;
}

void indexed_dequeue::take_out_middle(ideq_elem* tmp)
{
  tmp->left->right = tmp->right;
  tmp->right->left = tmp->left;
}

void indexed_dequeue::do_nothing1()
{
  return;
}