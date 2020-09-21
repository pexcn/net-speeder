CC ?= gcc
DESTDIR ?= /usr/local
SRC ?= src/netspeeder.c
BIN ?= netspeeder

all: $(SRC)
	$(CC) -O3 -lpcap -lnet -o $(BIN) $(SRC) -s -Wl,--build-id=none

install: $(BIN)
	install -d $(DESTDIR)/bin/
	install $(BIN) $(DESTDIR)/bin/

uninstall:
	rm -f $(DESTDIR)/bin/$(BIN)

clean:
	rm -f $(BIN)
