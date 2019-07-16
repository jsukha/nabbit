Dynamic Programmimg Benchmark for Nabbit
-------------------------------------------

This directory contains code for a dynamic program computed using
static Nabbit.

The dynamic programming is an O(n^3) version which models the
Smith-Waterman algorithm with a generic penalty gap.


There are several parameters built into the code that can be changed
to experiment with different scenarios.

1. To change how the E(i, j) and F(i, j) are computed, see
   `sw_ComputeEF.h`.

2. To model an O(n^2) version of the dynamic program, compile with
   `-DCOMPUTE_CONSTANT_EF`.

3. To change the layout of the M and S matrices between row-major and
   cache-oblivious layout, change the MType and SType definitions in
   `sw_compute.cpp`.

4. `K_VALUE`: Changes the value of K for the generic divide-and-conquer
   algorithm.  The default setting is K = 5.

5. `BLOCK_VALUE`: Changes the base case of the algorithms. Default value
   is B = 16.  See Makefile for compilation of versions of different
   block size.


Code organization:


`SWDagParams.h`: 	   Defines parameters of the simulation, other
		   basic data structures.

`sw_test_types.h`:   Defines enum for the different tests types.

`sw_compute`:   	   The main executable file.
`sw_computeEF`:  	   Methods for computing E(i, j) and F(i, j)

`sw_matrix_kernels`: Definition of the base case methods, and the
		   divide-and-conquer and wavefront algorithms for
		   computing the dynamic program.

`SWDagNode.h`: 	   Definition of the Static Nabbit code for the dynamic
		   program.

`matrix_utils`:	   Some code for copying matrices / converting between layouts. 
	   
`sw_test.sh`: 	   Sample script for running the test program.


To run the program:

```
CILK_NWORKERS=$P ./swblock_16  <N> <M> <test_type> <verbose>
```

For now, we require that N = M; this limitation mainly only for
convenience for the cache-oblivious layout.
test_type is the number, as defined by the enum in `sw_compute.cpp`
verbose = 0 to minimize printing, 1 otherwise.
