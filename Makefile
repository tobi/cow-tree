CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = dist/cow-tree
SRC = cow-tree.c

$(TARGET): $(SRC)
	mkdir -p dist
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: clean`