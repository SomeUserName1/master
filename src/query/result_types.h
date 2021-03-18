#ifndef RESULT_TYPES
#define RESULT_TYPES

#include "../data-struct/dict_ul.h"

typedef struct search_result
{
    dict_ul_int_t* search_numbers;
    dict_ul_ul_t* parents;
} search_result_t;

search_result_t*
create_search_result(dict_ul_int_t* search_numbers, dict_ul_ul_t* parents);
void
search_result_destroy(search_result_t* result);

#endif
