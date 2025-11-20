CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
TARGET = program

SOURCES = main.c test_config.c
OBJECTS = main.o test_config.o string.o
CHEMIN_STRING = ../../src/utils/string

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -c $(CHEMIN_STRING)/string.c -o string.o

test: all
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all test clean
