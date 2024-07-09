CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = cow-tree
SRC = cow-tree.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: clean`