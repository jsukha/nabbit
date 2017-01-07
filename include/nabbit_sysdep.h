/* nabbit_sysdep.h                  -*-C++-*-
 *
 *************************************************************************
 *
 * Copyright (c) 2013, Jim Sukha
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


/**
 * This file defines some macros, etc. for dealing with different
 * versions of Cilk and other generally sysdep-dependent functions.
 *
 *
 * TBD(jsukha): My implementation is terrible.  
 * We need to go through and replace many of these functions
 * with std::atomic now that C++11 is more common.
 *
 */ 
#ifndef __NABBIT_SYSDEP_H_
#define __NABBIT_SYSDEP_H_


#ifdef _WIN32
#   include <windows.h>
#endif


// Intel Cilk Plus definitions.
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#define NABBIT_WKR_ID __cilkrts_get_worker_number()
#define NABBIT_WKR_COUNT __cilkrts_get_nworkers()


namespace nabbit {

#ifdef _WIN32
    inline long atomic_add_and_fetch(long volatile *p, int x)
    {
        return _InterlockedExchangeAdd(p, x) + x;
    }

    inline long atomic_sub_and_fetch(long volatile *p, int x)
    {
        return _InterlockedExchangeAdd(p, -x) - x;
    }
    
    inline bool int_CAS(int volatile *p, int old_val, int new_val) {
        long initial_val = _InterlockedCompareExchange((long*)p, new_val, old_val);
        return (initial_val == (long)old_val);
    }
    inline bool long_CAS(long volatile *p, long old_val, long new_val) {
        long initial_val = _InterlockedCompareExchange(p, new_val, old_val);
        return (initial_val == old_val);
    }            

    inline bool ptr_CAS(void volatile* p, void*  old_val, void* new_val) {
        LONG64 initial_val = _InterlockedCompareExchange64((LONG64*)p, (LONG64)new_val, (LONG64)old_val);
        return (initial_val == (LONG64)old_val);
    }            

    inline void system_full_memory_barrier(void)
    {
        __asm
        {
            mfence
        }
    }

    inline void system_pause() {
        __asm
        {
            pause
        }
    }

    inline void system_yield() {
        Sleep(0);
    }

#else
    // GCC-compatible systems.
#include <pthread.h>

    inline long atomic_add_and_fetch(long volatile* p, long x) {
        return __sync_add_and_fetch(p, x);
    }
    inline long atomic_sub_and_fetch(long volatile *p, long x) {
        return __sync_sub_and_fetch(p, x);        
    }
    inline bool int_CAS(int volatile *p, int old_val, int new_val) {
        return __sync_bool_compare_and_swap(p, old_val, new_val);
    }
    inline bool long_CAS(long volatile *p, long old_val, long new_val) {
        return __sync_bool_compare_and_swap(p, old_val, new_val);
    }
    inline bool ptr_CAS(void volatile *p, void* old_val, void* new_val) {
        return __sync_bool_compare_and_swap((size_t*)p, (size_t)old_val, (size_t)new_val);
    }            

    inline void system_full_memory_barrier() {
        __sync_synchronize();
    }

    inline void system_pause() {
        __asm__("pause");
    }

    inline void system_yield() {
        pthread_yield();
    }
    
#endif

    // TBD: These is likely to be a suboptimal spin-lock
    // implementations, since they are doing a CAS on both acquire and
    // release, which may perform more memory barriers than necessary.
    // Should be fixed...
    inline bool try_lock_acquire(int volatile* lock) {
        return int_CAS(lock, 0, 1);
    }

    inline void lock_acquire(int volatile* lock) {
        bool acquired = false;
        int spin_count = 0;

        while (!acquired) {
            acquired = int_CAS(lock, 0, 1);
            if (!acquired) {
                spin_count++;
                system_pause();
                if (spin_count > 1000) {
                    system_yield();
                }
            }
        }
    }
    
    inline void lock_release(int volatile* lock) {
        assert((*lock) == 1);
        bool success;
        do {
            success = int_CAS(lock, 1, 0);
        } while (!success);
    }
};



// For now, I'm deprecating the support for Cilk++.
// Given that Cilk++ is no longer supported by anyone, I'm not sure
// there is much point in keeping it...
// 
/// Cilk++ versions:
// #    define NABBIT_WKR_ID cilk::current_worker_id()
// #    define NABBIT_WKR_COUNT cilk::current_worker_count()

#endif // __NABBIT_SYSDEP_H_
