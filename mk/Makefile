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
HAVE_ICC := $(shell command -v icc 2> /dev/null)
ifdef HAVE_ICC
include $(NABBIT_ROOT)/mk/Makefile.icc
else
include $(NABBIT_ROOT)/mk/Makefile.gcc
endif
endif
