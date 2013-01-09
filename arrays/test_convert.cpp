/* test_convert.cpp                  -*-C++-*-
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

#include <iostream>
#include <cstdlib>
#include <cilk/cilk.h>


#include "convert.h"


void test_blocksizes(void) {

    // 100 by 100 array, 5 by 5 blocks, no padding, ROW_MAJOR.

    assert(BlockLayout::MinArraySize(100, 100, 5, 5, 0,
                                     ROW_MAJOR_LAYOUT,
                                     ROW_MAJOR_LAYOUT) == 100*100);

    assert(BlockLayout::MinArraySize(100, 100, 5, 5, 0,
                                     COL_MAJOR_LAYOUT,
                                     ROW_MAJOR_LAYOUT) == 100*100);
    // 20 blocks rounds up to 32.
    assert(BlockLayout::MinArraySize(100, 100, 5, 5, 0,
                                     MORTON_LAYOUT,
                                     ROW_MAJOR_LAYOUT) == 32 * 32 * 25);
  
    // 20 blocks rounds up to 32, each blocks rounds up to 8 by 8. 
    assert(BlockLayout::MinArraySize(100, 100, 5, 5, 0,
                                     MORTON_LAYOUT,
                                     MORTON_LAYOUT) == 32 * 32 * 64);

    // Need 15 by 15 blocks, each block is 7 by 7.
    //  assert(BlockedArraySize(100, 100, 7, 7, 0) == (15*15*7*7));

    assert(BlockLayout::MinArraySize(100, 100, 7, 7, 0,
                                     ROW_MAJOR_LAYOUT,
                                     ROW_MAJOR_LAYOUT) == 15*15*7*7);

    // Padding of 3.
    //  assert(BlockedArraySize(100, 100, 7, 7, 3) == (15*15*(7*7+3)));
    assert(BlockLayout::MinArraySize(100, 100, 7, 7, 3,
                                     ROW_MAJOR_LAYOUT,
                                     ROW_MAJOR_LAYOUT) == 15*15*(7*7+3));

    // 5 by 7 blocks
    //  assert(BlockedArraySize(100, 100, 5, 7, 3) == (15*20*(7*5+3)));
    assert(BlockLayout::MinArraySize(100, 100, 5, 7, 3,
                                     ROW_MAJOR_LAYOUT,
                                     ROW_MAJOR_LAYOUT) == 15*20*(7*5+3));

    //  assert(BlockedArraySize(11, 13, 4, 4, 1) == 204);  
    assert(BlockLayout::MinArraySize(11, 13, 4, 4, 1,
                                     ROW_MAJOR_LAYOUT,
                                     ROW_MAJOR_LAYOUT) == 204);


    BlockLayout layout1 = { 6, 5, ROW_MAJOR_LAYOUT,
                            3, 4,
                            13,
                            ROW_MAJOR_LAYOUT };
    BlockLayout layout2 = { 6, 5, ROW_MAJOR_LAYOUT,
                            3, 4,
                            13,
                            MORTON_LAYOUT };
    assert(layout1.IsValidLayout());
    assert(!layout2.IsValidLayout());

    BlockLayout layout3 = { 11, 13,
                            COL_MAJOR_LAYOUT,
                            2, 3,
                            2*3+1,
                            ROW_MAJOR_LAYOUT };
  
    {
        ArrayLargeDim idx = 35;
        ArrayDim outer_row = layout3.OuterRow(idx);
        ArrayDim outer_col = layout3.OuterCol(idx);
        assert(outer_row == 5);
        assert(outer_col == 0);
        assert(layout3.OuterRow(35) == 5);
        assert(layout3.Idx(0, 3) == 42);
        assert(layout3.Idx(10, 0) == 35);
    }

}


void test_index_calc(int n, int m,
		     int sub_n, int sub_m,
		     int block_padding,
		     bool verbose) {
    BlockLayout layout;
    assert(n > 0);
    assert(m > 0);  
    assert(sub_n > 0);
    assert(sub_m > 0);

    // Try all combinations of layouts.
    for (layout.outer_layout = ROW_MAJOR_LAYOUT;
         layout.outer_layout <= MORTON_LAYOUT;
         layout.outer_layout = (NabbitArray2DLayout)((int)layout.outer_layout+1)) {
        for (layout.inner_layout = ROW_MAJOR_LAYOUT;
             layout.inner_layout <= MORTON_LAYOUT;
             layout.inner_layout = (NabbitArray2DLayout)((int)layout.inner_layout+1)) {
            
            ArrayLargeDim required_size = BlockLayout::MinArraySize(n, m,
                                                                    sub_n, sub_m,
                                                                    block_padding,
                                                                    layout.inner_layout,
                                                                    layout.outer_layout);
            ArrayLargeDim padded_size = required_size + 400;
            int* Apad = (int*)malloc(sizeof(int) * padded_size);
            int* A = Apad + 32;
            int i, j;

            if (verbose) {
                printf("n = %d, m = %d, sub_n = %d, sub_m = %d\n",
                       n, m, sub_n, sub_m);
            }
            //      printf("Required size is %lld\n",
            //	     (long long)required_size);
	     
            for (ArrayLargeDim z = 0; z < padded_size; z++) {
                Apad[z] = -1;
            }

            layout.n = n;
            layout.m = m;
            layout.sub_n = sub_n;
            layout.sub_m = sub_m;
            layout.block_size = BlockLayout::MinBlockSize(sub_n, sub_m,
                                                          block_padding,
                                                          layout.inner_layout);
            assert(layout.IsValidLayout());


            for (i = 0; i < n; i++) {
                for (j = 0; j < m; j++) {
                    int bi, bj, si, sj;
                    ArrayLargeDim idx = layout.Idx(i, j);
                    //	  ArrayLargeDim idx = NabbitArrayGenericIdx(i, j, &layout);
                    if (verbose) {
                        printf("(%d, %d): idx = %lld\n",
                               i, j, (long long)idx);
                    }
                    assert((int64_t)idx >= 0);
                    assert(idx < required_size);

                    bi = layout.OuterRow(idx);
                    bj = layout.OuterCol(idx);

                    si = layout.InnerRow(idx);	  
                    sj = layout.InnerCol(idx);

                    //	  si = NabbitArrayInnerRow(idx, &layout);


                    int num_errors = 0;
                    num_errors += (i != (bi*sub_n + si));
                    num_errors += (j != (bj*sub_m + sj));
	  
                    if ((verbose) || num_errors) {
                        printf("i = %d, j = %d\n", i, j);
                        printf("idx = %llu\n", idx);
                        printf("inner_layout = %d, outer_layout = %d\n",
                               layout.inner_layout, layout.outer_layout);
                        printf("(bi, bj) = (%d, %d)\n",
                               bi, bj);
                        printf("(si, sj) = (%d, %d)\n",
                               si, sj);
                        printf("sub_n = %d, si = %d, sub_m = %d, sj = %d\n",
                               sub_n, si, sub_m, sj);
                    }
                    assert(i == (bi*sub_n + si));
                    assert(j == (bj*sub_m + sj));
                    A[idx] = n*i + j;
                }
            }

            if (verbose) {
                for (i = 0; i < n; i++) {
                    for (j = 0; j < m; j++) {
                        ArrayLargeDim idx = layout.Idx(i, j);
                        //	    ArrayLargeDim idx = NabbitArrayGenericIdx(i, j, &layout);
                        printf("%3lld  ", (long long)idx);
                        assert(A[idx] == (n*i+j));
                    }
                    printf("\n");
                }
            }
            free(Apad);                 
        }
    }
}

void test_conversion(int n, int m,
		     int isub_n, int isub_m, int iblock_padding, // Input layout
		     int osub_n, int osub_m, int oblock_padding, // Output layout
		     bool NABBIT_ARRAY_UNUSED(verbose)) {
    
    BlockLayout input_layout;
    BlockLayout output_layout;
    assert(n > 0);
    assert(m > 0);  
    assert(isub_n > 0);
    assert(isub_m > 0);
    assert(osub_n > 0);
    assert(osub_n > 0);

    input_layout.n = n;
    input_layout.m = m;
    input_layout.sub_n = isub_n;
    input_layout.sub_m = isub_m;

    output_layout.n = n;
    output_layout.m = m;
    output_layout.sub_n = osub_n;
    output_layout.sub_m = osub_m;


    // Try all combinations of layouts.
    for (input_layout.outer_layout = ROW_MAJOR_LAYOUT;
         input_layout.outer_layout <= MORTON_LAYOUT;
         input_layout.outer_layout = (NabbitArray2DLayout)((int)input_layout.outer_layout+1)) {
        for (input_layout.inner_layout = ROW_MAJOR_LAYOUT;
             input_layout.inner_layout <= MORTON_LAYOUT;
             input_layout.inner_layout = (NabbitArray2DLayout)((int)input_layout.inner_layout+1)) {
      
            ArrayLargeDim input_size = BlockLayout::MinArraySize(n, m,
                                                                 isub_n, isub_m,
                                                                 iblock_padding,
                                                                 input_layout.inner_layout,
                                                                 input_layout.outer_layout);
            input_layout.block_size = BlockLayout::MinBlockSize(isub_n, isub_m,
                                                                iblock_padding,
                                                                input_layout.inner_layout);
      
            int* inputA = (int*)malloc(sizeof(int) * input_size);
            int* copyA = (int*)malloc(sizeof(int) * input_size);
      
            for (ArrayLargeDim z = 0; z < input_size; z++) {
                inputA[z] = z;
            }
      
            for (output_layout.outer_layout = ROW_MAJOR_LAYOUT;
                 output_layout.outer_layout <= MORTON_LAYOUT;
                 output_layout.outer_layout = (NabbitArray2DLayout)((int)output_layout.outer_layout+1)) {
                for (output_layout.inner_layout = ROW_MAJOR_LAYOUT;
                     output_layout.inner_layout <= MORTON_LAYOUT;
                     output_layout.inner_layout = (NabbitArray2DLayout)((int)output_layout.inner_layout+1)) {

                    // copyA is zero-initialized.
                    for (ArrayLargeDim z = 0; z < input_size; z++) {
                        copyA[z] = 0;
                    }	  
	  
                    ArrayLargeDim output_size = BlockLayout::MinArraySize(n, m,
                                                                          osub_n, osub_m,
                                                                          oblock_padding,
                                                                          output_layout.inner_layout,
                                                                          output_layout.outer_layout);
                    output_layout.block_size = BlockLayout::MinBlockSize(osub_n, osub_m,
                                                                         oblock_padding,
                                                                         output_layout.inner_layout);

                    int* outputA = (int*)malloc(sizeof(int) * output_size);


                    // Convert input to output layout.
                    NabbitArrayCopyConvert(n, m,
                                           inputA, &input_layout,
                                           outputA, &output_layout);
                    int diff1 = NabbitArraySquaredDiff(n, m,
                                                       inputA, &input_layout,
                                                       outputA, &output_layout);
                    assert(diff1 == 0);

                    // Convert back to input layout, check that we are valid.
                    NabbitArrayCopyConvert(n, m,
                                           outputA, &output_layout,
                                           copyA, &input_layout);
                    int diff2 = NabbitArraySquaredDiff(n, m,
                                                       inputA, &input_layout,
                                                       copyA, &input_layout);
                    assert(diff2 == 0);	  
                    free(outputA);	  
                }
            }      
            free(inputA);
            free(copyA);      
        }
    }
}


int main(int argc, char *argv[])
{


    bool small = false;
    bool verbose = true;


    if (argc > 1) {
        small = true;
    }
    printf("Testing array conversion utilities; small = %d\n", small);

    test_blocksizes();
    test_index_calc(6, 5, 3, 4, 1, false);
    test_index_calc(11, 13, 2, 3, 1, false);
    test_index_calc(1, 1, 1, 16, 9, false);

    test_conversion(11, 13,
                    3, 4, 1,
                    3, 4, 1,
                    false);
    test_conversion(11, 13,
                    3, 4, 1,
                    2, 3, 1,
                    false);
    test_conversion(11, 13,
                    3, 4, 1,
                    2, 3, 3,
                    false);


    const int NUM_BSIZES = 9;
    int MAX_DIM = 15;  
    int bsizes[NUM_BSIZES] = {1, 2, 3, 4, 5, 7, 11, 13, 16};
    if (small) {
        MAX_DIM = 4;
    }
  
    cilk_for (int n = 1; n < MAX_DIM; n++) {
        cilk_for (int m = 1; m < MAX_DIM; m++) {
      
            if (verbose) {
                //	printf("Testing n=%d, m=%d ...",
                //	       n, m);
            }

            cilk_for (int sn = 0; sn < NUM_BSIZES; sn++) {
                for (int sm = 0; sm < NUM_BSIZES; sm++) {
                    //	  printf("sn=%d, sm=%d, ... ", bsizes[sn], bsizes[sm]);
                    for (int padding = 0; padding < 10; padding+=3){
                        test_index_calc(n, m, bsizes[sn], bsizes[sm], padding, false);
                    }
                }
            }



            cilk_for (int sn = 0; sn < NUM_BSIZES; sn++) {
                for (int sm = 0; sm < NUM_BSIZES; sm++) {
                    test_conversion(n, m,
                                    bsizes[sn], bsizes[sm], 0,
                                    bsizes[sn], bsizes[sm], 0,
                                    false);
                    test_conversion(n, m,
                                    bsizes[sn], bsizes[sm], 0,
                                    bsizes[sm], bsizes[sn], 1,
                                    false);	    
                }
            }


            // Check row-major
            test_index_calc(n, m, 1, m, 0, false);
            test_conversion(n, m,
                            1, m, 0,
                            1, m, 0,
                            false);

            // Check column-major
            test_index_calc(n, m, n, 1, 0, false);
            test_conversion(n, m,
                            n, 1, 0,
                            n, 1, 0,
                            false);

            // Check row-to-column
            test_conversion(n, m,
                            1, m, 0,
                            n, 1, 0,
                            false);
      

            if (verbose) {
                printf("n=%d, m=%d ... PASSED\n", n, m);
            }
        }
    }
     
    return 0;
}


