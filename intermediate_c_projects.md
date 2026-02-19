# C Mastery Roadmap — From Backend to Fullstack

A structured learning path for mastering real-world C programming. Build progressively complex systems, starting from a web server backend and working toward a complete fullstack toolkit.

> **Your current progress:** Phase 1 (HTTP Web Server) ✅ complete

---

## Roadmap Overview

```
Phase 1: Backend Foundation ✅
  └─ HTTP Web Server (thread-per-connection, logging, routing)

Phase 2: Data Layer
  ├─ 2A. JSON Parser (wire format for APIs)
  ├─ 2B. Key-Value Store (in-memory + persistence)
  └─ 2C. SQL Database Engine (B-tree, pager, SQL subset)

Phase 3: Networking & Real-Time
  ├─ 3A. TCP Chat Server (epoll, non-blocking I/O, protocol design)
  ├─ 3B. DNS Resolver (UDP, binary protocols)
  └─ 3C. HTTP/1.1 Client Library (connect your server to external APIs)

Phase 4: Systems Infrastructure
  ├─ 4A. Thread Pool Library (reusable concurrency primitive)
  ├─ 4B. Memory Allocator (malloc/free from scratch)
  └─ 4C. Build System (dependency graphs, process execution)

Phase 5: Developer Tools
  ├─ 5A. Unix Shell (process management, pipes, job control)
  ├─ 5B. Terminal Text Editor (raw mode, piece table)
  └─ 5C. Version Control System (content-addressable storage, diffs)

Phase 6: Deep Systems
  ├─ 6A. Regular Expression Engine (NFAs, DFAs)
  ├─ 6B. Huffman File Compressor (bit manipulation, trees)
  ├─ 6C. Bytecode VM (stack machine, instruction set)
  └─ 6D. ELF Binary Parser (learning how your compiled programs work)
```

---

## Phase 1: Backend Foundation ✅ DONE

### Project: HTTP Web Server

**Status:** ✅ Complete — built with thread-per-connection model, modular common library (connection, HTTP, request/response, router, utils), and logging system.

**What you learned:**
- Socket programming (TCP, `bind`, `listen`, `accept`)
- HTTP protocol parsing (request lines, headers, body)
- Multi-threading with `pthreads`
- Modular C project structure (separate `include/` and `src/`)
- Thread-safe logging with mutexes

**Your codebase:** `web_server/`

---

## Phase 2: Data Layer

> **Goal:** Build the storage and serialization layer that every real backend needs. JSON for API communication, a key-value store for fast data access, and a SQL engine for structured queries.

### 2A. JSON Parser

**Why this matters:** Every web API speaks JSON. Building a parser teaches you recursive descent parsing, memory management for tree structures, and UTF-8 string handling — skills you'll reuse in the SQL parser and beyond.

**What you'll build:**
- Lexer (tokenizer) for JSON input
- Recursive descent parser producing an in-memory tree
- Serializer (tree → JSON string)
- DOM-style API (getters/setters/iterators)

**Key concepts:**
- Recursive descent parsing
- AST (Abstract Syntax Tree) construction
- Unicode/UTF-8 handling
- Memory arena or tree cleanup patterns

**Core data structures:**
```c
typedef enum {
    JSON_NULL, JSON_BOOL, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT
} json_type_t;

typedef struct json_value {
    json_type_t type;
    union {
        int bool_val;
        double number_val;
        char *string_val;
        struct { struct json_value **items; size_t count; } array;
        struct { char **keys; struct json_value **values; size_t count; } object;
    } data;
} json_value_t;
```

**Stages:**
1. Lexer — tokenize JSON input
2. Parser — recursive descent for scalars (null, bool, number, string)
3. Objects and arrays (recursive nesting)
4. Escape sequences and Unicode (`\uXXXX`)
5. Error reporting with line/column numbers
6. Serializer (pretty-print and minified)
7. Access API: `json_get_string(obj, "key")`, `json_array_at(arr, i)`

**Integration point:** Use this JSON parser in your web server to parse request bodies and build JSON responses.

---

### 2B. Key-Value Store

**Why this matters:** This is the simplest useful database — Redis, memcached, and etcd are all key-value stores at heart. You'll learn hash tables, persistence strategies, and the client-server model for databases.

**What you'll build:**
- In-memory hash table with collision resolution
- TCP server accepting `GET`, `SET`, `DEL` commands
- Persistence: append-only file (AOF) and/or snapshotting
- TTL (time-to-live) expiry for keys

**Key concepts:**
- Hash tables (open addressing or separate chaining)
- Serialization/deserialization for disk persistence
- Wire protocol design (text-based like Redis RESP)
- Event loop for concurrent clients (reuse your server code)

**Core data structures:**
```c
typedef struct kv_entry {
    char *key;
    char *value;
    size_t value_len;
    time_t expires_at;        // 0 = no expiry
    struct kv_entry *next;    // Chaining
} kv_entry_t;

typedef struct {
    kv_entry_t **buckets;
    size_t num_buckets;
    size_t count;
} kv_store_t;
```

**Stages:**
1. In-memory hash table with `get`/`set`/`del`
2. TCP server with text protocol (`SET key value\r\n`, `GET key\r\n`)
3. Handle multiple clients (reuse your thread-per-connection or add epoll)
4. Append-only log for persistence
5. Startup recovery from log file
6. TTL support and lazy expiry
7. Snapshotting (fork + serialize entire store)

**Integration point:** Use as a session store or cache for your web server.

---

### 2C. SQL Database Engine (SQLite Clone)

**Why this matters:** This is where you understand how PostgreSQL, MySQL, and SQLite actually work. B-trees, page-level storage, SQL parsing, and query execution — the deepest real-world systems knowledge you can build.

**What you'll build:**
- B-tree storage engine with disk-backed pages
- SQL parser for a subset (CREATE TABLE, INSERT, SELECT, WHERE)
- Pager/buffer pool for caching disk pages
- REPL for interactive queries

**Key concepts:**
- B-tree data structure (insert, search, split, delete)
- Page-based storage (4KB pages, page headers, cell pointers)
- SQL tokenization and parsing
- Virtual machine for query execution (like SQLite's approach)

**Architecture:**
```
SQL Input → Tokenizer → Parser → Code Generator → VM → B-Tree → Pager → Disk
```

**Stages:**
1. In-memory table (array of structs), no persistence
2. Simple file serialization (append-only log)
3. B-tree implementation for storage
4. Pager: read/write fixed-size pages from disk
5. SQL tokenizer and recursive descent parser
6. Query execution (table scans, WHERE filtering)
7. Indexing (secondary B-tree indexes)

---

## Phase 3: Networking & Real-Time

> **Goal:** Move beyond request-response HTTP. Build real-time systems, understand binary protocols, and learn to make outbound connections (turning your server into a client too).

### 3A. TCP Chat Server

**Why this matters:** Real-world backends need to handle thousands of persistent connections simultaneously. This project teaches you `epoll`, non-blocking I/O, and custom protocol design — the foundation of every chat, game, and streaming server.

**What you'll build:**
- Event-driven server using `epoll` (single-thread, many connections)
- Binary message protocol with framing
- Chat rooms, private messaging, user management
- Keepalive (ping/pong) and graceful disconnect

**Key concepts:**
- Non-blocking sockets (`O_NONBLOCK`, `EAGAIN`)
- I/O multiplexing (`epoll_create`, `epoll_wait`, `epoll_ctl`)
- Message framing (length-prefixed binary protocol)
- State machines for protocol parsing

**Stages:**
1. Blocking sockets, single client echo server
2. `select()` with multiple clients
3. Non-blocking sockets with `epoll`
4. Message framing protocol (magic + type + length + payload + checksum)
5. Username/login system
6. Chat rooms and broadcasting
7. Private messaging
8. Write buffering for partial sends

---

### 3B. DNS Resolver

**Why this matters:** DNS is the backbone of the internet. Building a resolver teaches you UDP sockets, binary protocol parsing, network byte order, and caching — skills critical for any network programming.

**What you'll build:**
- UDP socket client that queries DNS servers
- DNS message builder and parser (binary protocol)
- Support for A, AAAA, CNAME, MX record types
- Local cache with TTL-based expiry

**Key concepts:**
- UDP sockets (connectionless I/O)
- Binary protocol encoding/decoding (network byte order)
- DNS name compression (pointer-based encoding)
- Cache invalidation (TTL)

**Stages:**
1. UDP socket creation and raw send/receive
2. DNS message structure and byte order handling
3. Build A record queries
4. Parse DNS responses
5. Handle name compression pointers
6. Multiple record types (AAAA, CNAME, MX)
7. Caching with TTL
8. Recursive resolution (follow CNAME/NS chains)

---

### 3C. HTTP Client Library

**Why this matters:** Your web server handles *incoming* requests. A real backend also needs to call *external* APIs (payment gateways, third-party services). This is your `libcurl` equivalent.

**What you'll build:**
- TCP connection to remote HTTP servers
- Request building (method, headers, body)
- Response parsing (status, headers, body, chunked encoding)
- Connection pooling and keep-alive
- Simple API: `http_get(url)`, `http_post(url, body)`

**Key concepts:**
- DNS resolution (use your resolver from 3B!)
- TCP client sockets (`connect()`)
- HTTP/1.1 protocol from the client side
- Chunked transfer encoding
- TLS/SSL basics (optional, using OpenSSL)

**Stages:**
1. TCP client that connects and sends raw bytes
2. URL parsing (scheme, host, port, path)
3. Build HTTP requests with headers
4. Parse HTTP responses (status line, headers, body)
5. Handle chunked transfer encoding
6. Connection reuse (keep-alive)
7. Redirect following (301, 302)
8. Optional: TLS support via OpenSSL

**Integration point:** Use in your web server to fetch data from external APIs and return it to clients. Combined with your JSON parser, you have a fullstack C pipeline.

---

## Phase 4: Systems Infrastructure

> **Goal:** Build the reusable libraries that production systems depend on. These become building blocks you can plug into any future project.

### 4A. Thread Pool Library

**Why this matters:** Thread-per-connection doesn't scale. Every production server uses a thread pool. Build one as a reusable library and retrofit it into your web server.

**What you'll build:**
- Fixed-size pool of worker threads
- Thread-safe work queue (mutex + condition variables)
- Blocking and non-blocking task submission
- Graceful shutdown (finish in-flight tasks)

**Key concepts:**
- Condition variables (`pthread_cond_wait`, `pthread_cond_signal`)
- Producer-consumer pattern
- Lock-free techniques (optional)
- Work stealing (advanced)

**Stages:**
1. Basic thread pool with fixed workers
2. Thread-safe work queue with mutex
3. Condition variables for efficient waiting
4. Graceful shutdown mechanism
5. Dynamic pool sizing (grow/shrink on load)
6. Task priorities (multiple queues)
7. Retrofit into your web server as the concurrency model

---

### 4B. Memory Allocator

**Why this matters:** Understanding `malloc`/`free` at the implementation level transforms how you write C. You'll know why fragmentation happens, what alignment means, and how memory actually works.

**What you'll build:**
- Custom `malloc`, `free`, `realloc`, `calloc` using `sbrk()`/`mmap()`
- Free list management with splitting and coalescing
- Multiple allocation strategies (first-fit, best-fit)
- Thread-safe version with per-thread arenas

**Key concepts:**
- Heap management (`sbrk`, `mmap`, `munmap`)
- Block metadata and alignment requirements
- Fragmentation (internal vs external)
- Coalescing adjacent free blocks

**Stages:**
1. Basic malloc/free with `sbrk()`, no reuse
2. Free list, reuse freed blocks (first-fit)
3. Block splitting and coalescing
4. Best-fit strategy (compare performance)
5. Thread-safe version (mutex per arena)
6. Large allocation path via `mmap()`
7. Debugging tools (guard bands, canary values, leak tracking)

---

### 4C. Build System (Make Clone)

**Why this matters:** You've been using Make — now build it. Teaches dependency graphs, topological sorts, process execution, and file timestamps. Also teaches you how real build systems decide what to rebuild.

**What you'll build:**
- Makefile parser (variables, rules, commands)
- Dependency graph construction
- Topological sort with cycle detection
- Incremental builds via timestamp comparison
- Parallel job execution

**Stages:**
1. Parse variable assignments
2. Parse rules with targets and dependencies
3. Build dependency graph
4. Topological sort with cycle detection
5. Timestamp checking (rebuild only what changed)
6. Execute shell commands (`fork` + `exec`)
7. Variable substitution (`$@`, `$<`, `$^`)
8. Pattern rules (`%.o: %.c`)
9. Parallel job execution (`-j N`)

---

## Phase 5: Developer Tools

> **Goal:** Build the tools you use every day as a developer. These projects combine everything from earlier phases.

### 5A. Unix Shell

**Why this matters:** The shell is how you interact with everything you've built. Understanding `fork`, `exec`, `pipe`, and signals at the deepest level makes you a true systems programmer.

**What you'll build:**
- REPL with command parsing
- Process execution via `fork()` + `execve()`
- Pipes (`cmd1 | cmd2 | cmd3`)
- I/O redirection (`<`, `>`, `>>`)
- Job control (`&`, `fg`, `bg`, `Ctrl+C`, `Ctrl+Z`)
- Builtins (`cd`, `exit`, `export`, `history`)

**Stages:**
1. Execute single commands (`fork` + `exec`)
2. Arguments and `$PATH` resolution
3. Pipes (`|`)
4. Redirections (`<`, `>`, `>>`, `2>`)
5. Job control (`&`, `fg`, `bg`, `jobs`)
6. Signal handling (`SIGINT`, `SIGTSTP`, `SIGCHLD`)
7. Builtins (`cd`, `exit`, `export`)

---

### 5B. Terminal Text Editor

**Why this matters:** Combines raw terminal control, efficient data structures, and modal UI into one project. The piece table data structure is used in VS Code.

**What you'll build:**
- Raw terminal mode (disable canonical processing)
- Piece table for efficient text editing
- Modal editing (normal/insert/command modes)
- Syntax highlighting (optional)
- File save/load, search, undo/redo

**Stages:**
1. Raw terminal mode, read single keypresses
2. Display file content, cursor movement
3. Piece table data structure for text storage
4. Insert/delete characters
5. Line navigation, scrolling viewport
6. Modal editing (normal + insert modes)
7. Commands (`:w`, `:q`, search)

---

### 5C. Version Control System (Git-lite)

**Why this matters:** Git is the most important developer tool. Building a simplified version teaches content-addressable storage, DAGs, diff algorithms, and how snapshots actually work.

**What you'll build:**
- Content-addressable object store (SHA-1 → blob/tree/commit)
- Index/staging area
- `init`, `add`, `commit`, `log`, `diff` commands
- Branching and basic checkout

**Stages:**
1. Blob storage (`hash-object`, `cat-file`)
2. Tree objects (directory representation)
3. Commit objects with parent chain
4. Index/staging area
5. `add` and `commit` commands
6. `log` command (traverse history DAG)
7. `diff` command (Myers' algorithm)
8. Branching (multiple refs)
9. Basic `checkout`

---

## Phase 6: Deep Systems

> **Goal:** These projects go deeper into CS fundamentals. Tackle them when you want to understand how compilers, compression, and binary formats really work.

### 6A. Regular Expression Engine
Build NFAs and DFAs from regex patterns. Teaches finite automata, Thompson's construction, subset construction, and epsilon closures.

### 6B. Huffman File Compressor
Lossless compression using Huffman coding. Teaches min-heaps, binary trees, bit manipulation, and binary file formats.

### 6C. Bytecode Virtual Machine
Stack-based VM executing bytecode instructions. Teaches instruction set design, call stacks, constant pools, and eventually a compiler for a simple language.

### 6D. ELF Binary Parser
Parse Linux ELF binaries to extract headers, sections, symbols. Teaches `mmap`, binary formats, endianness, and how your compiled C programs are actually structured.

---

## Real-World Integration Map

Once Phases 2 and 3 are done, you'll have a complete C fullstack:

```
                    ┌──────────────────────────────────┐
                    │         YOUR C FULLSTACK          │
                    └──────────────────────────────────┘

  Client Request (curl/browser)
        │
        ▼
  ┌─────────────┐    Parse body    ┌─────────────┐
  │  Web Server  │ ──────────────▶ │ JSON Parser  │
  │  (Phase 1)   │ ◀────────────── │ (Phase 2A)   │
  └─────────────┘    Build resp    └─────────────┘
        │
        ▼  query / cache
  ┌─────────────┐              ┌─────────────┐
  │  SQL Engine  │              │   KV Store   │
  │  (Phase 2C)  │              │  (Phase 2B)  │
  └─────────────┘              └─────────────┘
        │
        ▼  call external API
  ┌─────────────┐    resolve     ┌─────────────┐
  │ HTTP Client  │ ────────────▶ │DNS Resolver  │
  │  (Phase 3C)  │               │  (Phase 3B)  │
  └─────────────┘               └─────────────┘
        │
        ▼  powered by
  ┌─────────────┐
  │ Thread Pool  │
  │  (Phase 4A)  │
  └─────────────┘
```

---

## General Tips

### Project Structure
```
project_name/
├── include/       # Public headers
├── src/           # Implementation
├── tests/         # Unit and integration tests
├── Makefile
└── README.md
```

### Essential Debugging Tools
| Tool | Purpose |
|------|---------|
| `gdb` | Step-through debugging, breakpoints, memory inspection |
| `valgrind` | Memory leak detection, invalid access tracking |
| `strace` | Trace system calls your program makes |
| `-fsanitize=address` | Compile-time memory error detection |
| `perf` | Performance profiling and hotspot analysis |

### Common Pitfalls
- Always `free` what you `malloc` — use valgrind to verify
- Use `snprintf` not `sprintf` — prevent buffer overflows
- Check every `malloc`/`fread`/`stat` return — don't ignore failures
- Convert network byte order with `htons`/`ntohs`/`htonl`/`ntohl`
- Never use uninitialized variables — compile with `-Wall -Wextra`

### Recommended Reading
| Book | What It Teaches |
|------|-----------------|
| K&R "The C Programming Language" | C fundamentals |
| "Computer Systems: A Programmer's Perspective" | How hardware + software interact |
| "The Linux Programming Interface" | System calls and POSIX |
| Beej's Guide to Network Programming | Free — sockets and networking |
| "Database Internals" by Alex Petrov | How databases actually work |

---

*Build in order. Each phase uses skills from the previous one. By Phase 3, you'll have a complete fullstack toolkit — entirely in C.*
