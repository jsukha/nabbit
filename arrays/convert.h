/* convert.h                  -*-C++-*-
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

#ifndef __CONVERT_H_
#define __CONVERT_H_

#include "block_layouts.h"
#include "morton.h"


/***********************************************************************
 * Some simple routines for allocating, converting, and comparing
 * arrays in (possibly different) layouts.
 *
 * TODO: These methods are not optimized (or parallelized).  For now,
 * I've only used these methods for checking for correctness and
 * setting up test cases.
 * 
 */ 


/**
 * Prints an entire block (of size block_n by block_m) to std::cout.
 *
 * For class T, the operator << must be overloaded.
 *
 * inner_layout specifies the layout for the block.
 */
template <class T>
void NabbitPrintInnerBlock(T* A,
			   ArrayLargeDim block_n,
			   ArrayLargeDim block_m,
			   NabbitArray2DLayout inner_layout);


/**
 * Prints the subblock of size (n by m), from a complete block of size
 * (block_n by block_m).  The subblock is aligned so its upper left
 * corner matches the upper left corner of the outer block.
 *
 * For class T, the operator << must be overloaded.
 *
 * inner_layout specifies the layout for the block.
 *
 * NOTE: The method currently prints the contents of the entire
 * (block_n by block_m) block, but denotes the elements which are out
 * of bounds with an "*".
 */
template <class T>
void NabbitPrintPartialBlock(T* A,
			     ArrayLargeDim n,
			     ArrayLargeDim m,
			     ArrayLargeDim block_n,
			     ArrayLargeDim block_m,
			     NabbitArray2DLayout inner_layout);


/**
 * Prints an n by m array arranged in the blocked-layout described by
 * "layout".
 *
 * For class T, the operator << must be overloaded.
 */
template <class T>
void NabbitPrintArray(T* A,
		      ArrayLargeDim n, 
		      ArrayLargeDim m,
		      BlockLayout* layout);



/**
 * Allocates memory for an n by m array of elements, according to the
 * specified layout.
 *
 *  elemsize:           Size (in bytes) of each element
 *  n, m:               Dimensions of the array to allocate 
 *  block_n, block_m:   Dimensions of a block at the lower level
 *  block_padding:      How many extra elements to allocate in each block
 *                      (so each block can store at least
 *                      block_n*block_m + block_padding elements)
 *  inner_layout:       The desired layout for elements within a block.
 *  outer_layout:       The desired layout for blocks.
 *  align_for_SSE:      Should be true if we want to align
 *                      the beginning of the array for SSE (16-byte alignment)
 *                      False if we don't care.
 *                      NOTE: to use SSE I think we need either
 *                      elemsize to divide 16 or be a multiple of 16.
 */ 
static inline void* NabbitAllocateArray(size_t elemsize,
					ArrayLargeDim n,
					ArrayLargeDim m,
					ArrayLargeDim block_n,
					ArrayLargeDim block_m,
					ArrayDim block_padding,
					NabbitArray2DLayout inner_layout,
					NabbitArray2DLayout outer_layout,
					bool align_for_SSE);
/**
 * Frees an array allocated by NabbitAllocateArray.
 */
static inline void NabbitFreeArray(void* A);


/**
 * Converts an n by m matrix from one layout to another.
 *
 * n:             Number of rows in input_A and output_A arrays
 * m:             Number of columns in input_A and output_A arrays
 * input_A:       Pointer to the input array
 * input_layout:  Description of the layout of the input array.
 * output_A:      Pointer to the output array
 * output_layout: Description of the layout of the output array.
 *
 * After this method completes, the n by m grid of elements in input_A
 * are copied into output_A.
 *
 * NOTE: This method assumes input_A and output_A do not overlap.  It
 *       may not work correctly if the arrays overlap.  You can't use
 *       this method to do an in-place transpose.
 *
 * TODO: This method is not optimized.  Instead, it relies on generic
 *       indexing, and doesn't know what the layouts are.  In fact, we
 *       probably want special cases for conversions between methods
 *       that we use often.
 */
template <class T>
void NabbitArrayCopyConvert(ArrayLargeDim n,
			    ArrayLargeDim m,
			    T* input_A,
			    BlockLayout* input_layout,
			    T* output_A,
			    BlockLayout* output_layout);

/**
 * Returns sum of squared differences two blocked arrays A1 and A2.
 *
 * n:            number of rows in A1 and A2
 * m:            number of columns in A1 and A2
 * A1:           block 1
 * A2:           block 2
 * layout1:      description of layout of A1
 * layout2:      description of layout of A2
 *
 *  Returns sum( (A1(i, j) - A2(i, j))^2), where the sum is over all
 *  (i, j) in [0, Bn-1] x [0, Bm-1].

 *  sum( (input_A(i, j) - output_A(i, j))^2), where the sum is over
 *  all (i, j) in [0, n-1] x [0, m-1].
 */ 
template <class T>
T NabbitArraySquaredDiff(ArrayLargeDim n,
			 ArrayLargeDim m,
			 T* A1,
			 BlockLayout* layout1,
			 T* A2,
			 BlockLayout* layout2);

/**
 * Returns sum of squared differences between two blocks A1 and A2.
 *
 * Bn:           number of rows in blocks A1 and A2
 * Bm:           number of columns in blocks A1 and A2
 * A1:           block 1
 * A2:           block 2
 * layout1:      (simple) layout of A1 
 * layout2:      (simple) layout of A2
 *
 *  Returns sum( (A1(i, j) - A2(i, j))^2), where the sum is over all
 *  (i, j) in [0, Bn-1] x [0, Bm-1].
 */ 
template <class T>
T NabbitInnerBlockSquaredDiff(ArrayLargeDim Bn,
			      ArrayLargeDim Bm,
			      T* A1,
			      NabbitArray2DLayout input_layout,			      
			      T* A2,
			      NabbitArray2DLayout output_layout);

/**
 * Similar to NabbitArraySquaredDiff, for T = double.  
 *
 * Also prints out extra information for any element which satisfies *
 * (A1(i, j) - A2(i, j))^2 > threshhold
 *
 * Currently, this method is useful for comparing whether A1 and A2
 * are the same.
 */
double NabbitArrayReportDiffDouble(ArrayLargeDim n,
				   ArrayLargeDim m,
				   const double* A1,
				   BlockLayout* layout1,
				   const double* A2,
				   BlockLayout* layout2,
				   double threshhold);




/**********************************************************************/
// Implementation of print and conversion methods.

template <class T>
void NabbitPrintInnerBlock(T* A,
			   ArrayLargeDim block_n,
			   ArrayLargeDim block_m,
			   NabbitArray2DLayout inner_layout) {

  std::cout << "Block " << A << ": \n";  
  for (ArrayLargeDim i = 0; i < block_n; i++) {
    for (ArrayLargeDim j = 0; j < block_m; j++) {
      ArrayLargeDim idx = NabbitIndexingWrapper::get_idx(i, j,
							 block_n, block_m,
							 inner_layout);

      std::cout << "(" << i << ", " << j << ", " << idx << ") ";
      std::cout << A[idx] << " ";
    }
    std::cout << "\n";
  }
}


template <class T>
void NabbitPrintPartialBlock(T* A,
			     ArrayLargeDim n,
			     ArrayLargeDim m,
			     ArrayLargeDim block_n,
			     ArrayLargeDim block_m,
			     NabbitArray2DLayout inner_layout) {
  std::cout << "Block " << A << ": \n";
  for (ArrayLargeDim i = 0; i < block_n; i++) {
    for (ArrayLargeDim j = 0; j < block_m; j++) {

      ArrayLargeDim idx = NabbitIndexingWrapper::get_idx(i, j,
							 block_n, block_m,
							 inner_layout);
      if ((i >= n) || (j >= m)) {
	std::cout << "*";
      }
      else {
	std::cout << "(Idx" << idx << ") " << A[idx] ;
      }
      std::cout << "  ";
    }
    std::cout << "\n";
  }    
}


template <class T>
void NabbitPrintArray(T* A,
		      ArrayLargeDim n, 
		      ArrayLargeDim m,
		      BlockLayout* layout) {

  std::cout << "Array " << A << ": \n";
  std::cout << "----------------------------------\n";
  for (ArrayLargeDim i = 0; i < n; i++) {
    if (i % layout->sub_n == 0) {
      std::cout << "- - - - - - - - - - - - - - - \n";
    }
    for (ArrayLargeDim j = 0; j < m; j++) {

      if (j % layout->sub_m == 0) {
	std::cout << " ";
      }

      ArrayLargeDim idx = layout->Idx(i, j);

      std::cout << "(Idx " << idx << "): " << A[idx] << " ";
    }
    std::cout << "\n";
  }
  std::cout << "----------------------------------\n";
}


static inline void* NabbitAllocateArray(size_t elemsize,
					ArrayLargeDim n,
					ArrayLargeDim m,
					ArrayLargeDim block_n,
					ArrayLargeDim block_m,
					ArrayDim block_padding,
					NabbitArray2DLayout inner_layout,
					NabbitArray2DLayout outer_layout,
					bool align_for_SSE) {

  ArrayLargeDim asize = BlockLayout::MinArraySize(n, m,
						  block_n, block_m,
						  block_padding,
						  inner_layout,
						  outer_layout);
  void* A = NULL;
#ifdef _WIN32
  (void) align_for_SSE;  // UNUSED for Windows.
  // For Windows, we always align it, because there is no easy way to 
  // distinguish between aligned and non-aligned when we are trying to
  // free.
  //
  // TBD: The interface for this entire array class should be reworked
  // anyway...
  A = _aligned_malloc(asize * elemsize, 16);
#else
  if (align_for_SSE) {
      int test = posix_memalign(&A, 16, asize * elemsize);
      assert(test == 0);
      assert(A);
      assert(((size_t)A & 15) == 0);
  }
  else {
    assert(A);
    A = malloc(asize * elemsize);
  }
#endif
  return A;
}

static inline void NabbitFreeArray(void* A) {
#ifdef _WIN32
    _aligned_free(A);
#else
    free(A);
#endif    
}

bool NabbitCheckSSEAlignment(void* A) {
  if (((int64_t)A & 15)) {
    printf("WARNING: ptr %p not 16-byte aligned \n", A);
    return false;
  }
  return true;
}




template <class T>
void NabbitArrayCopyConvert(ArrayLargeDim n,
			    ArrayLargeDim m,
			    T* input_A,
			    BlockLayout* input_layout,
			    T* output_A,
			    BlockLayout* output_layout) {

  ArrayLargeDim i, j;
  ArrayLargeDim input_idx, output_idx;  
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      input_idx = input_layout->Idx(i, j);
      output_idx = output_layout->Idx(i, j);     
      output_A[output_idx] = input_A[input_idx];
    }
  }
}

template <class T>
T NabbitArraySquaredDiff(ArrayLargeDim n,
			 ArrayLargeDim m,
			 T* input_A,
			 BlockLayout* input_layout,
			 T* output_A,
			 BlockLayout* output_layout) {

  T sum = 0;
  ArrayLargeDim i, j;
  ArrayLargeDim input_idx, output_idx;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      T diff;
      input_idx = input_layout->Idx(i, j);
      output_idx = output_layout->Idx(i, j);
      diff = (output_A[output_idx] - input_A[input_idx]);
      sum += diff * diff;
    }
  }
  return sum;
}




template <class T>
T NabbitInnerBlockSquaredDiff(ArrayLargeDim Bn,
			      ArrayLargeDim Bm,
			      T* input_A,
			      NabbitArray2DLayout input_layout,			      
			      T* output_A,
			      NabbitArray2DLayout output_layout) {
  T sum = 0;
  ArrayLargeDim i, j;
  ArrayLargeDim input_idx, output_idx;
  for (i = 0; i < Bn; i++) {
    for (j = 0; j < Bm; j++) {
      T diff;
      input_idx = NabbitIndexingWrapper::get_idx(i, j, Bn, Bm, input_layout);
      output_idx = NabbitIndexingWrapper::get_idx(i, j, Bn, Bm, output_layout);
      diff = (output_A[output_idx] - input_A[input_idx]);
      sum += diff*diff;
    }
  }
  return sum;
}



double NabbitArrayReportDiffDouble(ArrayLargeDim n,
				   ArrayLargeDim m,
				   const double* input_A,
				   BlockLayout* input_layout,
				   const double* output_A,
				   BlockLayout* output_layout,
				   double threshhold) {
  double sum = 0;
  ArrayLargeDim i, j;
  ArrayLargeDim input_idx, output_idx;
  int num_diffs = 0;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      double diff;

      input_idx = input_layout->Idx(i, j);
      output_idx = output_layout->Idx(i, j);

      diff = (output_A[output_idx] - input_A[input_idx]);

      if (diff * diff > threshhold) {
	num_diffs++;
	if (num_diffs < 10000) {
	  printf("Diff = %f (%llu, %llu): Aout[%llu] = %f, Ain[%llu] = %f\n",
		 diff,
		 i, j,
		 output_idx, output_A[output_idx],
		 input_idx, input_A[input_idx]);
	}
      }
      sum += diff*diff;
    }
  }

  if (num_diffs) {
    printf("Total differences: %d. sum = %f\n", num_diffs, sum);
  }
  return sum;    
}

#endif // __CONVERT_H_
