NABBIT_ROOT=..
include ${NABBIT_ROOT}/mk/Makefile

TARGETS = example_static
SRC	= $(addsuffix .cpp,$(TARGET))


H_FILES = sample_nabbit_node.h

.PHONY: clean check

all: $(TARGETS)

check: $(TARGETS)
	./example_static

example_%: example_%.cpp $(H_FILES)
	$(CXX) $(CXXFLAGS) $< $(NABBIT_INCLUDES) $(LIBARG) -o $@

clean:
	rm -f $(TARGETS) *.obj
