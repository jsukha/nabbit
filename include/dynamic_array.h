/* dynamic_array.h                  -*-C++-*-
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


#ifndef __DYNAMIC_ARRAY_H
#define __DYNAMIC_ARRAY_H


/**
 * An implementation of a simple concurrent dynamic array, which
 * supports insertions at the end of the array.
 *
 * When the array gets filled, it doubles in size (serially).
 *
 * The array is optimized for inserts; a "get" must wait until any
 * previous inserts complete before it can continue.
 *
 *
 * TBD(jsukha): This implementation is horrid.  I have only myself to
 * blame. :)
 *  
 * I should really simplify it; in most cases, we really don't care to
 * change the number of children in the node anyway, so the dynamic
 * array is overkill for static Nabbit.
 */

#include <assert.h>
#include <stdio.h>
#include "nabbit_sysdep.h"

#define PRINT_LOCK_ACQUIRE_TRACE 0

template <class T>
struct DynamicArrayBuffer {
  T* a;
  int capacity;
  DynamicArrayBuffer<T>* next;
};

template <class T>
void dynarray_print_item(T* val);

void dynarray_print_item(int* val) {
  printf("%d", *val);
}

template <class T>
void dynamic_array_buffer_print(DynamicArrayBuffer<T>* buf) {
  printf("DynArrayBuffer %p: ", buf);
  if (buf != NULL) {
    printf("cap = %d, next = %p, a = %p: [",
	   buf->capacity,
	   buf->next,
	   buf->a);

    for (int i = 0; i < buf->capacity; i++) {
      //      dynarray_print_item(&buf->a[i]);
      //      printf(", ");
    }
    printf("]");
  }
}


template <class T>
class DynamicArray {

 private:
  T* a;
  volatile long capacity;
  volatile long current_size;
  volatile long inserted_elements;
  volatile int resize_lock;

  DynamicArrayBuffer<T>* old_arrays;

  bool try_acquire_resize_lock();
  void release_resize_lock();

  // Doubles the size of the array  
  void resize_array_grow();

  
 public:
  
  DynamicArray(int init_capacity);
  ~DynamicArray();

  void print();

  int size_estimate();

  // Get is not necessarily a constant time operation;
  // In our implementation, it must wait for any inserts which start before
  // this get operation to complete.
  T get(int idx);

  T get_with_print(int idx);
  
  void add(T val);
  bool try_atomic_add(T val);
    
};


template <class T>
DynamicArray<T>::DynamicArray(int init_capacity) {

  assert(init_capacity > 0);
  this->capacity = init_capacity;
  this->current_size = 0;
  this->inserted_elements = 0;
  this->a = new T[init_capacity];

  //  printf("Allocated this->a = %p (cap = %d)\n",
  //	 this->a, init_capacity);
  this->resize_lock = 0;
  this->old_arrays = NULL;
}

template <class T>
DynamicArray<T>::~DynamicArray() {

  DynamicArrayBuffer<T>* current_old_arrays = this->old_arrays;

//  printf("HERE: dynamic array %p. old_arrays = %p\n",
//	 this, this->old_arrays);
  while (current_old_arrays != NULL) {
      this->old_arrays = this->old_arrays->next;
//      printf("Deleting old array %p, capacity = %d\n",
//             current_old_arrays->a,
//             current_old_arrays->capacity);

      delete[] current_old_arrays->a;
      delete current_old_arrays;
      current_old_arrays = this->old_arrays;
  }
  
  delete[] this->a;
}


template <class T>
int DynamicArray<T>::size_estimate() {
  return current_size;
}

template <class T>
void DynamicArray<T>::print() {
  printf("*******************\n");
  printf("DynamicArray %p: ", this);
  printf("current_size = %ld, inserted_elements = %ld, capacity = %ld, ",
	 this->current_size,
	 this->inserted_elements,
	 this->capacity);
  printf("a = %p\n", this->a);


  bool print_elems = true;

  if (print_elems) {
    printf("Elements = [");
    for (int i = 0; i < this->current_size; i++) {
      dynarray_print_item(&this->a[i]);
      printf(", ");
    }
    printf("]\n");
    printf("old_arrays = %p:\n",
	   this->old_arrays);

    DynamicArrayBuffer<T>* old_arrays = this->old_arrays;
    while (old_arrays != NULL) {
      dynamic_array_buffer_print(old_arrays);
      old_arrays = old_arrays->next;
    }
  }
  printf("*******************\n");
}


template <class T>
bool DynamicArray<T>::try_acquire_resize_lock() {

    volatile bool acquired = false;
    int retry_count = 0;
    while ((!acquired) && (retry_count < 10)) {
        acquired = nabbit::try_lock_acquire(&this->resize_lock);
        retry_count++;
    }
    return acquired;
}

template <class T>
void DynamicArray<T>::release_resize_lock() {
    nabbit::lock_release(&this->resize_lock);
}


template <class T>
T DynamicArray<T>::get(int idx) {
  if ((idx >= 0) && (idx < this->current_size)) {

    // Spin waiting for the inserted_element count to catch up to
    // number of elements we have inserted.
    // 
    // In theory, this "get" function can wait a long time if inserts
    // keep changing the size of the array.

    int spin_count = 0;
    while (this->inserted_elements < this->current_size) {
      spin_count++;
    }
    return a[idx];
  } else {
    return (long long int)NULL;
  }
}


template <class T>
T DynamicArray<T>::get_with_print(int idx) {
  if ((idx >= 0) && (idx < this->current_size)) {


    printf("Searching for idx %d, array a = %p\n",
	   idx, a);
    // Spin waiting for the inserted_element count to catch up to the
    // index slot we are looking at.
    while (this->inserted_elements < idx) {
      printf("SPIN waiting for idx = %d, element_count = %ld\n",
	     idx, this->inserted_elements);
    }

    nabbit::system_full_memory_barrier();


    printf("Returning value %d\n",
	   a[idx]);
    return a[idx];
  } else {
    return NULL;
  }
}


// This method is not thread-safe yet, even though it acquires a lock.
// The problem is, no lock is held while elements are being inserted.
// So it could be that we copy over garbage values when we are doing
// the copy-over.
//
// Also, we might free the original array after an insert has acquired
// a slot, but before it finishes its insert.
// 
// So we need a "valid" bit to make sure the elements we are copying
// over are valid?  Or we could have a "reader lock" on the entire
// array when we are doing inserts.
//
// 
// 
template <class T>
void DynamicArray<T>::resize_array_grow() {
  T* old_array = a;
  volatile bool got_lock = false;

  //  while (!got_lock) {

  if (this->current_size >= this->capacity) {    
    got_lock = this->try_acquire_resize_lock();
  }

  if (!got_lock) {
    return;
  }


  if (PRINT_LOCK_ACQUIRE_TRACE) {
    printf("Worker %d Acquired the lock here. size = %ld, inserted_elements = %ld, capacity = %ld\n",
	   NABBIT_WKR_ID,
	   this->current_size,
	   this->inserted_elements,
	   this->capacity);
  }
  assert(got_lock);
  assert(this->resize_lock > 0);


  // We might get the lock even though we don't need to resize.  In
  // this case, there is no need to resize, and we can release the
  // lock.
  if (this->current_size < this->capacity) {
    if (PRINT_LOCK_ACQUIRE_TRACE) {
      printf("Worker %d Release lock for unneeded resize... current size = %ld, current cap = %ld\n",
	     #ifdef HAVE_ICC
	     __cilkrts_get_worker_number(), 
	     #else
	     NABBIT_WKR_ID,
	     #endif
	     this->current_size,
	     this->capacity);
    }
    this->release_resize_lock();
    return;
  }
  
  
  int new_capacity = this->capacity * 2;

  T* new_buffer = new T[new_capacity];
  assert(new_buffer != NULL);

  // Push the old array onto the linked list of old arrays.
  DynamicArrayBuffer<T>* old_array_buffer;
  old_array_buffer = new DynamicArrayBuffer<T>;

  //  printf("Allocated old_array_buffer (%p)\n",
  //	 old_array_buffer);

  assert(old_array_buffer != NULL);
  old_array_buffer->a = old_array;
  old_array_buffer->capacity = this->capacity;
  old_array_buffer->next = this->old_arrays;
  this->old_arrays = old_array_buffer;

  // Check to see if outstanding inserts have finished.
  volatile int wait_count = 0;
  while (this->inserted_elements < this->capacity) {
    wait_count++;
    nabbit::system_full_memory_barrier();
  }
  assert(this->current_size == this->inserted_elements);


  // Now we know we can copy over all the elements.
  for (int i = 0; i < this->current_size; i++) {
    new_buffer[i] = old_array[i];
  }
  
  
  // Memory barrier to make sure things are copied out.
  nabbit::system_full_memory_barrier();


  // Actually swing the pointer from the old array to the new array.
  this->a = new_buffer;
  nabbit::system_full_memory_barrier();


  // Update the capacity of the array. This update will allow more
  // inserts to happen.
  this->capacity = new_capacity;

  if (PRINT_LOCK_ACQUIRE_TRACE) {
    printf("Worker %d Release lock. current size = %ld, current cap = %ld\n",
	   NABBIT_WKR_ID,
	   this->current_size,
	   this->capacity);
  }
  this->release_resize_lock();

  //delete[] old_array;
}


/**
 * Adds to the array without synchronization.  This method should be
 * called only when we know it is executing serially.
 */
template <class T>
void DynamicArray<T>::add(T val) {
  int idx;
  if (this->current_size >= this->capacity) {
    this->resize_array_grow();
  }
  
  assert(this->current_size < this->capacity);
  idx = this->current_size;
  this->current_size++;
  a[idx] = val;
  this->inserted_elements++;
}


/**
 * Atomically adds to the end of the array.  Uses a compare-and-swap
 * for synchronization.
 *
 * If the the array is full, it the operation will resize the array
 * first and then try to insert again.
 *
 * Returns true if insert succeeded, and false otherwise.
 */
template <class T>
bool DynamicArray<T>::try_atomic_add(T val) {

    long idx = -1;
    int retry_count = 0;

    // This first loop tries to reserve an index in the dynamic array
    // for the new element.
  
    while ((idx < 0) && (retry_count < 10)) {
        if (this->current_size >= this->capacity) {
            this->resize_array_grow();
        }
    
        volatile long temp_size = this->current_size;
        if (temp_size < this->capacity) {
            bool got_space = nabbit::long_CAS(&this->current_size,
                                              temp_size,
                                              temp_size+1);
            if (got_space) {
                idx = temp_size;
            }
        }
        retry_count++;
    }

    // If idx is still -1, then we didn't get a slot.
    if (idx < 0) {
        return false;
    }

    // Otherwise, we have a slot.  Add our element.
    a[idx] = val;

    // Then, atomically update the inserted elements count.  This atomic
    // increment should already be a full memory barrier.
    nabbit::atomic_add_and_fetch(&this->inserted_elements, 1);
    return true;  
}






#endif // __DYNAMIC_ARRAY_H
