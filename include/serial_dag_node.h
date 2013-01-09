/* serial_dag_node.h                  -*-C++-*-
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

#ifndef __SERIAL_DAG_NODE_H_
#define __SERIAL_DAG_NODE_H_


#include <dag_status.h>
#include <dynamic_array.h>

class SerialDAGNode;


typedef DynamicArray<SerialDAGNode*> DAGNodeArray;


class SerialDAGNode {

 public:
  long long key;
  DAGNodeArray* children;

  // Constructors
  SerialDAGNode(long long k);
  SerialDAGNode(long long k,
		int num_predecessors);
  ~SerialDAGNode();

  // Methods to call when constructing the DAG statically.
  void init_node(int default_children_count);
  void init_node();
  void add_child(SerialDAGNode* child);

  void root_visit();  

 protected:
  DAGNodeStatus get_status();      

  virtual void InitNode() = 0;
  virtual void Compute() = 0;

 private:
  // Status field
  DAGNodeStatus volatile status;

  // Methods which change and get status.
  inline void mark_as_visited();
    
};




SerialDAGNode::SerialDAGNode(long long k) 
  :  key(k),
     children(NULL),
     status(NODE_UNVISITED) {
}

// The same as the previous construct, except we pass in a default
// size for the blocking array.
SerialDAGNode::SerialDAGNode(long long k,
			     int num_predecessors)
  :  key(k),
     children(NULL),
     status(NODE_UNVISITED) {
}
     
SerialDAGNode::~SerialDAGNode() {
  if (this->children) {
    delete this->children;
  }
}

void SerialDAGNode::mark_as_visited() {
  bool valid = (this->status == NODE_UNVISITED);
  this->status = NODE_VISITED;
  assert(valid);
}

DAGNodeStatus SerialDAGNode::get_status() {
  return this->status;
}


/***************************************************************/
// Methods for constructing the dag statically. 
void SerialDAGNode::init_node(int default_children_count) {
  this->children = new DAGNodeArray(default_children_count);
  // Call user-defined initialization.
  this->InitNode();
}

void SerialDAGNode::init_node() {
  init_node(5);
}

void SerialDAGNode::add_child(SerialDAGNode* child) {
  this->children->add(child);
}


/***************************************************************/
// Method for traversing the dag. 

void SerialDAGNode::root_visit(void) {
  assert(this->get_status() == NODE_UNVISITED);
  this->mark_as_visited();

  int num_children = this->children->size_estimate();
  for (int i = 0; i < num_children; ++i) {
    SerialDAGNode* child_node = (SerialDAGNode*)this->children->get(i);
    if (child_node->get_status() == NODE_UNVISITED) {
      child_node->root_visit();
      assert(child_node->get_status() == NODE_VISITED);
    }
  }
  this->Compute();
}


#endif // __SERIAL_DAG_NODE_H_
