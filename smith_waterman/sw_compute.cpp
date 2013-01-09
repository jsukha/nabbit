/* sw_compute.cpp                 -*-C++-*-
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


#include <iostream>
#include <cstdlib>
#include <cilk/cilk.h>

#include <example_util_gettime.h>
#include <array2d_row.h>
#include <array2d_morton.h>
#include "matrix_utils.h"
#include "sw_computeEF.h"
#include "sw_matrix_kernels.h"
#include "SWDagNode.h"
#include "sw_test_types.h"

// Turn on this flag if we have Cilkview. 
// (Newer versions of Cilk++ have this enabled).

#ifdef HAVE_CILKVIEW
#include <cilkview.h>
#endif

#ifdef BLOCK_VALUE
const int B = BLOCK_VALUE;
#else
const int B = 16;
#endif

#ifdef K_VALUE
const int K = K_VALUE;
#else
const int K = 5;
#endif



template <class NodeType, class SType>
int RunDAGEval(int n, int m,
	       int* gamma,
	       SType* s,
	       long* start_time,
	       long* end_time,
	       bool verbose,
	       SWComputeType test_type) {
  SWDAGParams<SWDAGNode<NodeType> > params;
  params.InitParameters(B, n, m);
  params.InitGammaAndS(gamma, s, false);

  *start_time = example_get_time();
  SWDAGNode<NodeType>* root = params.ConstructBlockDAG();
  
  switch (test_type) {

  case SW_STATIC_SERIAL:
    {
      SWDAGNode<StaticSerialNode>* source;
      source = (SWDAGNode<StaticSerialNode>*) params.block_data;
      source->source_compute();
    }
    break;

  case SW_STATIC_NABBIT:
    {
      SWDAGNode<StaticNabbitNode>* source;
      source = (SWDAGNode<StaticNabbitNode>*) params.block_data;
      source->source_compute();      
    }
    break;

  default:
    assert(0);
  }
  *end_time = example_get_time();

  if (verbose) {
    printf("The result: %d\n",
	   root->GetResult());
  }
  params.CheckResult();
#ifdef TRACK_THREAD_CPU_IDS
  params.ReportStats();
#endif
  return root->GetResult();
}
		
		
		
int main(int argc, char *argv[])
{
  int n = 100;
  int m;
  SWComputeType gold_type = SW_DC_K2;
  SWComputeType test_type = SW_STATIC_NABBIT;
#ifdef TRACK_THREAD_CPUS_IDS
  bool run_gold = false;
#else
  bool run_gold = true;
#endif
  bool verbose = false;
  int P = NABBIT_WKR_COUNT;

  if (argc >= 2) {
    n = atoi(argv[1]);
  }     
  if (argc >= 3) {
    m = atoi(argv[2]);
  } else {
    m = n;
  }
  if (argc >= 4) {
    test_type = (SWComputeType)atoi(argv[3]);
  }
  if (argc >= 5) {
    run_gold = false;
  }

  if (verbose) {
    printf("n = %d, m = %d:  ", n, m);
  }
  assert(n > 0);
  assert(m > 0);
  int* gamma;
  int max_mn = m;
  if (n > m) {
    max_mn = n;
  }
  if (verbose) {
    printf("Test_type = %d, run_gold = %d\n",
	   test_type, run_gold);
  }

#ifndef COMPUTE_CONSTANT_EF 
  double scale = (1.0 * (n + m) * n * m);
#else
  double scale = (1.0 * n * m);
#endif    


  // Possible choices for layout of the M and S matrices.
  //   NabbitArray2DRowMajor<int>
  //   NabbitArray2DMorton<int, 0>
#define MType NabbitArray2DMorton<int, 0>
#define SType NabbitArray2DMorton<int, 0>
#define M2Type NabbitArray2DMorton<int, 0>
#define S2Type NabbitArray2DMorton<int, 0>

  MType* M;
  SType* s;     
  M2Type* M2 =  NULL;
  S2Type* s2 = NULL;
     
  // Allocate a gamma, s, and M arrays.     
  gamma = new int[max_mn];
  assert(gamma);
  s = new SType((ArrayDim)m+1, n+1);
  assert(s);
  M = new MType(m+1, n+1);
  assert(M);


#ifdef TRACK_THREAD_CPU_IDS
  // Start Nabbit's logging (to record
  // progress of the computation).
  {
    sw_global_stats = new NabbitTaskGraphStats<SWRec>(P);
    sw_global_stats->global_time_barrier(P);
  }
#endif

  int answer;
  int ans_gold = 0;
  
  //     printf("max_mn = %d\n", max_mn);
  fill_random_1D(gamma, max_mn, 100);
  fill_random_2D<SType >(s, 100);
  zero_top_and_left_borders<MType >(M);
  
  if (run_gold) {
    M2 = new M2Type(m+1, n+1);
    assert(M2);
    s2 = new S2Type((ArrayDim)m+1, n+1);
    assert(s2);
    copy_2D<SType, S2Type>(s, s2);
    zero_top_and_left_borders<M2Type >(M2);
  }

  if (run_gold) {
    long start_time = example_get_time();

    if (gold_type == SW_DC_K2) {
      sw_compute_divide_and_conquer<M2Type, S2Type, B>(s2, gamma, M2);
    }
    else {
      sw_compute_gold_generic<M2Type, S2Type >(s2, gamma, M2);
    }
    long end_time = example_get_time();
    double time_in_sec = (end_time - start_time) / 1000.f;
    double constant_val = 1e6 * (end_time - start_time) / (scale);
    ans_gold = M2->get(n, m);
    if (verbose) {
      printf("**Gold type = %d: Running time of %d by %d: %f seconds total, (n+m)*m*n constant= %f **\n ",
	     gold_type,
	     n, m,
	     time_in_sec,
	     constant_val);
      printf("Final answer: M(%d, %d) = %d\n",
	     n, m, M2->get(n, m));
    }
    else {
      printf("%f (s), %f\n",
	     time_in_sec,
	     constant_val);
    }
  }

#ifdef TRACK_THREAD_CPU_IDS
  sw_global_stats->set_collection(true);
#endif  


#ifdef HAVE_CILKVIEW
  cilk::cilkview cv;
  cv.start();  // For Cilkview output.
#endif
  long start_time = 0, end_time = 0;
  double time_in_sec = 0.0;
  //  double scale = (1.0 * (n+m) * n* m);
  double constant_val;
  const char* test_string = "";
  switch (test_type) {
  case SW_GENERIC:
    {
      test_string = "Generic";
      start_time = example_get_time();
      sw_compute_gold_generic<MType, SType >(s, gamma, M);
      end_time = example_get_time();
      answer = M->get(n, m);           
      
    }
    break;
  case SW_DC_K2:
    {
      test_string = "Divide_and_Conquer_K2";
      start_time = example_get_time();
      sw_compute_divide_and_conquer<MType, SType, B>(s, gamma, M);
      end_time = example_get_time();
      answer = M->get(n, m);           
    }
    break;

  case SW_DC_GENERIC_K:
    {
      test_string = "DC_Wavefront";
      start_time = example_get_time();
      sw_compute_DC_wavefront<MType, SType, B, K>(s, gamma, M);
      end_time = example_get_time();
      answer = M->get(n, m);           
    }
    break;

  case SW_PURE_WAVEFRONT:
    {
      test_string = "Pure_Wavefront";
      start_time = example_get_time();
      sw_compute_pure_wavefront<MType, SType, B, K>(s, gamma, M);
      end_time = example_get_time();
      answer = M->get(n, m);
    }
    break;

  case SW_STATIC_NABBIT:
    {
      test_string = "Static_Nabbit";
      answer = RunDAGEval<StaticNabbitNode, SType>(n, m, gamma, s,
						   &start_time,
						   &end_time,
						   verbose,
						   test_type);
    }
    break;

  case SW_STATIC_SERIAL:
    {
      test_string = "Static_Serial";
      answer = RunDAGEval<StaticSerialNode, SType>(n, m, gamma, s,
						   &start_time,
						   &end_time,
						   verbose,
						   test_type);
    }
    break;

  default:
    test_string = "Null test";
    answer = 0;
    assert(0);
  }

#ifdef HAVE_CILKVIEW
  cv.stop();
  char test_name[100];
  SWFillTestName(n, m, test_type, test_name, 100);
  if (verbose) {
    printf("Dumping Cilkview file %s\n", test_name);
  }
  cv.dump(test_name);  
#endif

  {
    time_in_sec = (end_time - start_time) / 1000.f;
    constant_val = 1e6 * (end_time - start_time) / (scale);

    if (verbose) {
      printf("** %s, P = %d: Running time of %d by %d: %f seconds total, (n+m)*m*n constant= %f **\n ",
	     test_string,
	     P,
	     n, m,
	     time_in_sec,
	     constant_val);
      printf("Final answer: M(%d, %d) = %d\n",
	     n, m, answer);
    }
    else {
      printf("%d, %d, %d, %f, %f, %d, %d ",
	     P,
	     B,
	     test_type,
	     time_in_sec,
	     P*constant_val,
	     n,
	     m);	     
      printf(" ; //Key: P, B, Test Type, Time(s), Constant, N, M\n");
    }
  }

  if (run_gold) {
    assert(answer == ans_gold);
    printf("Answers are identical\n");

  }
  else {
    if (verbose) {
      printf("Completed run of test type %d\n",
	     test_type);
    }
  }

#ifdef TRACK_THREAD_CPU_IDS
  sw_global_stats->global_time_barrier(P);
  process_sw_log(n, B, test_type, P, verbose, time_in_sec);
  delete sw_global_stats;
  sw_global_stats = NULL;
#endif
  
     
  delete[] gamma;
  delete s;
  delete M;
  if (run_gold) {
    delete M2;
    delete s2;
  }


  return 0;
}

