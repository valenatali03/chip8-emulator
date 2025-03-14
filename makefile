CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lSDL2
SRC = main.c
OUT = main

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
