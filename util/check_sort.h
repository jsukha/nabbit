/* check_sort.h                  -*-C++-*-
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


// Simple routine to check whether arrays are sorted. 

#ifndef __CHECK_SORT_H_
#define __CHECK_SORT_H_

#include <iostream>
#include <cstdlib>

// Confirm that a is sorted and that each element contains the index.
void check_sort(long long* a, long long n) {
  for (long long i = 0; i < n - 1; ++i) {
    if (a[i] >= a[i + 1] || a[i] != i) {
      std::cout << "Sort failed at location i=" << i << " a[i] = "
		<< a[i] << " a[i+1] = " << a[i + 1] << std::endl;
    }
  }
  std::cout << "Sort succeeded." << std::endl;
}

// Checks that a[i] < a[i+1] for all i in [0, n-1)
void check_strictly_increasing(long long* a, long long n) {
  int num_errors = 0;
  for (long long i = 0; i < n-1; ++i) {
    if (a[i] >= a[i+1]) {
      std::cout << "Not strictly increasing at location i=" << i << " a[i] = "
		<< a[i] << " a[i+1] = " << a[i+1] << std::endl;
      num_errors++;
    }    
  }
  std::cout << "Errors in sort: " << num_errors << std::endl;
}


#endif //__CHECK_SORT_H_
