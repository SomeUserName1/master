/*!
 * \file htable.c
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief See \ref htable.h
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#include "data-struct/htable.h"

#include "data-struct/cbs.h"

HTABLE_IMPL(dict_ul_ul,
            unsigned long,
            unsigned long,
            fnv_hash_ul,
            unsigned_long_eq);
dict_ul_ul_cbs d_ul_cbs = { NULL, NULL, unsigned_long_print, unsigned_long_eq,
                            NULL, NULL, unsigned_long_print };

dict_ul_ul*
d_ul_ul_create(void)
{
    return dict_ul_ul_create(d_ul_cbs);
}

HTABLE_IMPL(dict_ul_d, unsigned long, double, fnv_hash_ul, unsigned_long_eq);
dict_ul_d_cbs d_d_cbs = { NULL, NULL, unsigned_long_print, double_eq,
                          NULL, NULL, double_print };

dict_ul_d*
d_ul_d_create(void)
{
    return dict_ul_d_create(d_d_cbs);
}

HTABLE_IMPL(dict_ul_int, unsigned long, int, fnv_hash_ul, unsigned_long_eq);
dict_ul_int_cbs d_int_cbs = { NULL, NULL,     unsigned_long_print, int_eq, NULL,
                              NULL, int_print };

dict_ul_int*
d_ul_int_create(void)
{
    return dict_ul_int_create(d_int_cbs);
}

