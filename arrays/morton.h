/* morton.h                  -*-C++-*-
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
// Code for Morton-order indexing.


#ifndef __MORTON_H
#define __MORTON_H

#include "nabbit_array_defs.h"


/**************************************************************
 * This class defines static methods for converting from normal 2d
 * indices to Morton-order indices.
 *
 * A Morton-order index "x" can be derived from row/column indices
 * (i, j) by interleaving the bits of i and j. 
 *
 *
 * To implement this class, I worked from the description / code for
 *  Morton order indexing described in the following papers:
 *
 *  1. Converting to and from Dilated Integers
 *      by Rajeev Raman and  David S. Wise
 * and 
 *
 *  2. Morton-order Matrices Deserve Compilers' Support
 *             Technical Report 533
 *       by David S. Wise,    Jeremy D. Frensz
 *
 */



// Default (long) morton-order index (64 bits).
typedef ArrayLargeDim MortonIndex;

// Type definitions for a row-only index
// and a column-only index.
// These are only defined to help programmers
// distinguish the types.
//
// To get the full Morton index, you add row and
// column indices together.
typedef MortonIndex MortonRowIndex;
typedef MortonIndex MortonColIndex;


// Short Morton-order indices. (32 bits)
// (so each dimension is at most 16 bits).
typedef ArrayDim MortonSIndex;
typedef MortonSIndex MortonSRowIndex;
typedef MortonSIndex MortonSColIndex;


// Lookup tables used for indexing.
static const uint16_t dilate_tab2[256] = {
  0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
  0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
  0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
  0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
  0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
  0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
  0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 
  0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555, 
  0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 
  0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055, 
  0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 
  0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155, 
  0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 
  0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455, 
  0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 
  0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555, 
  0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 
  0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055, 
  0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 
  0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155, 
  0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 
  0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455, 
  0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 
  0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555, 
  0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 
  0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055, 
  0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 
  0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155, 
  0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 
  0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455, 
  0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 
  0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555, 
};

static const uint8_t undilate_tab2[256] = {
  0x00, 0x01, 0x10, 0x11, 0x02, 0x03, 0x12, 0x13, 0x20, 0x21, 0x30, 0x31, 0x22, 0x23, 0x32, 0x33,
  0x04, 0x05, 0x14, 0x15, 0x06, 0x07, 0x16, 0x17, 0x24, 0x25, 0x34, 0x35, 0x26, 0x27, 0x36, 0x37,
  0x40, 0x41, 0x50, 0x51, 0x42, 0x43, 0x52, 0x53, 0x60, 0x61, 0x70, 0x71, 0x62, 0x63, 0x72, 0x73,
  0x44, 0x45, 0x54, 0x55, 0x46, 0x47, 0x56, 0x57, 0x64, 0x65, 0x74, 0x75, 0x66, 0x67, 0x76, 0x77,
  0x08, 0x09, 0x18, 0x19, 0x0A, 0x0B, 0x1A, 0x1B, 0x28, 0x29, 0x38, 0x39, 0x2A, 0x2B, 0x3A, 0x3B,
  0x0C, 0x0D, 0x1C, 0x1D, 0x0E, 0x0F, 0x1E, 0x1F, 0x2C, 0x2D, 0x3C, 0x3D, 0x2E, 0x2F, 0x3E, 0x3F,
  0x48, 0x49, 0x58, 0x59, 0x4A, 0x4B, 0x5A, 0x5B, 0x68, 0x69, 0x78, 0x79, 0x6A, 0x6B, 0x7A, 0x7B,
  0x4C, 0x4D, 0x5C, 0x5D, 0x4E, 0x4F, 0x5E, 0x5F, 0x6C, 0x6D, 0x7C, 0x7D, 0x6E, 0x6F, 0x7E, 0x7F,
  0x80, 0x81, 0x90, 0x91, 0x82, 0x83, 0x92, 0x93, 0xA0, 0xA1, 0xB0, 0xB1, 0xA2, 0xA3, 0xB2, 0xB3,
  0x84, 0x85, 0x94, 0x95, 0x86, 0x87, 0x96, 0x97, 0xA4, 0xA5, 0xB4, 0xB5, 0xA6, 0xA7, 0xB6, 0xB7,
  0xC0, 0xC1, 0xD0, 0xD1, 0xC2, 0xC3, 0xD2, 0xD3, 0xE0, 0xE1, 0xF0, 0xF1, 0xE2, 0xE3, 0xF2, 0xF3,
  0xC4, 0xC5, 0xD4, 0xD5, 0xC6, 0xC7, 0xD6, 0xD7, 0xE4, 0xE5, 0xF4, 0xF5, 0xE6, 0xE7, 0xF6, 0xF7,
  0x88, 0x89, 0x98, 0x99, 0x8A, 0x8B, 0x9A, 0x9B, 0xA8, 0xA9, 0xB8, 0xB9, 0xAA, 0xAB, 0xBA, 0xBB,
  0x8C, 0x8D, 0x9C, 0x9D, 0x8E, 0x8F, 0x9E, 0x9F, 0xAC, 0xAD, 0xBC, 0xBD, 0xAE, 0xAF, 0xBE, 0xBF,
  0xC8, 0xC9, 0xD8, 0xD9, 0xCA, 0xCB, 0xDA, 0xDB, 0xE8, 0xE9, 0xF8, 0xF9, 0xEA, 0xEB, 0xFA, 0xFB,
  0xCC, 0xCD, 0xDC, 0xDD, 0xCE, 0xCF, 0xDE, 0xDF, 0xEC, 0xED, 0xFC, 0xFD, 0xEE, 0xEF, 0xFE, 0xFF, 
};




class MortonIndexing {
  
 public:

  static MortonIndex get_idx(ArrayDim i,
			     ArrayDim j) {
    MortonIndex low_idx;
    MortonIndex high_idx;    
    low_idx = get_idx_32bit((uint16_t)(i & 0xFFFF), 0) + get_idx_32bit(0, (uint16_t)(j & 0xFFFF));
    high_idx = get_idx_32bit((uint16_t)(i >> 16), 0) + get_idx_32bit(0, (uint16_t)(j >> 16));
    return low_idx | (high_idx << 32);
  }

  // From a Morton index, extract the value of the row.
  static inline ArrayDim get_row(MortonIndex x) {
    return (undilate_2(((x) & ROW_MASK)));
  }

  // From the Morton index, extract the value of the column.
  static inline ArrayDim get_col(MortonIndex x) {
    return (undilate_2(((x) & COL_MASK) >> 1));
  }

  // x is a Morton row index.
  // Moves x to the next row.
  // Since x is Morton row index, the bits of the column index
  // in the actual representation of x must be 0.
  static inline MortonRowIndex next_row(MortonRowIndex x) {
    return ((x - ROW_MASK) & ROW_MASK);
  }

  // x is a Morton column index.
  // Moves x to the next column.
  // Since x is Morton column index, the bits of the row index
  // in the actual representation of x must be 0.
  static inline MortonColIndex next_col(MortonRowIndex x) {
    return ((x - COL_MASK) & COL_MASK);
  }


  // Convert a row-only morton index to col-only morton index.
  static inline ArrayLargeDim idx_row_to_col(MortonRowIndex x) {
    return (MortonColIndex) ((x) << 1);
  }

  // Convert a column-only morton index to row-only morton index.
  static inline ArrayLargeDim idx_col_to_row(MortonColIndex x) {
    return (MortonRowIndex) ((x) >> 1);
  }



  // Morton index from row and column, but only creates a 32-bit
  // index.
  static MortonSIndex get_idx_32bit(uint16_t i,
				    uint16_t j) {
    return (MortonSIndex)((dilate_2((unsigned short)i)) | (dilate_2((unsigned short)j) << 1));
  }


  // x is a MortonRowIndex for the upper left corner of a block
  // of size (2^(lg_n)).
  //
  // Increments x by(2^(lg_n)) rows.
  //
  // For example, if lg_n = 3, then x should be the row index for some
  // row 8k, for some integer k.
  // Then, after the increment, the x will be the row index
  // for row 8(k+1).
  static inline MortonRowIndex BlockRowInc(MortonRowIndex x,
					   ArrayLgDim lg_n) {
    return ((x) + (1 << (2*(lg_n))));
  }

  // Corresponding method for column indices.
  static inline MortonColIndex BlockColInc(MortonColIndex x,
					   ArrayLgDim lg_n) {
    return ((x) + (1 << (2*(lg_n) + 1)));
  }
    
  // Returns the size required for an n by n array
  // Size is N * N, where N is n rounded up to the next power of 2.  
  static inline ArrayLargeDim MortonSize(ArrayDim n) {
    ArrayLargeDim test_size = hyperceil((ArrayLargeDim)n);
    return get_idx(test_size-1,
		   test_size-1) + 1;
  }



 protected:
  // These methods are "private", in the sense that they
  // are used to implement the higher-level interface above.
  // If you are inclined to manipulate the bits directly, then
  // the code below might be useful though.
  

  // Mask to extract the bits representing the row
  // from an index. (bit 0, 2, 4, 6, ...)
  // starting from the least significant bit.
  static const uint64_t ROW_MASK = 0x5555555555555555L;

  // Mask to extract bits of the column.
  // (bit 1, 3, 5, ...)
  static const uint64_t COL_MASK = 0xAAAAAAAAAAAAAAAAL;

  // Masks if we have dimensions which are only 16-bit.  
  static const uint32_t ROW_MASK_S = 0x55555555;
  static const uint32_t COL_MASK_S = 0xAAAAAAAA;

  static inline ArrayLargeDim dilate_2(ArrayDim x) {
    return dilate_tab2[ 0xFF & x]
      | (dilate_tab2[ (0xFFFF & x) >> 8] << 16);
  }

  static inline ArrayDim undilate_2(ArrayLargeDim x) {
    return (ArrayDim)undilate_tab2[0xFF & ((x>>7) | x)]
      | (ArrayDim)(undilate_tab2[0xFF & (((x>>7) | x) >> 16)] << 8);
  }
  
  
  /*********************************************************************/
  // Code which is not in use.
     
  // Two alternate methods for undilating and dilating
  // which we aren't using.
  static inline ArrayDim undilate_2_alt(ArrayLargeDim x) {
    x = (x *  3)  & 0x66666666;
    x = (x *  5)  & 0x78787878;
    x = (x * 17)  & 0x7F807F80;
    x = (x * 257) & 0x7FFF8000;
    return ((ArrayDim) (x >> 15));
  }
  static inline ArrayLargeDim slow_dilate_2(ArrayDim x) {
    int z;
    ArrayLargeDim ans = 0;
    for (z = 0; z < 16; z++) {
      ans += (((1 << z) & x) > 0) * (1 << (2*z));
    }
    return ans;
  }
  
  // Returns the size required for an n by m
  // rectangular array.
  //
  // Both dimensions n and m must be "small" (e.g., < 32 bits).
  //
  // The size allocated is N * M.
  //    N is n rounded up to the next power of 2
  //    M is m rounded up to the next power of 2
  //
  // The wasted space is at most 4 times n*m, if we almost double 
  // n and m when rounding.
  //
  // TODO: Indexing into this structure is significantly more
  // complicated at the boundaries though, so I don't know whether
  // this approach is a good idea or not.
  static inline ArrayLargeDim MortonRectangleSize(ArrayDim n,
						  ArrayDim m) {
    ArrayDim test_size_n = 1;
    ArrayDim test_size_m = 1;
    ArrayLargeDim final_size;

    test_size_n = hyperceil(n);
    test_size_m = hyperceil(m);

    // If we overflow, we fail the assert.
    assert(test_size_n >= n);
    assert(test_size_m >= m);
    final_size = ((int64_t)test_size_n) * ((int64_t)test_size_m);
    
    return final_size;
  }
  
};

#endif // __MORTON_H
