CC = gcc
CFLAGS = -Wall -Wextra
TEST_FLAGS = -Wall -Wextra -g

all: app

app: main.c
	$(CC) $(CFLAGS) -o app main.c

test: test_suite.c
	$(CC) $(TEST_FLAGS) -o test_suite test_suite.c
	./test_suite

clean:
	rm -f app test_suite
	rm -f *.o
	rm -f .shell_history

.PHONY: all test clean 