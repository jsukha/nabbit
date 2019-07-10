
# Defines $(CC) and $(NABBIT_INCLUDES)
include ../mk/Makefile

TARGETS = example_static
SRC	= $(addsuffix .cpp,$(TARGET))


H_FILES = sample_nabbit_node.h

.PHONY: clean check

all: $(TARGETS)

check: $(TARGETS)
	./example_static

example_%: example_%.cpp $(H_FILES)
	$(CC) $(CFLAGS) $< $(NABBIT_INCLUDES) $(LIBARG) -o $@

clean:
	rm -f $(TARGETS) *.obj
