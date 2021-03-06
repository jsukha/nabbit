Nabbit

----------------------------------------------------------------------
This folder contains the implementation of Nabbit, a Cilk library for
task graph evaluation.  Nabbit Version 1.1 is written for Cilk Plus.


Currently, Nabbit consists of a set of C++ header files; thus, there
are no binaries to link with.

The code is organized into the following folders:

`include`: The header files which make up the library.  Of these files,
  including `nabbit.h` should include all the other files.


`include/arrays`: Defines row-major and Morton-order 2-d arrays.  These arrays
                  are used only for the code in the `smith_waterman` directory.

`tests/arrays`: Some tests for the code from `include/arrays`.

`apps/sample`: A simple test program which demonstrates how to use the library.

        1.  Define a new DAG node class, which extends from one of the
        classes defined in nabbit_node.h

        2.  Define an InitNode() method which performs any specifics 
        initialization for the node.

        3.  Define a Compute() method, which computes the value for a
        node.  This method gets called only after the Compute() for
        all children of a node have completed.

        4.  When building a static DAG, use "init_node()" to create a
        node, and "add_child" to specify the children of a node.  The
        InitNode() method gets called by the init_node() method, after
        the library performs its own initialization.

        5.  On the root node of the DAG, call "source_compute()" to
        perform the DAG evaluation.

	By having each DAG node point to a global "parameters" data
	structure for the DAG, it is possible to access global
	variables.  This approach may be a bit tedious, but it works
	ok for the simple benchmarks we have.

	
`apps/smith_waterman`: A benchmark which performs a dynamic program
                       calculation with a similar recurrence as the dynamic
                       program for the Smith Waterman algorithm.

`tests/concurrent`: Some tests for the data structures defined for the library
                    (linked list, dynamic array, and hash table).


`toolchains`: CMake toolchain files for different configurations

`util`: Miscellaneous utilities used by `apps`.




The interface to the library is still changing.  Hopefully, we can
improve the interface (and possibly functionality) as we test more
applications.


For more documentation, please see the following paper in IPDPS 2010:


Executing Task Graphs Using Work-Stealing
by Kunal Agrawal, Charles E. Leiserson, and Jim Sukha
In Proceedings of the 24th IEEE International Parallel and Distributed Processing Symposium (IPDPS)
Atlanta, GA, USA
April 19--23, 2010


BibTex Entry:

```
@InProceedings{AgrawalLeSu10b,
 author       = {Kunal Agrawal and Charles E. Leiserson and Jim Sukha},
 title	      = {Executing Task Graphs Using Work-Stealing},
 booktitle    = {Proceedings of the 24th IEEE International Parallel and Distributed Processing Symposium (IPDPS)},
 address      = {Atlanta, GA, USA},
 month	      = APR,
 day          = {19--23},
 year	      = 2010,
}
```


Please feel free to send comments / suggestions to the authors.

Contact info:

Jim Sukha


---------------------------------------------
Version History

1.0 Initial version of Nabbit, as originally released on
    http://people.csail.mit.edu/sukhaj/nabbit/index.html
    This version was originally written for Cilk++.

1.1 Port of key components of version 1.0 to Cilk Plus.

    Major changes:
    - Converted code from Cilk++ to Cilk Plus.
    - Renamed  DAGNode class to NabbitNode.
    - Renamed dag_node.h to nabbit_node.h.
    - Including "nabbit.h" now includes all the relevant header files.
    - Added a "nabbit_sysdep.h" file, which is intended to encapsulate
      architecture/OS-dependent functions.
    - Added common "mk" directory for compiling on different systems.
    - Removed random_dag benchmark.

1.11 Minor tweaks to build with newer Intel compilers.

    - Adding missing #include of cilk_api.h and -lpthread flag.
    - Flagged some classes that require a major revision
      or cleaning (e.g., eliminate or simplify dynamic_array for static Nabbit).

1.12 Minor tweaks to build with Clang/LLVM branch of Cilk Plus.

    - Creating a "dev" branch for active development.
    - Extending Makefile to look for icc, then gcc, then clang.
    - Adding flags to use C++11 for compilation.
    - Other minor edits to eliminate compiler warnings with clang++.
    - Minor edits to eliminate compiler warnings with GCC 5.4
      
1.2 Change build to use CMake
    