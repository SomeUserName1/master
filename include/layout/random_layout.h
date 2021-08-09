#ifndef RANDOM_LAYOUT_H
#define RANDOM_LAYOUT_H

#include "access/heap_file.h"

dict_ul_ul*
identity_partition(heap_file* hf);

dict_ul_ul*
random_partition(heap_file* hf);

#endif
