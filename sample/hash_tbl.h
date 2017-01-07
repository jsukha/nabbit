// Code for the Nabbit task graph library
// Hash Table Implemented in the Random DAG microbenchmark.
//
// Copyright (c) 2010 Jim Sukha
//
//
/*
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __HASH_TBL_H_
#define __HASH_TBL_H_

#include <concurrent_hash_table.h>

typedef long long node_key_t;
typedef DynamicNode *(*new_node_f)(node_key_t, void *);

class DynHashTable: public TaskGraphHashTable {

    public:  
        ConcurrentHashTable* H;  
        new_node_f new_node;

        DynHashTable(int n, new_node_f mk)
            : H(new ConcurrentHashTable(n)), new_node(mk) {};
        ~DynHashTable() { delete H; };

        void* get_task(node_key_t key);

        int insert_task_if_absent(node_key_t key);
};


void *DynHashTable::get_task(node_key_t key) {
    DynamicNode *n = NULL;
    LOpStatus code = OP_FAILED;
    while (code == OP_FAILED) {
        n = (DynamicNode *)H->search(key, &code);
    }

    if (n != NULL) {
        if (n->get_status() >= NODE_VISITED) {
            return (void*)n;
        }
    }
    return NULL;
}

struct State2 {
  void *stack, *next;
  unsigned int type;
  int deps;
  int path;
  const char name[8];
};

int DynHashTable::insert_task_if_absent(node_key_t key) {

    DynamicNode *n = NULL;
    LOpStatus code = OP_FAILED;
    bool success = false;
    while (code == OP_FAILED) {
        n = (DynamicNode *)H->search(key, &code);
    }
    if(n == NULL) { // new node
        DynamicNode *node = new_node(key, H);
        code = OP_FAILED;
        while(code == OP_FAILED){
            n = (DynamicNode *)H->insert_if_absent(key, (void *)node, &code);
        }
        assert(code == OP_INSERTED);
    }

    assert(n != NULL);
    if (n->get_status() >= NODE_VISITED) {
#if PRINT_DEBUG_STATEMENTS == 1
        printf("HASH insert if absent on key %llu. failed\n", key);
#endif
        return 0;
    }

    while (n->get_status() == NODE_UNVISITED) {
        success = n->try_mark_as_visited();
    }

    if (success) {
#if PRINT_DEBUG_STATEMENTS == 1
        printf("HASH insert if absent on key %llu. success\n", key);
#endif    
        return 1;
    } else {
        assert(n->get_status() >= NODE_VISITED);
#if PRINT_DEBUG_STATEMENTS == 1
        printf("HASH insert if absent on key %llu. failed\n", key);
#endif
        return 0;
    }
}

#endif
