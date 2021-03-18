#include "result_types.h"
#include <stdlib.h>

search_result_t*
create_search_result(dict_ul_int_t* search_numbers, dict_ul_ul_t* parents)
{
    if (!search_numbers || !parents) {
        exit(-1);
    }

    search_result_t* result = malloc(sizeof(*result));

    if (!result) {
        exit(-1);
    }

    result->search_numbers = search_numbers;
    result->parents = parents;

    return result;
}

void
search_result_destroy(search_result_t* result)
{
    dict_ul_int_destroy(result->search_numbers);
    dict_ul_ul_destroy(result->parents);
    free(result);
}
