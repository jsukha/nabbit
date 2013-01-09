/* sw_test_types.h                 -*-C++-*-
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
//
// Header file defining the various test types for dynamic
// program example.

#ifndef _SW_TEST_TYPES_H_
#define _SW_TEST_TYPES_H_


typedef enum {
    SW_GENERIC=0,
    SW_DC_K2=1,
    SW_DC_GENERIC_K=2,
    SW_PURE_WAVEFRONT=3,
    SW_STATIC_NABBIT=4,
    SW_STATIC_SERIAL=5,
    SW_MAX_TYPE,
} SWComputeType;

static const char* SWTestTypeNames[] = {
    "Generic",
    "DC_K2",
    "DC_GenericK",
    "Wavefront",
    "StaticNabbit",
    "StaticSerial",
};


#ifdef _WIN32
#    define SNPRINTF(buffer, length, ...)  _snprintf_s((buffer), (length), (length), __VA_ARGS__)
#else
#    define SNPRINTF(buffer, length, ...)  snprintf((buffer), (length), __VA_ARGS__)
#endif


// Create a name for this test run.
inline static void SWFillTestName(int n, int m, int test_type,
				  char* test_name, int name_length) {
    assert(test_type >= 0);
    assert(test_type < SW_MAX_TYPE);
    SNPRINTF(test_name, name_length,
             "sw%d_%d_Typ%s",
             n, m,
             SWTestTypeNames[test_type]);
}

#endif  // _SW_TEST_TYPES_H_
