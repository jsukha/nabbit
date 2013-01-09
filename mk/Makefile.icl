# Makefile for compilation with icl (Windows)

# Include flags for Nabbit.
NABBIT_INCLUDES = /I $(UTIL_DIR) /I $(NABBIT_DIR) /I $(ARRAYS_DIR)

CC=icl
CXX=icl
CILKVIEW_FLAGS=
LIBARG= 

CFLAGS = /Wall /O3
CXXFLAGS = $(CFLAGS)
CILK_SERIALIZE_FLAG = /Qcilk-serialize
