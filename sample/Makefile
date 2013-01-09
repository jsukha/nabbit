
# Defines $(CC) and $(NABBIT_INCLUDES)
include ../mk/Makefile

TARGET = sample
SRC	= $(addsuffix .cpp,$(TARGET))


H_FILES = sample_nabbit_node.h

.PHONY: clean

$(TARGET): $(SRC) $(H_FILES) $(O_FILES)
	$(CC) $(CFLAGS) $(SRC) $(NABBIT_INCLUDES) $(LIBARG) -o $@

clean:
	rm -f $(TARGET) *.obj
