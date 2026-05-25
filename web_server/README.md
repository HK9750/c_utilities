# C Web Server Examples

This directory contains three small HTTP server implementations that share the same request parser, response builder, router, connection helpers, and demo routes.

## Build

Build every server type from this directory:

```sh
make
```

Build one server type directly:

```sh
make -C servers/thread_per_connection
make -C servers/thread_pool
make -C servers/event_loop
```

Each server binary is written as `server` inside its own directory.

## Run

Run one implementation from its directory:

```sh
cd servers/thread_per_connection
./server
```

All implementations listen on port `8080`.

## Routes

The root route serves `www/index.html`:

```sh
curl http://localhost:8080/
```

The demo `things` routes are registered in `common/src/demo_routes.c` and are available in every server implementation:

```sh
curl http://localhost:8080/things
curl -X POST http://localhost:8080/things
curl -X PUT http://localhost:8080/things
curl -X PATCH http://localhost:8080/things
curl -X DELETE http://localhost:8080/things
```

## Server Types

| Type | Location | Architecture | Advantages | Disadvantages |
| --- | --- | --- | --- | --- |
| Thread per connection | `servers/thread_per_connection` | The accept loop creates a new detached thread for each accepted client connection. Each thread reads, routes, responds, closes the socket, and exits. | Simple control flow, easy to understand, blocking I/O is straightforward, each request is isolated in its own thread. | High thread creation cost, poor scalability with many connections, more memory per client, can exhaust OS thread limits under load. |
| Thread pool | `servers/thread_pool` | The accept loop stays on one thread and submits each accepted connection to a fixed worker pool. Workers process blocking reads/writes. | Reuses threads, limits maximum concurrency, avoids unbounded thread creation, usually better than thread-per-connection for steady traffic. | Still uses blocking I/O per worker, slow clients can occupy workers, pool sizing matters, more synchronization complexity. |
| Event loop | `servers/event_loop` | One non-blocking event loop uses `epoll` to watch sockets for read/write readiness and processes clients without creating worker threads per connection. | Handles many idle connections with low memory overhead, avoids thread contention, efficient for I/O-heavy workloads. | More complex state management, CPU-heavy handlers block the loop, partial reads/writes are harder, debugging can be less direct. |

## Shared Architecture

All server types use these common pieces:

| Component | Purpose |
| --- | --- |
| `common/src/request.c` | Parses the HTTP method, path, version, headers, and body from raw request bytes. |
| `common/src/response.c` | Builds HTTP responses from status, headers, and body data. |
| `common/src/router.c` | Stores method/path handlers and dispatches matching requests. |
| `common/src/demo_routes.c` | Registers sample `GET`, `POST`, `PUT`, `PATCH`, and `DELETE` routes for `/things`. |
| `common/src/connection.c` | Wraps socket reads, writes, and closes for blocking server implementations. |
