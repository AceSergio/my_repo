CC = gcc
CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic -D_POSIX_C_SOURCE=200809L
INCLUDES = -I../src

SRC_STRING = ../src/utils/string/string.c
SRC_HTTP = ../src/http/request.c

BIN_TEST_STRING = test_string_bin
BIN_TEST_HTTP = test_http_bin

.PHONY: all clean check check_unit check_integration check_daemon

all: $(BIN_TEST_STRING) $(BIN_TEST_HTTP)

$(BIN_TEST_STRING): test_string/test_string.c $(SRC_STRING)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

$(BIN_TEST_HTTP): test_http/test_http.c $(SRC_HTTP)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

check: check_unit check_integration

check_unit: all
	@echo "\n>>> Running Unit Tests <<<"
	./$(BIN_TEST_STRING)
	./$(BIN_TEST_HTTP)

check_integration:
	@echo "\n>>> Running Integration Tests (Python) <<<"
	python3 test_integration.py

check_daemon:
	@echo "\n>>> Running Daemon Tests (Python) <<<"
	python3 test_daemon.py

clean:
	rm -f $(BIN_TEST_STRING) $(BIN_TEST_HTTP) *.conf *.log