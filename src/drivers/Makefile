CC = gcc
CFLAGS = -Wall -Wextra -g -fPIC -I../../include
LDFLAGS = -ldl

# Source files
CHAR_DRIVER_SRC = char_driver.c
TEST_DRIVER_SRC = test_char_driver.c
DRIVER_CORE_SRC = driver_core.c

# Object files
CHAR_DRIVER_OBJ = $(CHAR_DRIVER_SRC:.c=.o)
TEST_DRIVER_OBJ = $(TEST_DRIVER_SRC:.c=.o)
DRIVER_CORE_OBJ = $(DRIVER_CORE_SRC:.c=.o)

# Targets
all: libchar_driver.so test_char_driver

libchar_driver.so: $(CHAR_DRIVER_OBJ) $(DRIVER_CORE_OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

test_char_driver: $(TEST_DRIVER_OBJ) $(DRIVER_CORE_OBJ)
	$(CC) -o $@ $^ -L. -lchar_driver

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so test_char_driver

.PHONY: all clean 