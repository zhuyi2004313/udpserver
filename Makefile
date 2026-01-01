CC ?= gcc
CFLAGS ?= -Wall -Wextra -pedantic -std=c11 -O2
LDFLAGS ?=

BINARIES := udp_server udp_client

all: $(BINARIES)

udp_server: server.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

udp_client: client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(BINARIES)

.PHONY: all clean
