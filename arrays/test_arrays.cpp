/* test_arrays.cpp                  -*-C++-*-
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


#include "array2d_row.h"
#include "array2d_morton.h"


template <class MatrixType>
void testMatrix(MatrixType M,
		int width, int height) {
    printf("Initial layout of M: \n");
    M->print_layout();

    M->fill_with_constant_element(0);
    printf("Printing zero in M: \n");
    M->print();

    M->fill_with_constant_element(43);
    printf("Printing 43 in M?: \n");
    M->print();

    printf("Testing setters?: \n");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            M->set(i, j, i+j);
        }
    }
    printf("Done\n");
    printf("Testing getters?: \n");
    M->print();     
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            //	 printf("At (%d, %d) we got %d\n",
            //		i, j,
            //		M->get(i, j));
            assert(M->get(i, j) == (i+j));
	 
        }
    }

    printf("Testing iterators?: \n");
    int i = 0;
    for (ArrayLargeDim row_idx = M->row_iterator();
         M->has_next_row(&row_idx);
         M->increment_row(&row_idx)) {
        int j = 0;

        for (ArrayLargeDim col_idx = M->col_iterator();
             M->has_next_col(&col_idx);
             M->increment_col(&col_idx)) {

            //      printf("col_idx here has val %llu\n",
            //	     col_idx);
            //      	 printf("i = %d, j = %d, M->row = %d, M->col = %d\n",
            //      		i, j,
            //      		M->row_idx(&row_idx),
            //      		M->col_idx(&col_idx));

            assert(M->row_idx(&row_idx) == i);        
            assert(M->col_idx(&col_idx) == j);
            assert(M->idx_get(&row_idx,
                              &col_idx) == (i+j));
            j++;			   
        }

        assert(j == width);
        i++;
    }
    assert(i == height);
    printf("Done testing iterators\n");


    printf("Final matrix layout: \n");
    M->print_layout();  
}


int main(int argc, char *argv[])
{

    int width = 100;
    int height = 150;
    int padding = 4;

    if (argc >= 2) {
        width = atoi(argv[1]);

        if (argc >= 3) {
            height = atoi(argv[2]);
        }
    }

    printf("Width = %d, Height = %d\n", width, height);

    {
        NabbitArray2DRowMajor<int>* M = new NabbitArray2DRowMajor<int>(width, height, padding);
        assert(M);
        testMatrix<NabbitArray2DRowMajor<int>*>(M, width, height);
        delete M;
    }

    {
        NabbitArray2DMorton<int, 0, 0>* M2 = new NabbitArray2DMorton<int, 0, 0>(width, width);
        assert(M2);
        testMatrix<NabbitArray2DMorton<int, 0, 0>*>(M2, width, width);
        delete M2;
    }

    {
        // Morton array with some padding.
        NabbitArray2DMorton<int, 2, 3>* M3 = new NabbitArray2DMorton<int, 2, 3>(width, width);
        assert(M3);
        testMatrix<NabbitArray2DMorton<int, 2, 3>*>(M3, width, width);
        delete M3;
    }

    printf("Completed layout test\n");
    
    return 0;
}

