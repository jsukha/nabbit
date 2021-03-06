Array Indexing Library
Version 1.2
---------------------------------------------

This folder contains code for indexing two-dimensional arrays in
Morton-order and blocked layouts.

For a description of the various layouts, see `array_layouts.h`.



Indexing for blocked arrays:

`nabbit_array_defs.h`:
	Definitions of some math functions and global types.

`morton.h`:
	Methods for indexing Morton-order layouts.  A Morton-order
	index of a point (x, y) has the bits of the row-index x and
	the column-index y interleaved.

`array_layouts.h`:
	Indexing for all simple layouts -- row-major, column-major,
	and Morton-order layouts.

`block_layouts.h`:
	Indexing for two-level blocked layouts.  The arrangement of
	elements within a block, as well as the arrangement of blocks,
	can be one of the simple layouts described in array_layouts.h

`convert.h`:
	Some simple routines for allocating, converting, and comparing
	arrays in different layouts.



Simple array code (an older version of the interface):
	
`array2d_morton.h`:
        Creates a simple (non-blocked) array arranged in Morton-order layout.
	
`array2d_row.h`:
	Creates a simple (non-blocked) array arranged in a rowmajor layout. 

`array2d_base.h`:
	Base class that `array2d_morton.h` and `array2d_row.h` derive from. 

These simple arrays are convenient if one isn't trying to slice the
array (e.g., operate on subarrays).  For more complicated applications
though (e.g., matrix multiplication), it makes more sense to allocate
the array manually, and use the indexing code.


Version History:

 - 1.0  Initial version
 - 1.2  Package together with Nabbit
        Test cases are compiled and run as part of the larger Nabbit build.
