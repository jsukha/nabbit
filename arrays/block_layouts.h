/* block_layouts.h                  -*-C++-*-
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

#ifndef __BLOCK_LAYOUTS_H_
#define __BLOCK_LAYOUTS_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "nabbit_array_defs.h"
#include "array_layouts.h"


/*********************************************************************
 * The BlockLayout class gives a description of a particular layout of
 * a 2-level blocked array.
 * 
 * Conceptually, the layout is for an n by m array, whose elements are
 * grouped into blocks of size (up to) sub_n by sub_m.  Each block is
 * stored within a block of "block_size" elements.
 * 
 * Thus, we must have block_size >= sub_n * sub_m.  (For some layouts
 * of blocks, we may need padding within blocks, so block_size may be
 * bigger).
 *
 * (Note that block_size is defined in terms of number of elements,
 *  not # of bytes.  This definition won't allow the case where
 *  elements are a strange size, and we want to pad a block by a
 *  certain number of bytes which doesn't divide the element size.
 *  This might be useful we need to maintain some alignment on blocks
 *  of elements, but not on the individual elements themselves?
 *
 *  For now, however, the since I'm only using int or double, I
 *  haven't run into problems yet.)
 *  
 * Blocks on the right and bottom edges of the outer array are
 * conceptually padded to fill out to a complete block.
 * 
 *
 *
 * The elements within a block are arranged in memory according to the
 * layout defined by "inner_layout".
 * Similarly, the blocks themselves are arranged in memory according
 * to some (possibly different) layout.
 *
 * For example, for an n by m matrix:
 *
 *  outer_layout = MORTON_LAYOUT
 *  sub_n = 3
 *  sub_m = 7
 *  block_size = 24
 *  inner_layout = ROW_MAJOR_LAYOUT
 *
 *  corresponds to blocks of size 3 by 7, with each block stored in 24
 *  elements-worth of memory (or said differently, consecutive blocks
 *  are spaced 24 elements apart).  Each block is stored in a
 *  row-major layout, but the blocks are arranged in a Morton layout.
 *
 */

// Defines the structure of a block.
// This block may be recursively organized into sub-blocks.
class BlockLayout {

 public:
  ArrayLargeDim n;
  ArrayLargeDim m;

  // Layout of blocks in memory.
  NabbitArray2DLayout outer_layout; 

  // Sizes of the sub-blocks.
  ArrayLargeDim sub_n;  
  ArrayLargeDim sub_m;

  // block_size is the size of memory (in # of elements)
  // used to store each block.
  // If you are being careful / clever, it may sometimes be
  // used to represent the separation between contiguous blocks in
  // memory.
  //
  // Must be at least sub_n * sub_m
  ArrayLargeDim block_size;

  // Layout within each block.
  NabbitArray2DLayout inner_layout;


  
  // Computes the minimum number of elements we need to allocate
  // space for to store a block of 
  //    block_n by block_m elements,
  // with extra padding for "block_padding" elements,
  // in the specified layout.
  //
  static inline ArrayLargeDim MinBlockSize(ArrayLargeDim block_n,
					   ArrayLargeDim block_m,
					   ArrayDim block_padding,
					   NabbitArray2DLayout block_layout);

  // Computes the minimum number of elements we need to allocate
  // space for to store an 
  //    n by m matrix,
  //    with (block_n by block_m)-size blocks,
  //    using the specified layouts.
  // Also, each block should have sufficient padding at the end for
  //  "block_padding" elements. 
  // 
  // For any of the Morton-order layouts, we currently round the
  // size up to next largest square with dimensions = power of 2.
  //
  static inline ArrayLargeDim MinArraySize(ArrayLargeDim n,
					   ArrayLargeDim m,
					   ArrayLargeDim block_n,
					   ArrayLargeDim block_m,
					   ArrayDim block_padding,
					   NabbitArray2DLayout inner_layout,
					   NabbitArray2DLayout outer_layout);


  // Returns true if the layout structure is valid.
  //
  // Mainly, this method checks that "block_size"
  // is greater than the MinBlockSize() needed
  // handle a block of size sub_n by sub_m.
  //
  // It also checks for some other simple conditions.
  bool IsValidLayout() const;
  
  // Initializes this layout as a single block.
  // In other words, the outer level is a single block.
  // The inner level is a block of size n by m, in the specified layout type,
  // with space for "block_padding" padded elements.
  //
  // Returns the total size needed for this array.
  inline ArrayLargeDim InitSingleBlock(ArrayLargeDim n, ArrayLargeDim m,
				       ArrayDim block_padding,
				       NabbitArray2DLayout inner_layout_type);

  
  // Initializes this layout as a rowmajor layout.
  // The outer_layout is ROW_MAJOR_LAYOUT
  // 
  // The inner level is stored as 1 by 1 "blocks"
  // The inner layout doesn't matter since block dimension is 1 by 1.
  //
  // Returns the total size to allocate for the array.
  inline ArrayLargeDim InitAsRowmajor(int n, int m);




  // Given an array index, returns the row (in the outer layout)
  // for the block that contains the element at the index.
  inline ArrayLargeDim OuterRow(ArrayLargeDim idx) const;

  // Given an array index, returns the col (in the outer layout)
  // for the block that contains the element at the index.
  inline ArrayLargeDim OuterCol(ArrayLargeDim idx) const;

  // Given an array index, returns the row within the block for that
  // index.
  inline ArrayLargeDim InnerRow(ArrayLargeDim idx) const;

  // Given an array index, returns the column within the a block
  // for that index.
  inline ArrayLargeDim InnerCol(ArrayLargeDim idx) const;


  // Converts the row/col index within a block into
  // an array index (i.e., offset) within the block.
  inline ArrayLargeDim InnerIdx(ArrayDim si, ArrayDim sj) const;
 

  // Converts a row and column index into a complete
  // index for the block.
  inline ArrayLargeDim Idx(ArrayDim i, ArrayDim j) const;
  
};






/**********************************************************************/
// Implementation of BlockLayout methods.

inline ArrayLargeDim BlockLayout::MinBlockSize(ArrayLargeDim block_n,
					       ArrayLargeDim block_m,
					       ArrayDim block_padding,
					       NabbitArray2DLayout inner_layout) {
  ArrayDim extra_padding = 0;
  if (inner_layout == MORTON_LAYOUT) {
    ArrayLargeDim round_sn = hyperceil(block_n);
    ArrayLargeDim round_sm = hyperceil(block_m);
    if (round_sn > round_sm) {
      round_sm = round_sn;
    }
    else {
      round_sn = round_sm;
    }
    extra_padding = (round_sn * round_sm) - (block_n*block_m);
    assert((ArrayLargeDim)block_n <= round_sn);
    assert((ArrayLargeDim)block_m <= round_sm);    
  }
  return (block_n*block_m + block_padding + extra_padding);
}

// Compute the size of an array required for a generic layout.
// For any of the Morton-order layouts, we need to round the
// size up to next largest square with dimensions = power of 2.
// 
// For an inner layout, this introduces extra "padding" in the size of
// an inner block.
// For an outer layout, this just rounds n and m up to
// hyperceil(max(n, m)).
//
inline ArrayLargeDim BlockLayout::MinArraySize(ArrayLargeDim n,
					       ArrayLargeDim m,
					       ArrayLargeDim block_n,
					       ArrayLargeDim block_m,
					       ArrayDim block_padding,
					       NabbitArray2DLayout inner_layout,
					       NabbitArray2DLayout outer_layout) {
  
  ArrayLargeDim min_block_size = MinBlockSize(block_n, block_m,
					      block_padding,
					      inner_layout);  
  // Compute number of blocks we need at the outer level.
  ArrayLargeDim nB = block_count(n, block_n);
  ArrayLargeDim mB = block_count(m, block_m);
  ArrayLargeDim number_blocks = MinBlockSize(nB, mB,
					     0,
					     outer_layout);
  
  return number_blocks * min_block_size;  
}






// Checks if the layout structure is valid.
// Aside from simple checks, to be valid, we need block_size to be
// large enough to handle a block of size sub_n by sub_m.
// 
bool BlockLayout::IsValidLayout() const {
  if (this->outer_layout == OTHER_LAYOUT) return false;
  if (this->inner_layout == OTHER_LAYOUT) return false;

  switch (this->inner_layout) {
  case ROW_MAJOR_LAYOUT:
  case COL_MAJOR_LAYOUT:
    {
      if (this->block_size < (this->sub_n * this->sub_m)) {
	printf("this->block_size = %llu, expected = %llu\n",
	       this->block_size,
	       this->sub_n * this->sub_m);
	return false;
      }
    }
    break;
  case MORTON_LAYOUT: 
    {
      ArrayLargeDim test_size = 1;
      test_size = hyperceil(NABBIT_MAX(this->sub_n,
				       this->sub_m));
      if (test_size <= 0) {
	printf("test_size here is %llu\n",
	       test_size);
	return false;
      }
      // For the cache-oblivious layout, the

      if (this->block_size < (test_size * test_size)) {
	printf("this->sub_n = %llu, this->sub_m = %llu\n",
	       this->sub_n, this->sub_m);
	printf("test_size2 here is %llu, this->block_size is %llu\n",
	       test_size,
	       this->block_size);
	return false;
      }
    }
    break;
  case OTHER_LAYOUT:
  default:
    return false;
  }  
  return true;
}


/* static void InitBlockLayout(BlockLayout* L, */
/* 			    int n, int m, */
/* 			    NabbitArray2DLayout inner_layout_type,  */
/* 			    ArrayLargeDim block_size) { */



// Initializes this layout as a single block.
// In other words, the outer level is a single block.
// The inner level is a block of size n by m,
// in the specified layout type.
inline ArrayLargeDim BlockLayout::InitSingleBlock(ArrayLargeDim n, ArrayLargeDim m,
						  ArrayDim block_padding,
						  NabbitArray2DLayout inner_layout_type) {
  this->n = n;
  this->m = m;
  this->outer_layout = ROW_MAJOR_LAYOUT;
  this->sub_n = n;
  this->sub_m = m;
  this->block_size = BlockLayout::MinBlockSize(n, m, block_padding, inner_layout_type);
  this->inner_layout = inner_layout_type;
  assert(IsValidLayout());

  return this->block_size;
}




inline ArrayLargeDim BlockLayout::OuterRow(ArrayLargeDim idx) const {  
  // The index of the block.
  ArrayLargeDim bidx = idx / this->block_size;
  switch (this->outer_layout) {
  case ROW_MAJOR_LAYOUT:
    {
      ArrayLargeDim Bm = block_count(this->m, this->sub_m);
      return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_row(bidx, Bm);
    }    
  case COL_MAJOR_LAYOUT:
    {
      ArrayLargeDim Bn = block_count(this->n, this->sub_n);
      return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_row(bidx, Bn);
      //      return bidx % Bn;
    }    
  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::get_row(bidx, 0);
    //    return MortonIndexing::ROW_FROM_MORTON(bidx);
  default:
    // Should never get here.
    assert(0);
    return (ArrayLargeDim)-1;
  }
}

inline ArrayLargeDim BlockLayout::OuterCol(ArrayLargeDim idx) const {
  // The index of the block.
  ArrayLargeDim bidx = idx / this->block_size;
  switch (this->outer_layout) {
  case ROW_MAJOR_LAYOUT:
    {
      ArrayLargeDim Bm = block_count(this->m, this->sub_m);
      return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_col(bidx, Bm);
    }

  case COL_MAJOR_LAYOUT:
    {
      ArrayLargeDim Bn = block_count(this->n, this->sub_n);
      return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_col(bidx, Bn);
    }
    
  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::get_col(bidx, 0);

  default:
    // Should never get here.
    assert(0);
    return (ArrayLargeDim)-1;
  }
}

inline ArrayLargeDim BlockLayout::InnerRow(ArrayLargeDim idx) const {
  
  // The index within the block.
  ArrayLargeDim sidx = idx % this->block_size;

  switch (this->inner_layout) {

  case ROW_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_row(sidx, this->sub_m);   
    //    return sidx / this->sub_m;
    
  case COL_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_row(sidx, this->sub_n);
    //return sidx % this->sub_n;

  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::get_row(sidx, 0);

    //    return MortonIndexing::ROW_FROM_MORTON(sidx);
  default:
    // Should never get here.
    assert(0);
    return (ArrayLargeDim)-1;
  }
}

inline ArrayLargeDim BlockLayout::InnerCol(ArrayLargeDim idx) const {

  // The index within the block.
  ArrayLargeDim sidx = idx % this->block_size;

  switch (this->inner_layout) {
  case ROW_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_col(sidx, this->sub_m);   

  case COL_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_col(sidx, this->sub_n);

  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::get_col(sidx, 0);
    
  default:
    // Should never get here.
    assert(0);
    return (ArrayLargeDim)-1;
  }
}


ArrayLargeDim BlockLayout::InnerIdx(ArrayDim si, ArrayDim sj) const {
  // Offsets within a block.
  switch (this->inner_layout) {
  case ROW_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_idx(si, sj, this->sub_m);
    
  case COL_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_idx(si, sj, this->sub_n);
        
  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::get_idx(si, sj, 0);
  default:
    assert(0);
  }
  return (ArrayLargeDim)-1;
}




ArrayLargeDim BlockLayout::Idx(ArrayDim i, ArrayDim j) const {

  ArrayLargeDim outer_idx, inner_idx;
  ArrayDim Bi, Bj;
  ArrayDim si, sj;
  
  // Index of blocks.
  Bi = i / this->sub_n;
  Bj = j / this->sub_m;
  si = i % this->sub_n;
  sj = j % this->sub_m;
  

/*   printf("HERE: i = %d, j = %d, this->outer_layout = %d, this->inner_layout = %d\n", */
/*   	 i, j, */
/*   	 this->outer_layout, */
/*   	 this->inner_layout); */
//  printf("HERE: Bi = %d, Bj = %d\n", Bi, Bj);
  
 
  switch (this->outer_layout) {
  case ROW_MAJOR_LAYOUT:
    {
      ArrayLargeDim Bm = block_count(this->m, this->sub_m);
      //      printf("HERE: Bm = %ld\n", Bm);
      outer_idx = NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_idx(Bi, Bj, Bm);
    }
    break;
  case COL_MAJOR_LAYOUT:
    {
      ArrayLargeDim Bn = block_count(this->n, this->sub_n);
      outer_idx = NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_idx(Bi, Bj, Bn);
      // ArrayLargeDim Bn = block_count(layout->n, layout->sub_n);
      // outer_idx = Bi + Bn*Bj;
    }
    break;    
  case MORTON_LAYOUT:
    {
      outer_idx = NabbitLayoutIndexing<MORTON_LAYOUT>::get_idx(Bi, Bj, 0);
      //    outer_idx = MortonIndexing::MORTON_IDX(Bi, Bj);
    }
    break;
  default:
    assert(0);
  }

  inner_idx = this->InnerIdx(si, sj);

  assert(inner_idx < this->block_size);

  if (0) {
    printf("this->inner_layout = %d, outer_layout = %d\n",
	   this->inner_layout, this->outer_layout);
    printf("(i = %d, j = %d): this->block_size is %llu. outer_idx = %llu, inner_idx = %llu, returning answer %llu\n",
	   i, j,
	   this->block_size,
	   outer_idx, inner_idx,
	   this->block_size * outer_idx + inner_idx);
  }

  return this->block_size * outer_idx  + inner_idx;
}

#endif // __BLOCK_LAYOUTS_H_
