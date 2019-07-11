/* matrix_utils.h                 -*-C++-*-
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

// Code for the Nabbit task graph library
//
// Miscellaneous methods for copying matrices and converting between
// layouts.

#ifndef __MATRIX_UTILS_H
#define __MATRIX_UTILS_H

#include <arrays/array2d_row.h>
#include <arrays/array2d_morton.h>


#define MAX(m, n) ((m) > (n) ? (m) : (n))


/******************************************************/
// Simple matrix functions.

// Fill a 1D array with random elements.
void fill_random_1D(int* a, int length, int range) {
  for (int i = 0; i < length; i++) {
    a[i] = rand() % range;    
  }
}


// Fill a 2D array with random elements.
template <class MatrixType>
void fill_random_2D(MatrixType* M, int range) {
  
  int i = 0;
  for (ArrayLargeDim row_idx = M->row_iterator();
       M->has_next_row(&row_idx);
       M->increment_row(&row_idx)) {
    int j = 0;

    for (ArrayLargeDim col_idx = M->col_iterator();
	 M->has_next_col(&col_idx);
	 M->increment_col(&col_idx)) {

      int tval = rand() % range;

      M->idx_set(&row_idx,
		 &col_idx,
		 tval);
      assert(M->get(i, j) == tval);
      j++;
    }
    assert(j == M->get_width());
    i++;
  }
  assert(i == M->get_height());
}


// Copy from one matrix to another; the two matrices can use
// different layouts.
template <class MatrixType1, class MatrixType2>
void copy_2D(MatrixType1* M, MatrixType2* M2) {

  assert(M->get_width() == M2->get_width());
  assert(M->get_height() == M2->get_height());
  
  int i = 0;
  for (ArrayLargeDim row_idx = M->row_iterator();
       M->has_next_row(&row_idx);
       M->increment_row(&row_idx)) {
    int j = 0;

    for (ArrayLargeDim col_idx = M->col_iterator();
	 M->has_next_col(&col_idx);
	 M->increment_col(&col_idx)) {

      int tval = M->idx_get(&row_idx,
			    &col_idx);
      M2->set(i, j, tval);
      j++;
    }
    assert(j == M->get_width());
    i++;
  }
  assert(i == M->get_height());
}


// Sets zero'th row and columns to 0.
template <class MatrixType>
void zero_top_and_left_borders(MatrixType* M) {
  for (int j = 0; j < M->get_width(); j++) {
    M->set(0, j, 0);
  }
  for (int i = 0; i < M->get_height(); i++) {
    M->set(i, 0, 0);
  }
}


#endif // __MATRIX_UTILS_H
