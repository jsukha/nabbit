/* sw_matrix_kernels.h                 -*-C++-*-
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
// Dynamic Programming benchmark.

// Definition of the base case methods, and the divide-and-conquer and
// wavefront algorithms for computing the dynamic program.

#ifndef __SW_MATRIX_KERNELS_H
#define __SW_MATRIX_KERNELS_H


#include "matrix_utils.h"
#include "sw_computeEF.h"

#ifdef TRACK_THREAD_CPU_IDS
#include "sw_visual.h"
#endif

#define USE_CILKFOR_WAVEFRONT


/**************************************************************/
// Various methods for computing  Eij and Fij. 
//

template <class MMatrixType, class SMatrixType>
void sw_compute_gold_generic(SMatrixType* s,
			     int* gamma,
			     MMatrixType* M);



// Base case for recursion.
template <class MMatrixType, class SMatrixType>
void sw_compute_base(SMatrixType* s,
		     int* gamma,
		     MMatrixType* M,
		     int start_row,
		     int end_row,
		     int start_col,
		     int end_col);

template <class MMatrixType, class SMatrixType, int block_size>
void sw_compute_blocked(SMatrixType* s,
			int* gamma,
			MMatrixType* M);


template <class MMatrixType, class SMatrixType, int base_size>
void sw_compute_divide_and_conquer(SMatrixType* s,
				   int* gamma,
				   MMatrixType* M);

template <class MMatrixType, class SMatrixType, int base_size, int K>
void sw_compute_DC_wavefront(SMatrixType* s,
			     int* gamma,
			     MMatrixType* M);



/********************************************************************/
// The actual implementations.


// The straightforward way to compute the Smith-Waterman DP.
template <class MMatrixType, class SMatrixType>
void sw_compute_gold_generic(SMatrixType* s,
			     int* gamma,
			     MMatrixType* M) {

  assert(s->get_width() == M->get_width());
  assert(s->get_height() == M->get_height());
  
  for (int i = 1; i < M->get_height(); i++) {
    for (int j = 1; j < M->get_width(); j++) {
      int Mij_val = 0;
      int Eij, Fij;

      Eij = computeEij(gamma, M, i, j);
      Fij = computeFij(gamma, M, i, j);

      // Compute the max of the quantities.
      if (Eij > Mij_val) { Mij_val = Eij; }
      if (Fij > Mij_val) { Mij_val = Fij; }
      {
	int temp = M->get(i-1, j-1) + s->get(i, j);
	if (temp > Mij_val) {
	  Mij_val = temp;
	}
      }
      M->set(i, j, Mij_val);
    }
  }
}



template <class MMatrixType, class SMatrixType>
void sw_compute_base(SMatrixType* s,
		     int* gamma,
		     MMatrixType* M,
		     int start_row,
		     int end_row,
		     int start_col,
		     int end_col) {

#ifdef TRACK_THREAD_CPU_IDS
  NabbitNodeRecord<SWRec> node_rec;
  if (sw_global_stats->is_collecting()) {
    NabbitTimers::cycleCounter(&node_rec.start_ts);
    node_rec.data.start_i = start_row;
    node_rec.data.end_i = end_row;
    node_rec.data.start_j = start_col;
    node_rec.data.end_j = end_col;
    node_rec.compute_id = cilk::current_worker_id();
  }
#endif

  for (int i = start_row; i < end_row; i++) {
    for (int j = start_col; j < end_col; j++) {
      int Mij_val = 0;

      int Eij, Fij;
      {
	Eij = computeEij(gamma, M, i, j);
      }      
      {
	Fij = computeFij(gamma, M, i, j);
      }
      //      cilk_sync;

      // Compute the max of the quantities.

      Mij_val = MAX(Mij_val, Eij);
      Mij_val = MAX(Mij_val, Fij);
      {
	int temp = M->get(i-1, j-1) + s->get(i, j);
	Mij_val = MAX(Mij_val, temp);
      }
      M->set(i, j, Mij_val);
    }
  }

#ifdef TRACK_THREAD_CPU_IDS
  if (sw_global_stats->is_collecting()) {
    NabbitTimers::cycleCounter(&node_rec.end_ts);
    sw_global_stats->add_noderec(&node_rec);
  }
#endif  
}



template <class MMatrixType, class SMatrixType, int block_size>
void sw_compute_blocked(SMatrixType* s,
			int* gamma,
			MMatrixType* M) {

  int width = M->get_width();
  int height = M->get_height();

  for (int bi = 1; bi < height; bi += block_size) {
    for (int bj = 1; bj < width; bj += block_size) {
      int end_col = bj + block_size;
      int end_row = bi + block_size;
      if (end_col > width) {
	end_col = width;
      }
      if (end_row > height) {
	end_row = height;
      }

      sw_compute_base(s, gamma, M,
		      bi, end_row,
		      bj, end_col);            
    }
  }  
}




template <class MMatrixType, class SMatrixType, int base_size>
void sw_compute_d_and_c_helper(SMatrixType* s,
					  int* gamma,
					  MMatrixType* M,
					  int start_row, int end_row,
					  int start_col, int end_col) {
  
  bool base_row = ((end_row - start_row) <= base_size);
  bool base_col = ((end_col - start_col) <= base_size);

  if (!base_row && !base_col) {

    int mid_row = (start_row + end_row)/2;
    int mid_col = (start_col + end_col)/2;
    sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
								   start_row, mid_row,
								   start_col, mid_col);
    
    cilk_spawn sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
									      start_row, mid_row,
									      mid_col, end_col);
    sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
								   mid_row, end_row,
								   start_col, mid_col);
    cilk_sync;
    
    sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
								   mid_row, end_row,
								   mid_col, end_col);      
  }
  else if (base_row && !base_col) {

    int mid_col = (start_col + end_col)/2;
    sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
								   start_row, end_row,
								   start_col, mid_col);
    sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
								   start_row, end_row,
								   mid_col, end_col);
  }
  else if (!base_row && base_col) {

    int mid_row = (start_row + end_row)/2;
    sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
					 start_row, mid_row,
					 start_col, end_col);
    sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
								   mid_row, end_row,
								   start_col, end_col);
  }
  else {
    sw_compute_base(s, gamma, M,
		    start_row, end_row,
		    start_col, end_col);
  }
}


template <class MMatrixType, class SMatrixType, int base_size>
void sw_compute_divide_and_conquer(SMatrixType* s,
				   int* gamma,
				   MMatrixType* M) {
  sw_compute_d_and_c_helper<MMatrixType, SMatrixType, base_size>(s, gamma, M,
								 1, M->get_height(),
								 1, M->get_width());
}
		     


template <class MMatrixType, class SMatrixType, int base_size, int K>
void sw_compute_d_and_c_Ksplit(SMatrixType* s,
			       int* gamma,
			       MMatrixType* M,
			       int start_row, int end_row,
			       int start_col, int end_col) {

  const int test_base_size = (base_size < K) ? K : base_size;
  bool base_row = ((end_row - start_row) <= test_base_size);
  bool base_col = ((end_col - start_col) <= test_base_size);
  
  if (!base_row && !base_col) {

    int row_delta = (end_row - start_row)/K;
    int col_delta = (end_col - start_col)/K;

    // Everything up in the upper left corner, including the longest
    // diagonal.
    for (int diag = 0; diag < K; diag++) {

#ifdef USE_CILKFOR_WAVEFRONT
      cilk_for (int i = 0; i <= diag; i++) 
#else
      for (int i = 0; i <= diag; i++) 
#endif
      {
	int j = diag - i;

	int row_s = start_row + i*row_delta;
	int row_e = start_row + (i+1)*row_delta;

	if (i == K-1) {
	  row_e = end_row;
	}

	int col_s = start_col + j*col_delta;
	int col_e = start_col + (j+1)*col_delta;
	if (j == K-1) {
	  col_e = end_col;
	}

	//	printf("(%d, %d): row_s = %d, row_e = %d, col_s = %d, col_e = %d\n",
	//	       i, j,
	//	       row_s, row_e,
	//	       col_s, col_e);
#ifdef USE_CILKFOR_WAVEFRONT
	sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
									  row_s, row_e,
									  col_s, col_e);
#else
	cilk_spawn sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
										     row_s, row_e,
										     col_s, col_e);
#endif
      }
      
#ifdef USE_CILKFOR_WAVEFRONT
#else
      cilk_sync;
#endif
    }

    // Everything in the lower right corner.
    for (int diag = K; diag <= 2*K-2; diag++) {
#ifdef USE_CILKFOR_WAVEFRONT
      cilk_for (int j = diag - (K-1);  j < K; j++)
#else
      for (int j = diag - (K-1);  j < K; j++)
#endif
    {
	int i = diag - j;

	int row_s = start_row + i*row_delta;
	int row_e = start_row + (i+1)*row_delta;
	if (i == K-1) {
	  row_e = end_row;
	}

	int col_s = start_col + j*col_delta;
	int col_e = start_col + (j+1)*col_delta;
	if (j == K-1) {
	  col_e = end_col;
	}

	//	printf("(%d, %d): row_s = %d, row_e = %d, col_s = %d, col_e = %d\n",
	//	       i, j,
	//	       row_s, row_e,
	//	       col_s, col_e);

#ifdef USE_CILKFOR_WAVEFRONT
	sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
									  row_s, row_e,
									  col_s, col_e);
#else
	cilk_spawn  sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
										      row_s, row_e,
										      col_s, col_e);	
#endif
      }
#ifdef USE_CILKFOR_WAVEFRONT
#else
      cilk_sync;
#endif    
    }
  }
  else if (base_row && !base_col) {
    int mid_col = (start_col + end_col)/2;
    sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
								      start_row, end_row,
								      start_col, mid_col);
    sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
								      start_row, end_row,
								      mid_col, end_col);
  }
  else if (!base_row && base_col) {
    int mid_row = (start_row + end_row)/2;

    sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
								      start_row, mid_row,
								      start_col, end_col);
    sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
								      mid_row, end_row,
								      start_col, end_col);
  }
  else {
    sw_compute_base(s, gamma, M,
		    start_row, end_row,
		    start_col, end_col);
  }
}


template <class MMatrixType, class SMatrixType, int base_size, int K>
void sw_compute_DC_wavefront(SMatrixType* s,
			     int* gamma,
			     MMatrixType* M) {
  sw_compute_d_and_c_Ksplit<MMatrixType, SMatrixType, base_size, K>(s, gamma, M,
								    1, M->get_height(),
								    1, M->get_width());
}


template <class MMatrixType, class SMatrixType, int block_size, int K>
void sw_compute_pure_wavefront(SMatrixType* s,
			       int* gamma,
			       MMatrixType* M) {

  int width = M->get_width();
  int height = M->get_height();
  int bW = (width - 1)/block_size;
  int bH = (height - 1)/block_size;

  //  printf("width = %d, bW = %d.  height = %d, bH = %d\n",
  //	 width, bW, height, bH);
  
  // Scan the diagonals.
  for (int q = 0; q <= bW+bH; q++) {

    int start_brow = 0;
    int end_brow = q;

    if (end_brow > bH) {
      end_brow = bH;
    }
    if (q - start_brow > bW) {
      start_brow = q - bW;
    }
    
/*     printf("HERE: q = %d, brow = (%d, %d), bcol = (%d, %d)\n", */
/* 	   q, */
/* 	   start_brow, end_brow, */
/* 	   q - start_brow, */
/* 	   q - end_brow); */

    // Process the block in the top row.
    {
      int bi = 1 + start_brow * block_size;
      int end_row = bi + block_size;
      int bj = 1 + (q - start_brow) * block_size;
      int end_col = bj + block_size;

      if (end_col > width) {
	end_col = width;
      }
      if (end_row > height) {
	end_row = height;
      }

      //      printf("bi = %d, bj = %d\n",
      //	     bi, bj);
      //      printf("Spawning base. row range (%d, %d), col range (%d, %d)\n",
      //	     bi, end_row, bj, end_col);
      
      cilk_spawn sw_compute_base(s, gamma, M,
				 bi, end_row,
				 bj, end_col);

      //      sw_compute_base(s, gamma, M,
      //		      bi, end_row,
      //		      bj, end_col);

    }


#ifdef USE_CILKFOR_WAVEFRONT
    cilk_for (int i = start_brow+1; i <= end_brow; i++) {
      int bi = 1 + i * block_size;
      int bj = 1 + (q - i) * block_size;
      int end_row = bi + block_size;
      if (end_row > height) {
	end_row = height;
      }      

      //      printf("Spawning base. row range (%d, %d), col range (%d, %d)\n",
      //	     bi, end_row, bj, bj + block_size);
      sw_compute_base(s, gamma, M,
		      bi, end_row,
		      bj, bj + block_size);
    }
    
#else
    assert(0);
    for (int i = start_brow+1; i <= end_brow; i++) {      
      int bi = 1 + i * block_size;
      int bj = 1 + (q - i) * block_size;
      int end_row = bi + block_size;
      if (end_row > height) {
	end_row = height;
      }      

      cilk_spawn sw_compute_base(s, gamma, M,
				 bi, end_row,
				 bj, bj + block_size);
    }    
    cilk_sync;                
#endif

  }
}
		     
#endif // __SW_MATRIX_KERNELS_H
