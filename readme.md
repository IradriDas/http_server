# Simple HTTP Server in C

A minimal HTTP/1.1 static file server built from scratch in C using raw
BSD sockets — no frameworks, no libraries beyond the C standard library
and POSIX networking APIs. Built as a learning exercise to understand
what actually happens under the hood when a server "listens" for
requests, following [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/).

## What it does

- Listens for TCP connections on port `8080`
- Accepts incoming client connections
- Parses the HTTP request line to extract the method and path
- Maps the requested path to a file inside the `view/` directory
- Reads the file from disk and serves it back with the correct
  `Content-Type` header (HTML, CSS, PNG, JPEG, or a generic binary
  fallback)
- Returns a `404 Not Found` response if the requested file doesn't exist

## Project structure

```
.
├── server.c        # server source code
├── view/           # static files served by the server
│   ├── index.html
│   ├── style.css
│   ├── favicon.png
│   └── profile_pic.jpg
└── README.md
```

## How it works

The server follows the standard BSD sockets flow for a TCP server:

1. **`getaddrinfo()`** — resolves local address info for binding,
   protocol-agnostic (IPv4/IPv6)
2. **`socket()`** — creates the listening socket
3. **`setsockopt()` with `SO_REUSEADDR`** — allows the port to be
   reused immediately after restarting the server
4. **`bind()`** — attaches the socket to port `8080`
5. **`listen()`** — marks the socket as passive, ready to queue
   incoming connections
6. **`accept()`** (in a loop) — blocks until a client connects, then
   returns a new socket dedicated to that client
7. **`recv()`** — reads the raw HTTP request text from the client
8. **`sscanf()`** — extracts the HTTP method and requested path from
   the request line
9. File lookup — maps the path to a file under `view/` and reads it
   into memory
10. **`send()`** — writes the HTTP response (status line, headers,
    blank line, body) back to the client
11. **`close()`** — closes the per-client connection

Each request is handled synchronously, one at a time, in a single
`while (1)` loop.

## Building

Requires `gcc` (or any C compiler) on a POSIX-compliant system
(Linux/macOS).

```bash
gcc server.c -o server
```

## Running

Run the binary from the project root, so the relative path to `view/`
resolves correctly:

```bash
./server
```

You should see:

```
server listening on port 8080...
```

Then visit `http://localhost:8080` in a browser, or test with `curl`:

```bash
curl http://localhost:8080/
```

## Current limitations

This is intentionally a learning project, not a production server.
Known gaps:

- **No real routing** — every path is resolved to a file under
  `view/`; there's no support for dynamic routes or handlers
- **GET only, effectively** — the request method is parsed but not
  checked, so any method serves the same file
- **No request body handling** — `Content-Length` on incoming
  requests (e.g. for POST) isn't read
- **Single-threaded** — one client is handled at a time; a second
  client must wait until the current one is `close()`d
- **No keep-alive** — every connection is closed after a single
  response, even under HTTP/1.1
- **Minimal error handling** — malformed requests or edge cases in
  parsing aren't robustly handled

## Possible next steps

- Handle `POST` requests and read request bodies via `Content-Length`
- Add concurrency (`fork()` per connection, or a thread pool)
- Support persistent connections (`Connection: keep-alive`)
- Add basic routing for dynamic responses, not just static files
- Guard against path traversal (e.g. `GET /../../etc/passwd`)

## Why build this instead of using a framework

Frameworks like Express (Node.js) hide exactly the steps this project
implements manually: socket creation, binding, listening, accepting
connections, parsing raw HTTP text, and formatting responses by hand.
Building it from scratch is a way to understand what `app.listen()`,
`res.send()`, and `res.sendFile()` are actually doing underneath.