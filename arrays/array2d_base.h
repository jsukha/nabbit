/* array2d_base.h                  -*-C++-*-
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

// Nabbit, Array Indexing Library

#ifndef __ARRAY2D_BASE__
#define __ARRAY2D_BASE__


/******************************************/
// Base class for a simple 2d array.
//
// This class contains only data and a simple initialization method.

#include "array_layouts.h"


template <class T>
class NabbitArray2DBase {
  
 protected:

  ArrayLargeDim total_size;
  ArrayDim width;
  ArrayDim height;
  NabbitArray2DLayout layout;  
  T* data;

 public:

  NabbitArray2DBase(ArrayDim width,
		    ArrayDim height,
		    NabbitArray2DLayout layout);

  ~NabbitArray2DBase();

  inline T* get_data() { return data; }
};


template <class T>
NabbitArray2DBase<T>::NabbitArray2DBase(ArrayDim width,
					ArrayDim height,
					NabbitArray2DLayout the_layout) :
  total_size(0), width(width), height(height), layout(the_layout), data(NULL)
{

}

template <class T>
NabbitArray2DBase<T>::~NabbitArray2DBase() {
/*   printf("Destroying 2D array with width = %d, height = %d, ", */
/* 	 this->width, this->height); */
/*   printf("layout = %d, total_size = %lu\n", */
/* 	 this->layout, (size_t)this->total_size); */
  assert(data != NULL);
  delete [] this->data;
}
  


#endif // __ARRAY2D_BASE__

