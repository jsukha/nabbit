/* sample_nabbit_node.h                  -*-C++-*-
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

// Sample code that illustrates how to use create a node for static Nabbit.

#ifndef __SAMPLE_NABBIT_NODE_H
#define __SAMPLE_NABBIT_NODE_H

#include <nabbit.h>


const int SAMPLE_DAG_SIZE = 10;

template <class NodeType>
class SampleDAGNode: public NodeType {

 private:
  void InitNode();
  void Compute();
  
 public:
  SampleDAGNode();
  void* params;
  int result;
  

};


template <class NodeType>
SampleDAGNode<NodeType>::SampleDAGNode() 
  : NodeType(0, 3),
    params(NULL) {    
}


template <class NodeType>
void SampleDAGNode<NodeType>::InitNode() {
  if (this->key < SAMPLE_DAG_SIZE-1) {
    this->result = (int)this->key;
  }
  else {
    // Source node has no value associated with it.
    this->result = 0;
  }
  printf("InitNode with key %lld: initialized result to %d\n",
	 this->key,
	 this->result);
}

template <class NodeType>
void SampleDAGNode<NodeType>::Compute() {

  for (int i = 0; i < this->predecessors->size_estimate(); i++) {
    SampleDAGNode<NodeType>* child = (SampleDAGNode<NodeType>*)this->predecessors->get(i);
    this->result += child->result;
  }

  printf("At key %lld: computed value %d\n",
	 this->key,
	 this->result);
}


#endif // __SAMPLE_NABBIT_NODE_H
