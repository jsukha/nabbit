/* array2d_row.h                  -*-C++-*-
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


#ifndef __ARRAY2D_ROW__
#define __ARRAY2D_ROW__

#include <stdio.h>
#include <stdlib.h>

#include "array2d_base.h"

// An array of elements, stored in row-major order.


template <class T>
class NabbitArray2DRowMajor: public NabbitArray2DBase<T> {

 protected:
  int padding;
  ArrayDim rowsep;
  
 public:

  NabbitArray2DRowMajor(ArrayDim width,
			ArrayDim height,
			int padding=0);
/*   NabbitArray2DRowMajor(ArrayDim width, */
/* 			ArrayDim height) : NabbitArray2DRowMajor(width, height, 0) { } */
  
  void fill_with_constant_element(T element);
  void print();

  void print_layout();

  inline ArrayDim get_width();
  inline ArrayDim get_height();

  inline ArrayDim get_rowsep() { return rowsep; }


  inline T get(ArrayDim i,
	       ArrayDim j);

  inline void set(ArrayDim i, ArrayDim j, T val);

  ArrayLargeDim iterator();
  ArrayLargeDim iterator(ArrayDim row_num, ArrayDim col_num);

  inline ArrayLargeDim row_iterator();
  inline ArrayLargeDim row_iterator(ArrayDim row_num);
  void increment_row(ArrayLargeDim* idx);
  void prev_row(ArrayLargeDim* idx);
  inline bool has_prev_row(ArrayLargeDim* idx);  
  inline bool has_next_row(ArrayLargeDim* idx);
  ArrayDim row_idx(ArrayLargeDim* idx);


  inline ArrayLargeDim col_iterator();
  inline ArrayLargeDim col_iterator(ArrayDim col_num);
  void increment_col(ArrayLargeDim* idx);
  void prev_col(ArrayLargeDim* idx);
  inline bool has_prev_col(ArrayLargeDim* idx);
  inline bool has_next_col(ArrayLargeDim* idx);     
  ArrayDim col_idx(ArrayLargeDim* idx);
  
  
  inline T idx_get(ArrayLargeDim* row_idx,
		   ArrayLargeDim* col_idx);

  inline void idx_set(ArrayLargeDim* row_idx,
		      ArrayLargeDim* col_idx,
		      T val);
};



template <class T>
NabbitArray2DRowMajor<T>::NabbitArray2DRowMajor(ArrayDim test_width,
						ArrayDim test_height,
						int the_padding) :
  NabbitArray2DBase<T>(test_width, test_height, ROW_MAJOR_LAYOUT), padding(the_padding)
{
  assert(this->layout == ROW_MAJOR_LAYOUT);  
  this->rowsep = (this->width + this->padding);
  this->total_size = sizeof(T) * (ArrayLargeDim)(this->rowsep) * (ArrayLargeDim)this->height;  
  assert(this->total_size > 0);


  this->data = new T[this->total_size];
  assert(this->data != NULL);

#ifdef DEBUG_PRINT
  printf("Creating ROWMAJOR 2D array with width = %d, height = %d, ",
	 this->width, this->height);
  printf("layout = %d, padding = %d, rowsep = %d, total_size = %lld\n",
	 this->layout, this->padding, this->rowsep,
	 (size_t)this->total_size);  
#endif
}

template <class T>
void NabbitArray2DRowMajor<T>::fill_with_constant_element(T element) {
  assert(this->data);
  for(ArrayDim i = 0; i < this->height; i++) {
    for(ArrayDim j = 0; j < this->width; j++) {
      this->data[(this->width + this->padding)*i + j] = element;
    }  
  }
}

template <class T>
void NabbitArray2DRowMajor<T>::print() {
  printf("2D Array %p (width = %d, height = %d, ",
	 this, this->width, this->height);
  printf("layout = %d, padding = %d, total_size = %llu)\n",
	 this->layout, this->padding, this->total_size);

  if (this->total_size < 1000) {
    for (ArrayDim i = 0; i < this->width; i++) {
      printf("i = %3d: ", i);
      for (ArrayDim j = 0; j < this->width; j++) {
	printf("%4d ", get(i, j));
      }
      printf("\n");
    }
  }
}


template <class T>
void NabbitArray2DRowMajor<T>::print_layout() {
  printf("2D Array %p LAYOUT (width = %d, height = %d, ",
	 this, this->width, this->height);
  printf("layout = %d, padding = %d, total_size = %llu)\n",
	 this->layout, this->padding, this->total_size);

  if (this->total_size < 1000) {
    for (ArrayDim i = 0; i < (this->height); i++) {
      printf("i = %3d: ", i);
      for (ArrayDim j = 0; j < (this->width + this->padding); j++) {
	printf("%4d ", get(i, j));
      }
      printf("\n");
    }
  }
}

template <class T>
ArrayDim NabbitArray2DRowMajor<T>::get_width() {
  return this->width;
}

template <class T>
ArrayDim NabbitArray2DRowMajor<T>::get_height() {
  return this->height;
}



#define GET_ROW(i, j)(this->data[(i)*(this->rowsep) + (j)])
#define SET_ROW(i, j, val)(this->data[(i)*(this->rowsep) + (j)] = (val))

#define GET_COL(i, j)(this->data[(i) + (j)*(this->height+this->padding)])
#define SET_COL(i, j, val)(this->data[(i) + (j)*(this->height+this->padding)] = (val))


template <class T>
T NabbitArray2DRowMajor<T>::get(ArrayDim i,
			      ArrayDim j) {
  return GET_ROW(i, j);
}

template <class T>
void NabbitArray2DRowMajor<T>::set(ArrayDim i,
				 ArrayDim j,
				 T val) {
  SET_ROW(i, j, val);  
}


// Iterators and indexing.

template <class T>
ArrayLargeDim NabbitArray2DRowMajor<T>::iterator() {
  return 0;
}
template <class T>
ArrayLargeDim NabbitArray2DRowMajor<T>::iterator(ArrayDim row_num, ArrayDim col_num) {
  return row_num * this->row_sep + col_num;
}

template <class T>
ArrayLargeDim NabbitArray2DRowMajor<T>::row_iterator() {
  return 0;
}
template <class T>
ArrayLargeDim NabbitArray2DRowMajor<T>::row_iterator(ArrayDim row_num) {
  return row_num *(this->rowsep);
}

template <class T>
ArrayDim NabbitArray2DRowMajor<T>::row_idx(ArrayLargeDim* idx) {
  return (*idx)  / (this->rowsep);
}
template <class T>
bool NabbitArray2DRowMajor<T>::has_next_row(ArrayLargeDim* idx) {
  return ((*idx) + this->rowsep  <= ((ArrayLargeDim)this->height * (this->width + this->padding)));
}
template <class T>
void NabbitArray2DRowMajor<T>::increment_row(ArrayLargeDim* idx) {
  *idx += this->rowsep;
}
template <class T>
bool NabbitArray2DRowMajor<T>::has_prev_row(ArrayLargeDim* idx) {
  return (*idx - this->rowsep >= 0);
}
template <class T>
void NabbitArray2DRowMajor<T>::prev_row(ArrayLargeDim* idx) {
  *idx -= (this->row_sep);
}




template <class T>
ArrayLargeDim NabbitArray2DRowMajor<T>::col_iterator() {
  return 0;
}
template <class T>
ArrayLargeDim NabbitArray2DRowMajor<T>::col_iterator(ArrayDim col_num) {
  return col_num;
}
template <class T>
ArrayDim NabbitArray2DRowMajor<T>::col_idx(ArrayLargeDim* idx) {
  return (*idx)  % (this->width + this->padding);
}
template <class T>
bool NabbitArray2DRowMajor<T>::has_next_col(ArrayLargeDim* idx) {
  return (*idx) < (ArrayLargeDim)this->width;
}
template <class T>
void NabbitArray2DRowMajor<T>::increment_col(ArrayLargeDim* idx) {
  *idx = *idx + 1;
}
template <class T>
bool NabbitArray2DRowMajor<T>::has_prev_col(ArrayLargeDim* idx) {
  return (*idx >= 0);
}
template <class T>
void NabbitArray2DRowMajor<T>::prev_col(ArrayLargeDim* idx) {
  *idx --;
}

template <class T>
T NabbitArray2DRowMajor<T>::idx_get(ArrayLargeDim* row_idx, ArrayLargeDim* col_idx) {
  return this->data[*row_idx + *col_idx];
}

template <class T>
void NabbitArray2DRowMajor<T>::idx_set(ArrayLargeDim* row_idx,
				       ArrayLargeDim* col_idx,
				       T val) {
  this->data[*row_idx + *col_idx] = val;
}

#endif  // __ARRAY2D_ROW__

