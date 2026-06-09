CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = quoridor_pacman
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)

run: all
	./$(TARGET)
