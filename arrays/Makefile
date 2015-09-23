include ../mk/Makefile

TARGETS = test_convert test_arrays


H_FILES = nabbit_array_defs.h morton.h array_layouts.h block_layouts.h	\
convert.h array2d_base.h array2d_row.h array2d_morton.h
TEST_FILES = $(addsuffix .cpp, $(TARGETS))


.PHONY: clean run_tests tarball


all: $(TARGETS)

test_%:  test_%.cpp $(H_FILES)
	$(CXX) $< $(INCLUDES) $(CFLAGS) -o $@

check: $(TARGETS)
	./test_convert
	./test_arrays

clean:
	rm -rf $(TARGETS) $(PACKAGE_DIR) nabbit_arrays.tar.gz *.obj



## Code for creating the package file.

PACKAGE_DIR = nabbit_arrays
PACKAGE_FILES = $(H_FILES) $(TEST_FILES) README Makefile
PACKAGE_NAME = nabbit_arrays

tarball: nabbit_arrays.tar.gz

nabbit_arrays.tar.gz: $(H_FILES) $(TEST_FILES)
	mkdir -p $(PACKAGE_DIR)
	cp $(PACKAGE_FILES) $(PACKAGE_DIR)
	tar -cvf nabbit_arrays.tar $(PACKAGE_DIR)
	gzip nabbit_arrays.tar
	rm -rf $(PACKAGE_DIR)
