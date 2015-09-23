include ../mk/Makefile

# The names of the tests to run.
TEST_NAMES = dynamic_array concurrent_linked_list concurrent_hash_table
OTHER_TESTS = malloc_test ser_malloc_test

# The extra include files
UTILS = example_util_gettime.h check_sort.h
UTIL_FILES = $(addprefix $(UTIL_DIR)/,$(UTILS))

TEST_TARGETS = $(addprefix test_, $(TEST_NAMES)) 
TARGETS = $(TEST_TARGETS)  $(OTHER_TESTS) 

VALGRIND_TARGETS =  $(addprefix ser_test_, $(TEST_NAMES))

.PHONY: all clean check

default: $(TARGETS)

all: $(TARGETS) $(VALGRIND_TARGETS)

check: $(TARGETS)
	./test_dynamic_array
	./test_concurrent_linked_list
	./test_concurrent_hash_table
	./ser_malloc_test
	./malloc_test


# Pattern rule for building tests cases.
test_%: %_test.cpp $(UTIL_FILES) $(NABBIT_DIR)/%.h
	$(CXX) $< $(NABBIT_INCLUDES) $(CFLAGS) -o $@

malloc_test: malloc_test.cpp $(UTIL_FILES)
	$(CXX) $< $(NABBIT_INCLUDES) $(CFLAGS) -o $@

# Serial elision of test cases.
ser_test_%: %_test.cpp $(UTIL_FILES) $(NABBIT_DIR)/%.h
	$(CXX) $< $(NABBIT_INCLUDES) $(CFLAGS) $(CILK_SERIALIZE_FLAG) -o $@

ser_malloc_test: malloc_test.cpp $(UTIL_FILES)
	$(CXX) $< $(NABBIT_INCLUDES) $(CFLAGS) $(CILK_SERIALIZE_FLAG) -o $@

clean:
	rm -f $(TARGET) $(TARGETS) $(VALGRIND_TARGETS) *.exe *.obj
