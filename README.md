# udpserver

Simple UDP server and client written in C for testing basic datagram communication.

## Building

Use the included `Makefile` to build both binaries:

```bash
make
```

This produces `udp_server` and `udp_client`. Clean build outputs with:

```bash
make clean
```

## Usage

In one terminal, start the server on a chosen port (e.g., 9000):

```bash
./udp_server 9000
```

In another terminal, run the client, passing the server IP, port, and the message to send:

```bash
./udp_client 127.0.0.1 9000 "Hello from client"
```

The server prints each received datagram and replies with `"Message received"`, which the client displays upon receipt. Stop the server with `Ctrl+C`.
