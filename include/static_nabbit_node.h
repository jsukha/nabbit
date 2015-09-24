/* static_nabbit_node.h                  -*-C++-*-
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

#ifndef __STATIC_NABBIT_NODE_H_
#define __STATIC_NABBIT_NODE_H_

#include "dag_status.h"
#include "dynamic_array.h"
#include "nabbit_sysdep.h"

// Debugging flag.
// #define STATIC_NABBIT_PRINT_DEBUG 1

class StaticNabbitNode;
typedef DynamicArray<StaticNabbitNode*> StaticNabbitNodeArray;


class StaticNabbitNode {

 public:
  long long key;
  StaticNabbitNodeArray* predecessors;
  StaticNabbitNodeArray* successors;
  //  StaticNabbitNodeArray* children;

  // Constructors for a node.
  StaticNabbitNode(long long k);
  StaticNabbitNode(long long k, int num_predecessors);

  ~StaticNabbitNode();

  // Methods to call when constructing a DAG statically.
  void init_node(int default_degree);
  void init_node();

  void add_dep(StaticNabbitNode* child);
  
  void add_child(StaticNabbitNode* child);
  void source_compute();
  
 protected:
  virtual void InitNode() = 0;
  virtual void Compute() = 0;

 private:
  volatile long join_counter;
  void compute_and_notify();

};


StaticNabbitNode::StaticNabbitNode(long long k) 
  :  key(k),
     predecessors(NULL),
     successors(NULL) {
}

StaticNabbitNode::StaticNabbitNode(long long k, int num_predecessors) 
  :  key(k),
     predecessors(NULL),
     successors(NULL) {
    (void)num_predecessors; // UNUSED parameter.
}

     
StaticNabbitNode::~StaticNabbitNode() {
  if (this->predecessors) {
    delete this->predecessors;
  }
  if (this->successors != NULL) {
    delete this->successors;
  }
}


/***************************************************************/
// Methods for constructing the dag statically. 

void StaticNabbitNode::init_node(int default_degree) {

  this->predecessors = new StaticNabbitNodeArray(default_degree);
  this->successors = new StaticNabbitNodeArray(default_degree);
  this->join_counter = 0;
  //  this->children = this->predecessors;

  // Call user-defined initialization.
  this->InitNode();
}

void StaticNabbitNode::init_node() {
  init_node(5);
}



// Both "this" node and dep_node should have been initialized already.
void StaticNabbitNode::add_dep(StaticNabbitNode* dep_node) {

  // Add an edge from dep_node -> this.
  this->predecessors->add(dep_node);
  dep_node->successors->add(this);
  nabbit::atomic_add_and_fetch(&this->join_counter,
                               1);
}

void StaticNabbitNode::add_child(StaticNabbitNode* dep_node) {
  // Add an edge from dep_node -> this.
  this->predecessors->add(dep_node);
  dep_node->successors->add(this);
  nabbit::atomic_add_and_fetch(&this->join_counter,
                               1);
}


void StaticNabbitNode::source_compute(void) {
  this->compute_and_notify();
}


/***************************************************************/
// Methods which call Compute() and do bookkeepping.

void StaticNabbitNode::compute_and_notify() {

#if STATIC_NABBIT_PRINT_DEBUG == 1
  printf("COMPUTE AND NOTIFY called on key %lld, worker %d\n",
  	 this->key,
	 NABBIT_WKR_ID);
#endif
  this->Compute();
  
  int end_to_notify = this->successors->size_estimate();

  // Handle the current range of values in the blocking array.
  //    cilk_for (int i = this->notify_counter; i < end_to_notify; i++) {
  for (int i = 0; i < end_to_notify; i++) {

    StaticNabbitNode* current_succ = this->successors->get(i);
    if (current_succ->join_counter <= 0) {
      /*printf("ERROR: this key = %lld, current_succ = %p (key = %lld), join counter = %ld\n",
	     this->key,
	     current_succ, current_succ->key,
	     current_succ->join_counter);*/
        //printf("ERROR: negative join counter!\n"); // even this gives "function not inlinable..." in g++ 4.9.1
    }
    assert(current_succ->join_counter > 0);
    int updated_val = nabbit::atomic_sub_and_fetch(&(current_succ->join_counter),
                                                   1);

    if (updated_val == 0) {
#if STATIC_NABBIT_PRINT_DEBUG == 1
      printf("Worker %d enabling current_pred with key = %llu.\n",
	     NABBIT_WKR_ID,
	     current_succ->key);
#endif
      cilk_spawn current_succ->compute_and_notify();
    }
  }
  cilk_sync;
}

#endif // __STATIC_NABBIT_NODE_H_
