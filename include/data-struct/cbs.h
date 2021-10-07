/*!
 * \file cbs.h
 * \version 1.0
 * \date Sep 15, 2021
 * \author Fabian Klopfer <fabian.klopfer@ieee.org>
 * \brief Callback functions for different types, used in various data
 * structures.
 *
 * \copyright Copyright (c) 2021- University of Konstanz.
 * This software is the proprietary information of the above-mentioned
 * institutions. Use is subject to license terms. Please refer to the included
 * copyright notice.
 */
#ifndef CBS_H
#define CBS_H

#include <stdbool.h>
#include <stddef.h>

/*!
 *  Compares the addresses of two pointers to each other.
 *  Wrapper arround the standard == operator.
 *
 *  \param a The first pointer to compare.
 *  \param b The second pointer to compare.
 *  \return true if the pointers are refering to the same address, false
 * otherwise.
 */
bool
ptr_eq(const void* a, const void* b);

/*!
 * Popular hash function used for hash-based data structures.
 * For details see Fowler-Noll-Vo hash function, implemented is the version
 * FNV-1a
 * (https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function).
 *
 * \param in Value to be hashed.
 * \param seed A seed to avoid homogeneous behviour in all hash maps when it
 * comes to collision probabilities.
 * \return An unsigned long hash based on the input value \p in
 */
size_t
fnv_hash_ul(unsigned long in, unsigned int seed);

/*!
 *  Compares the value of two unsigned longs to each other.
 *  Wrapper arround the standard == operator.
 *
 *  \param a The first unsigned long to compare.
 *  \param b The second unsigned long to compare.
 *  \return true if the values are the same, false otherwise.
 */
bool
unsigned_long_eq(unsigned long a, unsigned long b);

/*!
 *  A function to specify how to print unsigned longs.
 *  A wrapper arroung printf("%lu\n");
 *
 *  \param in The unsigned long to be printed.
 */
void
unsigned_long_print(unsigned long in);

/*!
 * A comparator for unsigned longs, complying to the interface that qsort from
 * the GNU extentions requires.
 *
 * \param a the first value to compare.
 * \param b the second value to compare.
 * \return A value >0 if \p a is larger, 0 if the values are equal and a value
 * <0 if b is larger.
 */
int
ul_cmp(const void* a, const void* b);

/*!
 *  Compares the value of two integers to each other.
 *  Wrapper arround the standard == operator.
 *
 *  \param first The first integer to compare.
 *  \param second The second integer to compare.
 *  \return true if the values are the same, false otherwise.
 */
bool
int_eq(int first, int second);

/*!
 *  A function to specify how to print integers.
 *  A wrapper arroung printf("%d\n");
 *
 *  \param in The integer to be printed.
 */
void
int_print(int in);

/*!
 *  Compares the value of two longs to each other.
 *  Wrapper arround the standard == operator.
 *
 *  \param first The first long to compare.
 *  \param second The second long to compare.
 *  \return true if the values are the same, false otherwise.
 */
bool
long_eq(long first, long second);

/*!
 *  A function to specify how to print longs.
 *  A wrapper arroung printf("%l\n");
 *
 *  \param in The long to be printed.
 */
void
long_print(long in);

/*!
 *  Compares the value of two doubles to each other.
 *  Compares the absolute difference between the doubles to the machine epsilon
 * scaled by the max value of the parameters.
 *
 *  \param first The first pointer to compare.
 *  \param second The second pointer to compare.
 *  \return true if the values are the same, false otherwise.
 */
bool
double_eq(double first, double second);

/*!
 *  A function to specify how to print doubles.
 *  A wrapper arroung printf("%lf\n");
 *
 *  \param in The double to be printed.
 */
void
double_print(double in);

#endif
