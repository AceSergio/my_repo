# --- Variables ---

CC = gcc

CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic -D_POSIX_C_SOURCE=200809L

INCLUDES = -Isrc -Isrc/config -Isrc/server -Isrc/daemon -Isrc/http -Isrc/logger

LDFLAGS = 

TARGET = httpd

SRC_DIR_MAIN = src
SRC_DIR_CONFIG = src/config
SRC_DIR_STRING = src/utils/string
SRC_DIR_SERVER = src/server
SRC_DIR_DAEMON = src/daemon
SRC_DIR_HTTP = src/http
SRC_DIR_LOGGER = src/logger

SRCS = $(SRC_DIR_MAIN)/main.c \
       $(SRC_DIR_CONFIG)/config.c \
       $(SRC_DIR_STRING)/string.c \
       $(SRC_DIR_SERVER)/server.c \
	   $(SRC_DIR_DAEMON)/daemon.c \
       $(SRC_DIR_HTTP)/request.c \
       $(SRC_DIR_HTTP)/response.c \
       $(SRC_DIR_LOGGER)/logger.c


OBJS = $(patsubst %.c, %.o, $(SRCS))

# --- Cibles ---

.PHONY: all clean check config_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

config_test: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) config_test *.o
	@echo "Clean completed."

check: config_test
	@echo "Testing configuration..."
	./tests/config_reader.sh --path-bin ./config_test --path-config config.txt --daemon start
