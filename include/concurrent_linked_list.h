/* concurrent_linked_list.h                  -*-C++-*-
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

#ifndef __CONCURRENT_LINKED_LIST_H
#define __CONCURRENT_LINKED_LIST_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "nabbit_sysdep.h"

/*************************************************
 * Implementing a simple concurrent linked list.
 * 
 * The implementation is geared to support insert_if_absent
 * efficiently; a concurrent delete is not implemented.
 *
 *************************************************/


// Possible status for a node.
typedef enum { UNINITIALIZED=0, DUMMY=1, INVALID = 4, VALID = 16, DEAD = 64 } LNodeStatus;


// Possible status values that can be returned
// for an operation.

typedef enum { OP_NULL = 0,
               OP_FOUND = 1,
	       OP_NOT_FOUND=2,
	       OP_FAILED = 3,
	       OP_INSERTED = 4,
	       OP_DELETED = 5,
	       OP_ERROR = 6,
	       OP_LAST = 7 } LOpStatus;
	       
	       

// A node
struct ListNode {
  long long hashkey;
  ListNode* next;
  LNodeStatus status;
  void* value;

  ListNode() :
    hashkey(0), next(NULL), status(DUMMY), value(NULL) { }
  
  ListNode(long long k) :
    hashkey(k), next(NULL), status(INVALID), value(NULL) { }
  
  ListNode(long long k, void* val) :
    hashkey(k), next(NULL), status(VALID), value(val) { }
  
  ListNode(long long k, void* val, ListNode* nxt) :
    hashkey(k), next(nxt), status(VALID), value(val) { }  
};

class ConcurrentLinkedList
{

 private:
  ListNode* head;

  long long size_estimate;
  // This field is a cache which is an (approximate) count of the
  // number of elements in the list.  The increment of this count
  // is not done atomically, so it may be incorrect.


  void delete_list_helper(ListNode* current) {
    ListNode* rest = NULL;
    if (current != NULL) {
      bool have_rest = false;
      if (current->next != NULL) {
	rest = current->next;
	have_rest = true;
      }
      delete current;
      if (have_rest) {
	delete_list_helper(rest);
      }
    }
  }

  

 public:
  ConcurrentLinkedList() : size_estimate(0) {
    head = new ListNode();
    assert(head != NULL);
    head->status = DUMMY;
    head->hashkey = 0;
  }

  
  ~ConcurrentLinkedList() {
    delete_list_helper(head);
  }


  ListNode* get_list_head() {
    if (head != NULL) {
      return head->next;
    }
    return NULL;
  }
  
  void print_node(ListNode* node) {
    if (node != NULL) {
      printf("(%p: k=%lld, val=%p, stat=%d)",
	     node,
	     node->hashkey,
	     node->value,
	     node->status);
    } else {
      printf("(%p: null)",
	     node);
    }
  }

  void print_list() {
    ListNode* current = head->next;
    printf("***********************\n");
    printf("**** List %p, Size=%lld: ",
	   head,
	   size_estimate);

    while (current != NULL) {
      print_node(current);
      printf("\n");
      current = current->next;
    }
    printf("***********************\n");
    printf("\n");
  }


  void update_size_estimate() {
    int updated_estimate = 0;

    ListNode* current = head->next;
    while (current != NULL) {
      updated_estimate++;
      current = current->next;
    }
    this->size_estimate = updated_estimate;
  }

  long long get_size_estimate(void) {
    return size_estimate;
  }

  void* search(long long k,
	       LOpStatus* status) {

    int retry_count = 0;

    while (retry_count < 10) {

      volatile ListNode* temp_first = head->next;

      ListNode* target = NULL;
      // Where we store the pointer to the linked list node containing
      // the value we are going to return.
      ListNode* current = head->next;

      // Pointer used when traversing the list.
      while ((current != NULL) &&
	     (target == NULL)) {
	if ((current->hashkey == k)
	    && (current->status != DEAD)) {
	  target = current;
	}
	current = current->next;
      }
      if (target) {
	*status = OP_FOUND;
	return target->value;
      }

      // If the head of the list is still the same, assume that the
      // element is not there.
      if (temp_first == head->next) {
	*status = OP_NOT_FOUND;
	return NULL;
      }
      retry_count++;
    }

    *status = OP_FAILED;
    return NULL;            
  }

	       

  /**
   * Attempts to atomically insert a key value pair (k, val) into the
   * linked list.  The "status" argument should be a pointer to the
   * location where the output code of the operation gets stored.
   *
   * This method can have 3 possible return codes.
   *
   * 1. OP_FOUND: A value for that key is already in the hash table.
   *               In this case, we return the "value" field of the
   *               element already in the table.
   * 2. OP_INSERTED: A value for this key was not found.
   *                  Create a new node with key value pair (k, val).
   *                  Method returns "val".
   * 3. OP_FAILED:  The operation failed too many times because of
   *                contention.   Returns NULL.
   */
  void* insert_if_absent(long long k,
			 void* val,
			 LOpStatus* status) {
    
    int retry_count = 10;
    ListNode* temp_node = NULL;

    while (retry_count > 0) {      
      ListNode* target = NULL;
      // Where we store the pointer to the linked list node containing
      // the value we are going to return.

      ListNode* temp_first = head->next;
      // Remembers the head of the list.

      
      ListNode* current = head->next;
      // Pointer used when traversing the list. 

      while ((current != NULL) &&
	     (target == NULL)) {
	if ((current->hashkey == k)
	    && (current->status != DEAD)) {
	  target = current;
	}
	current = current->next;
      }
      if (target) {
	*status = OP_FOUND;
	return target->value;
      }
      else {
	
	// Allocate a new node object to insert.
	if (temp_node == NULL) {
	  temp_node = new ListNode(k, val);
	  assert(temp_node != NULL);
	}
	temp_node->next = head->next;

	// Check to make sure the first element in the list is still
	// the same as the head we knew about originally.  If it has
	// changed, then we have already failed.
	// Otherwise, we will try to CAS the old head of the list
	// to the new temp node.


	// Check without CAS.
	if (temp_node->next == temp_first) {
	  
	  // The CAS operation:
	  // Abstractly, this performs the assignment,
	  //  "this->head->next = temp_node"
	  //  assuming the head of the list doesn't change. 

	  // The prototype for gcc's builtin CAS:
	  //  type __sync_val_compare_and_swap (type *ptr, type oldval type newval, ...)

	  bool valid = false;

          valid = nabbit::ptr_CAS(&this->head->next,
                                  temp_first,
                                  temp_node);

	  if (valid) {
	    // There is a race condition here...
	    // That's why it is an estimate.
	    this->size_estimate++;
	    *status = OP_INSERTED;
	    return temp_node->value;
	  }
	}
      }

      retry_count--;
    }

    *status = OP_FAILED;
    return NULL;
  }




  // Takes the current list, copies the keys of nodes in the list into
  // a newly allocated array, and returns a pointer to the array.
  //
  // After execution, "final_size" stores the number of elements in
  // the array.
  
  long long* get_keys(int* final_size) {    
    int n = 0;
    long long* a = NULL;

    ListNode* current = this->head;
    while (current->next != NULL) {
      n++;
      current = current->next;
    }

    //    printf("List has %d elements\n", n);
    if ( n > 0) {
      a = new long long[n];
      assert(a != NULL);
      current = this->head;
      int i = 0;
      while (current->next != NULL) {
	current = current->next;
	a[i] = current->hashkey;
	i++;
      }
      
      //      printf("Array: ");
      //      for (i = 0; i < n; i++) {
      //	printf("%d  ", a[i]);
      //      }
      *final_size = n;
    }    
    return a;
  }


  // Same as get_keys, except it only takes up to n
  // elements.
  void get_n_keys(long long* a,
		  long long n,
		  long long* final_size) {
    long long k = 0;
    ListNode* current = this->head;
    while ((current->next != NULL) && (k < n)){
      current = current->next;
      a[k] = current->hashkey;
      k++;
    }
    *final_size = k;
  }

    
  // Same as get_keys, only returns the values instead.  
  void** get_values(int* final_size) {    
    int n = 0;
    void** a = NULL;

    ListNode* current = this->head;
    while (current->next != NULL) {
      n++;
      current = current->next;
    }

    if ( n > 0) {
      a = new void* [n];
      current = this->head;
      int i = 0;
      while (current->next != NULL) {
	current = current->next;
	a[i] = current->value;
	i++;
      }
      *final_size = n;
    }
    return a;
  }

};

#endif // __CONCURRENT_LINKED_LIST_H

