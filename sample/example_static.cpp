/* sample.cpp                  -*-C++-*-
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



// Sample program that illustrates how to use Static Nabbit.
//
// TODO(jsukha): Add an example that illustrates how to use dynamic
// Nabbit.

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "sample_nabbit_node.h"

typedef enum {
    TEST_SERIAL = 0,
    TEST_STATIC_NABBIT = 1,
    TEST_ALL,
} SampleTestType;


// Constructs a DAG with sink node 0, and
// source node SAMPLE_DAG_SIZE-1.
//
// The value of each node is its key + the values of its
// immediate predecessors.
template <class NodeType>
void create_static_DAG(SampleDAGNode<NodeType>* nodes, int n) {
    assert(n <= SAMPLE_DAG_SIZE);
    for (int i = 0; i < n; i++) {
        nodes[i].key = i;
        nodes[i].params = NULL;
        nodes[i].init_node();    
    }

    nodes[0].add_dep(&nodes[1]);
    nodes[0].add_dep(&nodes[2]);

    nodes[1].add_dep(&nodes[3]);
    nodes[1].add_dep(&nodes[4]);
    nodes[1].add_dep(&nodes[5]);

    nodes[2].add_dep(&nodes[3]);
    nodes[2].add_dep(&nodes[5]);

    nodes[3].add_dep(&nodes[6]);
    nodes[4].add_dep(&nodes[6]);
    nodes[5].add_dep(&nodes[7]);

    nodes[6].add_dep(&nodes[SAMPLE_DAG_SIZE-1]);
    nodes[7].add_dep(&nodes[SAMPLE_DAG_SIZE-1]);
}


void run_test(SampleTestType test_type) {  
    switch (test_type) {
    case TEST_SERIAL:
    {
        SampleDAGNode<StaticSerialNode> nodes[SAMPLE_DAG_SIZE];
        create_static_DAG(nodes, SAMPLE_DAG_SIZE);
        nodes[SAMPLE_DAG_SIZE-1].source_compute();
        assert(nodes[0].result == 55);
    }
    break;
    case TEST_STATIC_NABBIT:
    {
        SampleDAGNode<StaticNabbitNode> nodes[SAMPLE_DAG_SIZE];
        create_static_DAG(nodes, SAMPLE_DAG_SIZE);
        nodes[SAMPLE_DAG_SIZE-1].source_compute();
        assert(nodes[0].result == 55);
    }
    break;    
    default:
        printf("No test type %d\n", test_type);
        assert(0);
    }  
}
				
int main(int argc, char *argv[])
{

    SampleTestType test_type = TEST_SERIAL;
    int P = NABBIT_WKR_COUNT;
  
    if (argc >= 2) {
        test_type = (SampleTestType)atoi(argv[1]);
        printf("Test_type = %d, P = %d\n",
               test_type, P);
        run_test(test_type);
    }
    else {
        int q;
        for (q = (int)TEST_SERIAL;
             q < (int)TEST_ALL;
             q++) {

            printf("*************************\n");
            printf("Test_type = %d, P = %d\n",
                   q, P);
            run_test((SampleTestType)q);
            printf("\n\n");
        }
    }  
    return 0;
}


