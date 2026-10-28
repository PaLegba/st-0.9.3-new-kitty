CC ?= cc
CFLAGS ?= -O2
CFLAGS += -std=c99 -pedantic -Wall -Wextra
LDLIBS = -lm

BIN = kitty_pet
SRC = kitty_pet.c

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

assets:
	./xpm2png.py pets pets-png

clean:
	rm -f $(BIN)

.PHONY: all assets clean
