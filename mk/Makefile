NABBIT_ROOT=..

# The directory where Nabbit's .h files are located.
NABBIT_DIR=$(NABBIT_ROOT)/include

# Directory for array indexing routines.
ARRAYS_DIR=$(NABBIT_ROOT)/arrays

# The directory for other common utilities
UTIL_DIR=$(NABBIT_ROOT)/util

# Choose a platform-dependent Makefile
ifeq ($(OS),Windows_NT)
include $(NABBIT_ROOT)/mk/Makefile.icl
else
include $(NABBIT_ROOT)/mk/Makefile.icc
endif
