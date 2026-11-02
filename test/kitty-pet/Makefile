CC ?= cc
CFLAGS ?= -O2
PETCFLAGS = -std=c99 -pedantic -Wall -Wextra
LDLIBS = -lm

BIN = kitty_pet
SRC = kitty_pet.c

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PETCFLAGS) -o $@ $(SRC) $(LDFLAGS) $(LDLIBS)

assets:
	./xpm2png.py pets pets-png

clean:
	rm -f $(BIN)

.PHONY: all assets clean
