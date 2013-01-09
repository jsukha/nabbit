/* nabbit_array_defs.h                  -*-C++-*-
 *
 *************************************************************************
 *
 * Copyright (c) 2010, Jim Sukha
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the authors nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// Nabbit, Array Indexing Library

#ifndef __NABBIT_ARRAY_DEFS_H_
#define __NABBIT_ARRAY_DEFS_H_

#include <assert.h>
#include <stdint.h>

typedef int8_t ArrayLgDim;
typedef int32_t ArrayDim;
typedef uint64_t ArrayLargeDim;


// Hack to get unused parameter warnings to go away. 
#ifdef __GNUC__
#define NABBIT_ARRAY_UNUSED(x) x  __attribute__((__unused__))
#else
#define NABBIT_ARRAY_UNUSED(x) 
#endif


/******************************************************************/
// Define some simple math functions used when calculating indexing.

#define NABBIT_MAX(a, b) ((a) > (b) ? (a) : (b))
#define NABBIT_MIN(a, b) ((a) > (b) ? (a) : (b))


// Returns the smallest power of 2 which is >= n.
static inline ArrayLargeDim hyperceil(ArrayLargeDim n) {
  ArrayLargeDim x = n-1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  x = x | (x >> 32);
  return x+1;

  /*   uint64_t ans = 1; */
  /*   while ((ans < n) && (ans > 0)) { */
  /*     ans <<=1; */
  /*   } */
  /*   return (ArrayLargeDim)ans; */
}

// Returns the largest power of 2 which is <= n.
static inline ArrayLargeDim hyperfloor(ArrayLargeDim n) {
  int pos = 0;
  if (n >= 1ULL<<32) { n >>= 32; pos += 32; }
  if (n >= 1ULL<<16) { n >>= 16; pos += 16; }
  if (n >= 1ULL<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1ULL<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1ULL<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1ULL<< 1) {           pos +=  1; }
  return ((n == 0) ? 0 : (1 << pos));
}

// Returns lg(hyperfloor(n))
static inline int floor_lg(uint64_t n) {
  int pos = 0;
  if (n >= 1ULL<<32) { n >>= 32; pos += 32; }
  if (n >= 1ULL<<16) { n >>= 16; pos += 16; }
  if (n >= 1ULL<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1ULL<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1ULL<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1ULL<< 1) {           pos +=  1; }
  return ((n == 0) ? (-1) : pos);
}


// Returns ceil(x/block_size)
static inline ArrayLargeDim block_count(ArrayLargeDim x,
					ArrayLargeDim block_size) {
  return (x / block_size) + ((x % block_size != 0) ? 1 : 0);
}
 


#endif // __NABBIT_ARRAY_DEFS_H_
