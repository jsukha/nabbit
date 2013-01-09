/* nabbit_node.h                  -*-C++-*-
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


#ifndef __NABBIT_NODE_H_
#define __NABBIT_NODE_H_

/**************************************************
 * nabbit_node.h
 *
 *  This header file wraps all the various DAG nodes we have
 *  implemented so far into one templated class.
 * 
 *  This structure (hopefully?) makes it easier for users to switch
 *  between DAG nodes, by switching the template parameter.
 *
 *  Alternatively, the user can subclass from the DAG node type
 *  directly.
 */


#include "static_serial_node.h"
#include "static_nabbit_node.h"
#include "dynamic_serial_node.h"
#include "dynamic_nabbit_node.h"


// Possible status for a node.
typedef enum { SERIAL_STATIC_TRAVERSAL=0,
	       STATIC_NABBIT_TRAVERSAL=1,
	       SERIAL_DYNAMIC_TRAVERSAL=2,
	       DYNAMIC_NABBIT_TRAVERSAL=3
} DAGTraversalType;


// The supported types of nodes: 
//
// 1. StaticSerialNode
// 2. StaticNabbitNode
// 3. DynamicSerialNode
// 4. DynamicNabbitNode


template <class NodeType>
class NabbitNode: public NodeType {

 protected:
  NabbitNode(long long k);
  NabbitNode(long long k,
             int num_predecessors);  
};


template <class NodeType>
NabbitNode<NodeType>::NabbitNode(long long k)
  :  NodeType(k) {
}

template <class NodeType>
NabbitNode<NodeType>::NabbitNode(long long k, int num_predecessors)
  :  NodeType(k, num_predecessors) {
}


#endif // _NABBIT_NODE_H_

