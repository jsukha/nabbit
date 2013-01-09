/* SWDagParams.h                 -*-C++-*-
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

// This file defines some data structures for global parameters of the
// benchmark, and defines some methods for setting up the simulation.

#ifndef __SW_DAG_PARAMS_H
#define __SW_DAG_PARAMS_H

#include <array2d_base.h>
#include <array2d_morton.h>
#include "sw_matrix_kernels.h"

#define RANDOM_CHILD_ORDER 0



#ifdef TRACK_THREAD_CPU_IDS
#include "image.h"
#endif


// Structure defining parameters for the dag.

template <class SWNodeType>
class SWDAGParams {

 public:
  // Dimensions of a block.
  int width;
  int height;
  short Bwidth;
  short Bheight;
  NabbitArray2DMorton<int, 0>* data;
  NabbitArray2DMorton<int, 0>* s;
  int* gamma;

  SWNodeType* block_data;
  SWNodeType* root;
  
  ArrayDim blockdag_side;

#ifdef TRACK_THREAD_CPU_IDS
  color_image* compute_image;
#endif


  static const int BORDER_VAL = 0;

  inline void InitParameters(int B,
			     int n, int m) {
    this->Bwidth = B;
    this->Bheight = B;
    assert(n == m);
    this->width = n;
    this->height = m;    
  }
		      
  
  void InitGammaAndS(int* gamma,
		     NabbitArray2DMorton<int, 0>* s,
		     bool make_copy);

  int ComputeAtKey(long long key);

  SWNodeType* ConstructBlockDAG(void);
  void CheckResult();
  void ReportStats();
};


template <class SWNodeType>
void SWDAGParams<SWNodeType>::InitGammaAndS(int* gamma,
					    NabbitArray2DMorton<int, 0>* s,
					    bool make_copy) {

  SWDAGParams<SWNodeType>* params = this;
  int max_mn = params->width;

  if (params->height > max_mn) { max_mn = params->height; }

  if (make_copy || (gamma == NULL)) {
    params->gamma = new int[max_mn+1];
    assert(params->gamma);
  }

  if (make_copy || (s == NULL)) {
    params->s = new NabbitArray2DMorton<int, 0>((ArrayDim)params->width+1,
						(ArrayDim)params->height+1);
    assert(params->s);
  }


  if (gamma == NULL) {
    fill_random_1D(params->gamma, max_mn+1, 100);
  }
  else {

    if (make_copy) {
      for (int i = 0; i < max_mn+1; i++) {
	params->gamma[i] = gamma[i];
      }
    }
    else {
      params->gamma = gamma;
    }
  }

  if (s == NULL) {
    fill_random_2D(params->s, 100);
  }
  else {
    if (make_copy) {
      assert(s->get_height() == params->height+1);
      assert(s->get_width() == params->width+1);
      copy_2D(s, params->s);
    }
    else {
      params->s = s;
    }
  }
}

template <class SWNodeType>
int SWDAGParams<SWNodeType>::ComputeAtKey(long long key) {
  
  int result_val = 0;
  SWDAGParams<SWNodeType>* params = this;

  int row_num = MortonIndexing::get_row(key);
  int col_num = MortonIndexing::get_col(key);
  
  if ((row_num == 0) || (col_num == 0)) {
    result_val = BORDER_VAL;

    if ((row_num == 0)  && (col_num > 0)) {
      int start_col = 1 + (col_num - 1) * params->Bwidth;
      int end_col = start_col + params->Bwidth;
      if (end_col > params->width+1) {
	end_col = params->width+1;
      }

      for (int q = start_col; q < end_col; q++) {
	params->data->set(0, q, BORDER_VAL);
      }
    }

    if ((row_num > 0) && (col_num == 0)) {
      int start_row = 1 + (row_num - 1) * params->Bheight;
      int end_row = start_row + params->Bheight;
      if (end_row > params->height+1) {
	end_row = params->height+1;
      }

      for (int q = start_row; q < end_row; q++) {
	params->data->set(q, 0, BORDER_VAL);
      }      
    }

    if ((row_num == 0) && (col_num == 0)) {
      params->data->set(0, 0, BORDER_VAL);
    }
  }
  else {
    result_val = 0;

    // Compute the values in the block.
    int start_row = 1 + (row_num - 1) * params->Bheight;
    int end_row = start_row + params->Bheight;
    int start_col = 1 + (col_num - 1) * params->Bwidth;
    int end_col = start_col + params->Bwidth;


    if (end_row > params->height+1) {
      end_row = params->height+1;
    }
    if (end_col > params->width+1) {
      end_col = params->width+1;
    }

    //    printf("AFTER row_num = %d, col_num = %d: (start_row = %d, end_row = %d)",
    //	   row_num, col_num, start_row, end_row);
    //    printf(" (start_col = %d, end_col = %d)\n",
    //	   start_col, end_col);

    sw_compute_base(params->s,
		    params->gamma,
		    params->data,
		    start_row, end_row,
		    start_col, end_col);


    result_val = params->data->get(end_row-1,
				   end_col-1);
    //    printf("Rownum = %d, col_num = %d: EXTRACTING FINAL RESULT FROM (%d, %d): %d",
    //	   row_num, col_num,
    //	   end_row-1, end_col - 1,
    //	   result_val);
  }

  //  printf("Result of computing block (%d, %d):\n",
  //	 row_num, col_num);
  //  params->data->print();
  //  printf("FINAL RESULT:   Key = %llu (%d, %d): result = %d\n",
  //	 key, row_num, col_num, result_val);

  return result_val;
}




template <class SWNodeType>
SWNodeType* SWDAGParams<SWNodeType>::ConstructBlockDAG(void) {
  SWDAGParams<SWNodeType>* params = this;

  ArrayDim final_col_blocks = 1 + (params->width + params->Bwidth-1) / params->Bwidth;
  ArrayDim final_row_blocks = 1 + (params->height + params->Bheight-1) / params->Bheight;


#ifdef DEBUG_PRINT  
  ArrayLargeDim root_idx = MortonIndexing::MORTON_IDX(final_row_blocks,
						  final_col_blocks);
  printf("Width = %d, height = %d, root_idx = %lu\n",
	 params->width, params->height, root_idx);
  printf("Width in blocks = %d, height in blocks = %d\n",
	 final_col_blocks, final_row_blocks);
#endif
  params->data = new NabbitArray2DMorton<int, 0>(params->width+1,
						 params->height+1);

#ifdef DEBUG_PRINT
  printf("final_col_blocks is %d, final_row_blocks is %d\n",
	 final_col_blocks,
	 final_row_blocks);
  printf("Sizeof SWStaticNoe is %zd\n",
	 sizeof(SWNodeType));
#endif

  {
    assert(final_col_blocks == final_row_blocks);
    params->blockdag_side = final_col_blocks;
    long long block_dag_size = MortonIndexing::MortonSize(final_col_blocks);

#ifdef DEBUG_PRINT
    printf("Block_dag_size is %llu \n",
	   block_dag_size);
#endif
    params->block_data = new SWNodeType[block_dag_size];

#ifdef DEBUG_PRINT
    printf("Done with allocation of dag nodes\n");
#endif
  }

  // Init all the nodes first.
  for (int bi = 0; bi < final_row_blocks; bi++) {
    for (int bj = 0; bj < final_col_blocks; bj++) {
      long long midx = MortonIndexing::get_idx(bi, bj);
      SWNodeType* tmp_data = params->block_data;
      SWNodeType* current_node = &(tmp_data[midx]);
      current_node->init_node(3);      
    }
  }

  // Creating the DAG nodes for each of the blocks.
  for (int bi = 0; bi < final_row_blocks; bi++) {
    for (int bj = 0; bj < final_col_blocks; bj++) {
            
      long long midx = MortonIndexing::get_idx(bi, bj);
      //  SWNodeType* tmp_data = params->block_dag->get_data();
      SWNodeType* tmp_data = params->block_data;

      tmp_data[midx].key = midx;
      tmp_data[midx].params = params;
      tmp_data[midx].result = 0;

      //      printf("Setting midx %llu key = %llu\n",
      //	     midx, tmp_data[midx].key);

      SWNodeType* current_node = &(tmp_data[midx]);
      SWNodeType* child = NULL;
      //      current_node->init_node(3);


      if ((!RANDOM_CHILD_ORDER) || (rand() % 2 == 0)) {
	if (bj > 0) {
	  long long child_idx = MortonIndexing::get_idx(bi, bj-1);
	  child = &(tmp_data[child_idx]);
	  current_node->add_child(child);
	}
	if (bi > 0) {
	  long long child_idx = MortonIndexing::get_idx(bi-1, bj);
	  child = &(tmp_data[child_idx]);
	  current_node->add_child(child);
	}
      } else {
	if (bi > 0) {
	  long long child_idx = MortonIndexing::get_idx(bi-1, bj);
	  child = &(tmp_data[child_idx]);
	  current_node->add_child(child);
	}
	if (bj > 0) {
	  long long child_idx = MortonIndexing::get_idx(bi, bj-1);
	  child = &(tmp_data[child_idx]);
	  current_node->add_child(child);
	}	
      }
    }
  }

  long long final_child_idx = MortonIndexing::get_idx(final_row_blocks-1,
						      final_col_blocks-1);
  SWNodeType* tmp_data = params->block_data;
  params->root = &(tmp_data[final_child_idx]);
  
  assert(params->Bwidth >= 1);
  assert(params->Bheight >= 1);

  return params->root;
}


// For now, this method doesn't actually do anything...
template <class SWNodeType>
void SWDAGParams<SWNodeType>::CheckResult() {
  if ((this->height < 20) &&
      (this->width < 20)) {

    printf("FINAL matrix: \n");
    this->data->print();
    for(int i = 0; i <= this->height; i++) {
      for (int j = 0; j <= this->width; j++) {

	ArrayLargeDim midx = MortonIndexing::get_idx(i, j);
	printf("%4d ", this->data->idx_get(&midx));
      }
      printf("\n");      
    }
    printf("\n");
  }
}


// This method is old code for generating a static image of
// the grid after computation is done, showing which workers
// computed which cells.  This functionality is redundant if 
// we are already doing logging.

template <class SWNodeType>
void SWDAGParams<SWNodeType>::ReportStats() {
  SWDAGParams<SWNodeType>* params = this;
#ifdef TRACK_THREAD_CPU_IDS

  int chist[maxP+2];
  int cwhist[maxP+2];  

  params->compute_image = create_color_image(params->blockdag_side,
					     params->blockdag_side,
					     maxP);
  assert(params->compute_image);
  
					   
  long long total_num_blocks = 0;
  long long total_weight = 0;

  for (int i = 0; i < maxP+1; i++) {
    //    vhist[i] = 0;
    //    vwhist[i] = 0;
    chist[i] = 0;
    cwhist[i] = 0;
  }

  printf("Dimensions of final block dag: %d by %d\n",
	 params->blockdag_side, params->blockdag_side);

  for (ArrayDim i = 0; i < params->blockdag_side; i++) {
    for (ArrayDim j = 0; j < params->blockdag_side; j++) {

      ArrayLargeDim idx = MortonIndexing::get_idx(i, j);
      SWNodeType* current = &params->block_data[idx];

      if (params->blockdag_side <= 15) {
	printf("%d ",
	       current->compute_id);
      }

      {
	int X = maxP;
	if ((current->compute_id >= -1) && (current->compute_id < maxP-1)) {
	  X = current->compute_id+1;
	}
	chist[X]++;
	cwhist[X] += (i+j);

	set_image_color_for_proc(params->compute_image,
				 i, j,
				 X-1);
      }
	
      total_num_blocks++;
      total_weight += (i+j);
    }

    if (params->blockdag_side <= 15) {
      printf("\n");
    }
  }

  printf("Total number of blocks in dag: %llu\n",
	 total_num_blocks);


  for (int i = -1; i <= maxP; i++) {
    double cval = chist[i+1]*1.0/total_num_blocks;
    double cwval = cwhist[i+1]*1.0 / total_weight;


    if ((cval >0)) {
      printf("Proc Compute %d: %d = %f.  Weighted: %d = %f\n",
	     i,
	     chist[i+1],
	     cval,
	     cwhist[i+1],
	     cwval);
    }
  }

  {
    char cName[100];
    //    char vName[100];
    int id = getpid();
    int wkr_count = NABBIT_WKR_COUNT;

    // TODO(jsukha): The naming convention for the
    // images should probably incorporate more of the information
    // about which run (e.g., test type, matrix size, etc.)
    
    snprintf(cName, 100,
	     "compute_%d_p%d.ppm", id, wkr_count);    
    save_color_image_to_file(cName, params->compute_image);
    destroy_color_image(params->compute_image);
  }
#endif
}

#endif //  __SW_DAG_PARAMS_H
