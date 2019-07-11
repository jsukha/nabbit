/* SWDagNode.h                 -*-C++-*-
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

// This file defines the Nabbit static task graph node for the dynamic
// program.


#ifndef __SW_DAG_NODE_H
#define __SW_DAG_NODE_H

#include <nabbit.h>
#include <arrays/array2d_base.h>
#include <arrays/array2d_morton.h>

#include "sw_matrix_kernels.h"
#include "SWDagParams.h"

// #define DEBUG_PRINT

template <class NodeType>
class SWDAGNode: public NodeType {

 private:
  friend class SWDAGParams<SWDAGNode<NodeType> >;    
  SWDAGParams<SWDAGNode<NodeType> >* params;
  int result;

  SWDAGNode(long long k,
	    SWDAGParams<SWDAGNode<NodeType> >* params);
  
  void InitNode();
  void Compute();

#ifdef TRACK_THREAD_CPU_IDS  
  // int init_id;
  // No visit image unless we are using dynamic Nabbit. 
  int compute_id;
#endif

  
 public:
  SWDAGNode();

  int GetResult();

  void CheckResult();
  
};


template <class NodeType>
SWDAGNode<NodeType>::SWDAGNode()
  : NodeType(0, 3)
{
  
}


template <class NodeType>
SWDAGNode<NodeType>::SWDAGNode(long long k,
			       SWDAGParams<SWDAGNode<NodeType> >* params)
  : NodeType(k),
    params(params) {
}
		    
template <class NodeType>
void SWDAGNode<NodeType>::InitNode() {
  this->result = 0;
}

template <class NodeType>
void SWDAGNode<NodeType>::Compute() {
  assert(this->get_status() == NODE_EXPANDED);  
  this->result = params->ComputeAtKey(this->key);
}

template <>
void SWDAGNode<StaticSerialNode>::Compute() {
  this->result = params->ComputeAtKey(this->key);
}

template <>
void SWDAGNode<StaticNabbitNode>::Compute() {
  this->result = params->ComputeAtKey(this->key);

#ifdef TRACK_THREAD_CPU_IDS
  this->compute_id = cilk::current_worker_id();
#endif  
}


template <class NodeType>
int SWDAGNode<NodeType>::GetResult() {
  return this->result;
}


#endif  // __SW_DAG_NODE_H
