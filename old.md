# Intermediate C Programming Projects

A curated list of 15 intermediate-level C projects designed to build real-world programming skills. Each project includes detailed architecture, concepts used, and implementation flow.

---

## 1. HTTP Web Server

### Overview
Build a minimal HTTP/1.1 compliant web server that can handle multiple concurrent connections, serve static files, and implement basic HTTP methods.

### Core Concepts
- **Socket Programming**: BSD sockets, TCP/IP networking
- **Concurrency**: Multi-threading or async I/O (select/poll/epoll)
- **Protocol Parsing**: HTTP request/response format
- **File I/O**: Efficient file reading and serving
- **Memory Management**: Connection state management

### Architecture

#### Connection Handling
```
Main Loop
    |
    v
Socket Creation (socket())
    |
    v
Bind to Port (bind())
    |
    v
Listen (listen())
    |
    v
Accept Connections (accept()) -----> Client Handler Thread/Process
    |                                      |
    v                                      v
Back to Accept                      Parse HTTP Request
                                    |
                                    v
                                Route Resolution
                                    |
                                    v
                                Generate Response
                                    |
                                    v
                                Send Response
                                    |
                                    v
                                Close/Cleanup
```

#### Request Parsing Flow
1. Read raw bytes from socket into buffer
2. Parse request line: `METHOD PATH HTTP-VERSION`
3. Parse headers (key-value pairs until empty line)
4. Read body if Content-Length present or chunked encoding
5. Validate request format
6. Route to appropriate handler

#### Concurrency Models (Choose one)

**Model A: Thread-Per-Connection**
- Create new thread for each incoming connection
- Simple but resource-intensive
- Good for learning pthreads

**Model B: Thread Pool**
- Pre-allocate fixed pool of worker threads
- Main thread accepts and queues connections
- Workers pick up connections from queue
- Better resource control

**Model C: Event-Driven (select/poll/epoll)**
- Single or few threads handling many connections
- Non-blocking sockets
- Event loop waits for ready sockets
- Most scalable approach

### Implementation Stages
1. **Stage 1**: Single-threaded, handle one connection at a time
2. **Stage 2**: Add fork() for multi-process handling
3. **Stage 3**: Replace with thread pool
4. **Stage 4**: Implement persistent connections (Connection: keep-alive)
5. **Stage 5**: Add support for chunked transfer encoding

### Key Data Structures
```c
// Connection context
typedef struct {
    int socket_fd;
    char buffer[BUFFER_SIZE];
    size_t buffer_used;
    http_state_t state;
    request_t request;
} connection_t;

// HTTP Request
typedef struct {
    char method[16];
    char path[256];
    char version[16];
    header_t *headers;
    size_t body_length;
    char *body;
} request_t;
```

---

## 2. Simple Database Engine (SQLite Clone)

### Overview
Build a minimal SQL database engine with disk-based storage, supporting CREATE TABLE, INSERT, and SELECT statements.

### Core Concepts
- **B-Trees**: Self-balancing tree for indexing
- **Pager/Buffer Pool**: Disk block caching
- **ACID Properties**: Atomicity, Consistency, Isolation, Durability
- **SQL Parsing**: Recursive descent parser
- **Virtual Machine**: Bytecode execution
- **File Formats**: Binary page layout

### Architecture

#### System Layers
```
SQL Interface
      |
      v
SQL Parser (Tokenizer -> Parser)
      |
      v
Code Generator
      |
      v
Virtual Machine (Bytecode Executor)
      |
      v
B-Tree Layer
      |
      v
Pager (Page Cache + Disk I/O)
      |
      v
OS Interface
```

#### Storage Architecture

**Database File Structure:**
```
Page 0: Header + Root B-Tree Node
Page 1-N: Additional B-Tree Nodes
Page N+1: Overflow pages for large records
```

**Page Layout (4KB typical):**
```
+------------------+
| Page Header      |  (page type, free space, cell count)
+------------------+
| Cell Pointer     |  (array of offsets to records)
| Array            |
+------------------+
| Free Space       |
+------------------+
| Cell Content     |  (actual records, grows upward)
| (grows upward)   |
+------------------+
```

#### B-Tree Implementation

**Node Types:**
- **Internal Nodes**: Store keys and pointers to child pages
- **Leaf Nodes**: Store actual row data

**Operations:**
1. **Search**: Traverse from root, compare keys, follow pointers
2. **Insert**: Find leaf, insert key/value, split if full
3. **Delete**: Remove key, redistribute or merge if underflow

#### SQL Parsing Flow
```
Input: "SELECT * FROM users WHERE id = 5"

Tokenizer:
    [SELECT] [*] [FROM] [users] [WHERE] [id] [=] [5]

Parser (Recursive Descent):
    parse_select_stmt()
        -> parse_select_clause()
        -> parse_from_clause()
        -> parse_where_clause()

AST Generation:
    select_stmt_t
        ├── columns: [wildcard]
        ├── table: "users"
        └── where: binary_expr(=, column("id"), literal(5))
```

#### Virtual Machine
Convert AST to bytecode instructions:
```
OP_OPEN_READ  "users"      ; Open table cursor
OP_REWIND                   ; Go to first row
OP_READ_KEY                 ; Read current key
OP_LOAD_IMMEDIATE  5        ; Push 5 onto stack
OP_EQUAL                    ; Compare
OP_JUMP_IF_FALSE  +3        ; Skip if not equal
OP_READ_ROW                 ; Read full row
OP_PRINT                    ; Output result
OP_NEXT                     ; Move to next row
OP_JUMP  -7                 ; Loop back
OP_CLOSE                    ; Clean up
```

### Implementation Stages
1. **Stage 1**: In-memory table (array of structs), no persistence
2. **Stage 2**: Add simple file serialization (append-only log)
3. **Stage 3**: Implement B-Tree for storage
4. **Stage 4**: Add SQL parser (subset)
5. **Stage 5**: Virtual machine for query execution

---

## 3. Unix Shell Implementation

### Overview
Build a POSIX-compliant shell supporting command execution, pipes, redirections, job control, and basic scripting features.

### Core Concepts
- **Process Management**: fork(), exec(), wait()
- **File Descriptors**: stdin(0), stdout(1), stderr(2)
- **Pipes**: Inter-process communication
- **Signal Handling**: SIGINT, SIGTSTP, SIGCHLD
- **Job Control**: Foreground/background processes
- **Parsing**: Tokenization, command substitution

### Architecture

#### Main Loop
```
REPL (Read-Eval-Print Loop)
    |
    v
Print Prompt
    |
    v
Read Input Line
    |
    v
Tokenize Input
    |
    v
Parse Commands (build AST)
    |
    v
Execute Pipeline
    |
    v
Wait for Completion / Background
    |
    v
Loop
```

#### Parsing Pipeline

**Input:** `ls -la | grep .c > output.txt &`

**Tokenization:**
```
[WORD: "ls"] [WORD: "-la"] [PIPE] 
[WORD: "grep"] [WORD: ".c"] [REDIRECT_OUT] [WORD: "output.txt"] 
[BACKGROUND]
```

**AST Construction:**
```
pipeline_t
├── command[0]: "ls" ["-la"]
├── command[1]: "grep" [".c"]
│   └── redirect_out: "output.txt"
└── background: true
```

#### Execution Flow

**Simple Command:**
```
1. fork() creates child process
2. Child: Setup any redirections (dup2())
3. Child: execve() replaces process with command
4. Parent: waitpid() for foreground, or store job for background
```

**Pipeline:**
```
Commands: cmd1 | cmd2 | cmd3

Process Tree:
    Parent (Shell)
        |
    +---+---+---+
    |   |   |
  cmd1 cmd2 cmd3

Pipe Setup:
    cmd1 stdout -> pipe[0] -> cmd2 stdin
    cmd2 stdout -> pipe[1] -> cmd3 stdin
```

**Implementation Steps for Pipeline:**
1. Create pipes: `pipe(fds)` for each connection
2. Loop through commands:
   - fork()
   - Child: dup2() to set up stdin/stdout from pipes
   - Child: Close unused pipe ends
   - Child: execve()
   - Parent: Close pipe ends it doesn't need
3. Parent: wait for all children

#### Job Control

**Job Structure:**
```c
typedef struct job {
    int job_id;
    pid_t pgid;           // Process group ID
    char *command_line;   // Original command
    process_t *processes; // List of processes
    job_state_t state;    // RUNNING, STOPPED, DONE
} job_t;
```

**Signals:**
- **SIGINT (Ctrl+C)**: Send to foreground process group
- **SIGTSTP (Ctrl+Z)**: Stop foreground job
- **SIGCHLD**: Child status changed (zombie cleanup)

**Process Groups:**
- Each pipeline is a process group
- tcsetpgrp() to give terminal control to foreground group

### Implementation Stages
1. **Stage 1**: Execute single commands (fork + exec)
2. **Stage 2**: Add arguments and path resolution
3. **Stage 3**: Implement pipes (|)
4. **Stage 4**: Add redirections (<, >, >>)
5. **Stage 5**: Job control (&, fg, bg, jobs)
6. **Stage 6**: Signal handling
7. **Stage 7**: Builtins (cd, exit, export)

---

## 4. Terminal Text Editor (Vim-like)

### Overview
Build a terminal-based text editor with modal editing, efficient text manipulation using a piece table data structure, and raw terminal mode.

### Core Concepts
- **Terminal Raw Mode**: Disable canonical mode, handle input directly
- **Piece Table**: Efficient text buffer for large files
- **Screen Buffering**: Double buffering for flicker-free rendering
- **Data Structures**: Ropes, gap buffers, or piece tables
- **Escape Sequences**: ANSI control codes for cursor/screen control

### Architecture

#### Terminal Setup
```
Save Original Terminal State (tcgetattr)
    |
    v
Enable Raw Mode:
  - Disable canonical mode (ICANON)
  - Disable echo (ECHO)
  - Disable signal generation (ISIG)
  - Read single bytes (VMIN=1, VTIME=0)
    |
    v
Enable Alternative Screen Buffer (\x1b[?1049h)
    |
    v
Hide Cursor (\x1b[?25l) [optional]
    |
    v
Clear Screen
```

#### Piece Table Data Structure

**Concept:** Store original file content + all edits in separate buffers

```
Original Buffer: "Hello World"
Add Buffer: "Beautiful "

Piece Table (sequence of pieces):
[0: original, 0, 6]      -> "Hello "
[1: add, 0, 10]          -> "Beautiful "
[0: original, 6, 5]      -> "World"

Result: "Hello Beautiful World"
```

**Structure:**
```c
typedef struct piece {
    buffer_t *buffer;     // Points to original or add buffer
    size_t start;         // Start offset in buffer
    size_t length;        // Length of piece
    struct piece *next;   // Next piece in sequence
} piece_t;

typedef struct {
    char *original;       // Original file content (immutable)
    size_t original_len;
    
    char *add;           // Append-only buffer for edits
    size_t add_len;
    size_t add_capacity;
    
    piece_t *pieces;     // Linked list of pieces
} piece_table_t;
```

**Operations:**

*Insert at position P:*
1. Find piece containing position P
2. Split piece at P
3. Create new piece pointing to add buffer
4. Append text to add buffer
5. Link: [before] -> [new] -> [after]

*Delete range [start, end]:*
1. Find pieces containing start and end
2. Split pieces at boundaries
3. Remove pieces in range
4. Link remaining pieces

#### Rendering Pipeline

```
Editor State Update
        |
        v
Calculate Visible Range
        |
        v
For each line in viewport:
  - Traverse piece table to extract text
  - Apply syntax highlighting (optional)
  - Add to render buffer
        |
        v
Move Cursor to Top-Left (\x1b[H)
        |
        v
Write Render Buffer to stdout
        |
        v
Position Cursor (\x1b[row;colH)
        |
        v
Flush stdout
```

#### Modal Editing

**Modes:**
```
NORMAL MODE (default)
  - Navigation: h,j,k,l, w,b,gg,G
  - Editing: dd, yy, p, x, r
  - Switch: i (insert), v (visual), : (command)

INSERT MODE
  - Text insertion at cursor
  - ESC returns to NORMAL

VISUAL MODE
  - Selection with movement
  - Operations on selection

COMMAND MODE
  - Line at bottom of screen
  - :w, :q, :wq, :%s/old/new/g
```

**Input Handling:**
```c
int handle_input() {
    char c = read_key();  // Single byte
    
    if (c == '\x1b') {    // Escape sequence
        char seq[3];
        seq[0] = read_key();
        seq[1] = read_key();
        
        if (seq[0] == '[') {
            switch(seq[1]) {
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                // ... etc
            }
        }
    }
    // Handle regular keys based on current mode
}
```

### Implementation Stages
1. **Stage 1**: Raw terminal mode, read single keys
2. **Stage 2**: Display file content, simple cursor movement
3. **Stage 3**: Implement piece table for text storage
4. **Stage 4**: Basic editing (insert/delete characters)
5. **Stage 5**: Line navigation, scrolling
6. **Stage 6**: Modal editing (normal/insert modes)
7. **Stage 7**: Commands (:w, :q, search)

---

## 5. Memory Allocator (malloc/free Implementation)

### Overview
Implement your own memory allocator that manages the heap, replacing malloc/free with your own versions using sbrk() or mmap().

### Core Concepts
- **System Calls**: sbrk(), mmap(), munmap()
- **Memory Layout**: Heap growth, page alignment
- **Fragmentation**: Internal and external fragmentation
- **Allocation Strategies**: First-fit, best-fit, worst-fit
- **Coalescing**: Merging adjacent free blocks

### Architecture

#### Heap Layout
```
Low Addresses
    |
    v
+------------------+
| Program Code     |
+------------------+
| Global Data      |
+------------------+
| Heap (grows up)  |  <-- sbrk() extends this
|                  |      |
|  [block]         |      v
|  [block]         |
|  ...             |
|                  |
+------------------+
|                  |
|  (Unused)        |
|                  |
+------------------+
| Stack (grows dn) |
+------------------+
    ^
    |
High Addresses
```

#### Block Structure

**Metadata (overhead for each block):**
```c
typedef struct block {
    size_t size;          // Size of user data area
    int free;             // 1 if free, 0 if allocated
    struct block *next;   // Next block in free list
    struct block *prev;   // Previous block (optional)
    // Data starts here
} block_t;

// User sees pointer to data area:
// [metadata | user_data]
//            ^
//         returned ptr
```

#### Allocation Algorithm

**malloc(size):**
```
1. Adjust size for alignment (8 or 16 byte boundary)
2. Search free list for suitable block:
   
   Strategy A - First Fit:
   - Return first block with size >= requested
   
   Strategy B - Best Fit:
   - Return smallest block with size >= requested
   
   Strategy C - Worst Fit:
   - Return largest block with size >= requested

3. If found:
   - Split block if significantly larger than needed
   - Mark as allocated
   - Return pointer to data area

4. If not found:
   - Request memory from OS (sbrk or mmap)
   - Add new block to heap
   - Return allocated block
```

**free(ptr):**
```
1. Convert user pointer to block pointer (subtract metadata size)
2. Mark block as free
3. Coalesce with adjacent free blocks:
   
   Check previous block:
   - Calculate: current - prev->size - sizeof(metadata)
   - If result points to valid free block, merge
   
   Check next block:
   - Calculate: current + sizeof(metadata) + current->size
   - If result points to free block, merge

4. Add to free list (sorted by address or LIFO)
```

#### Free List Management

**Implicit List (simpler):**
- All blocks linked in address order
- Traversal: block = block + sizeof(block) + block->size
- Must check free flag on every block

**Explicit List (better):**
- Only free blocks linked via next/prev pointers
- Faster allocation (skip allocated blocks)
- More metadata overhead

**Segregated Fits (advanced):**
- Multiple free lists for different size classes
- Size classes: 8, 16, 32, 64, 128, ... bytes
- Very fast O(1) allocation for common sizes

### Implementation Stages
1. **Stage 1**: Basic malloc/free with sbrk(), no free list reuse
2. **Stage 2**: Add free list, reuse freed blocks
3. **Stage 3**: Implement splitting and coalescing
4. **Stage 4**: Multiple allocation strategies (compare performance)
5. **Stage 5**: Thread-safe version (mutex locks)
6. **Stage 6**: Use mmap() for large allocations (> threshold)

### Testing & Debugging
- Use valgrind to detect memory leaks
- Implement malloc_usable_size() equivalent
- Add debugging info (guard bands, canaries)
- Track statistics (allocations, fragmentation)

---

## 6. JSON Parser

### Overview
Build a JSON parser that can parse JSON text into an in-memory tree structure and serialize it back to JSON.

### Core Concepts
- **Recursive Descent Parsing**: Top-down grammar parsing
- **Tokenization (Lexing)**: Converting characters to tokens
- **AST (Abstract Syntax Tree)**: Representing JSON structure
- **Memory Management**: Tree allocation and cleanup
- **Unicode**: UTF-8 string handling

### Architecture

#### Parsing Pipeline
```
Input JSON String
      |
      v
Lexer (Tokenizer)
      |
      v
Token Stream: { STRING: "name", COLON, STRING: "John", ... }
      |
      v
Parser (Recursive Descent)
      |
      v
JSON Value Tree
      |
      v
Consumer (your application)
```

#### Token Types
```c
typedef enum {
    TOK_LBRACE,      // {
    TOK_RBRACE,      // }
    TOK_LBRACKET,    // [
    TOK_RBRACKET,    // ]
    TOK_COMMA,       // ,
    TOK_COLON,       // :
    TOK_STRING,      // "..."
    TOK_NUMBER,      // 123, -45.67
    TOK_TRUE,        // true
    TOK_FALSE,       // false
    TOK_NULL,        // null
    TOK_EOF          // End of input
} token_type_t;

typedef struct {
    token_type_t type;
    char *value;      // For STRING and NUMBER
    size_t line;      // For error reporting
    size_t column;
} token_t;
```

#### JSON Value Structure
```c
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type_t;

typedef struct json_value {
    json_type_t type;
    union {
        int bool_val;
        double number_val;
        char *string_val;
        struct {
            struct json_value **items;
            size_t count;
        } array;
        struct {
            char **keys;
            struct json_value **values;
            size_t count;
        } object;
    } data;
} json_value_t;
```

#### Grammar (EBNF)
```
value     := object | array | string | number | "true" | "false" | "null"

object    := "{" [ pair *( "," pair ) ] "}"
pair      := string ":" value

array     := "[" [ value *( "," value ) ] "]"

string    := "\"" *char "\""
char      := unescaped | escape ( """ | "\" | "/" | "b" | "f" | "n" | "r" | "t" | "u" hex hex hex hex )

number    := [ "-" ] int [ frac ] [ exp ]
int       := "0" | ( digit1-9 *digit )
frac      := "." 1*digit
exp       := ( "e" | "E" ) [ "+" | "-" ] 1*digit
```

#### Recursive Descent Parser Implementation

```c
// Each grammar rule becomes a function
json_value_t *parse_value(lexer_t *lexer);
json_value_t *parse_object(lexer_t *lexer);
json_value_t *parse_array(lexer_t *lexer);
json_value_t *parse_string(lexer_t *lexer);
json_value_t *parse_number(lexer_t *lexer);

// Main parsing entry point
json_value_t *parse_json(const char *input) {
    lexer_t lexer = lexer_init(input);
    json_value_t *result = parse_value(&lexer);
    
    // Ensure we consumed all input
    token_t tok = lexer_next(&lexer);
    if (tok.type != TOK_EOF) {
        error("Unexpected token after value");
    }
    
    return result;
}

json_value_t *parse_object(lexer_t *lexer) {
    expect_token(lexer, TOK_LBRACE);
    
    json_value_t *obj = create_json_object();
    
    // Empty object: {}
    if (peek_token(lexer).type == TOK_RBRACE) {
        consume_token(lexer);
        return obj;
    }
    
    while (1) {
        // Parse key (must be string)
        token_t key = expect_token(lexer, TOK_STRING);
        expect_token(lexer, TOK_COLON);
        
        // Parse value
        json_value_t *value = parse_value(lexer);
        
        // Add to object
        json_object_add(obj, key.value, value);
        
        // Check for comma or end
        token_t next = lexer_next(lexer);
        if (next.type == TOK_RBRACE) break;
        if (next.type != TOK_COMMA) error("Expected , or }");
    }
    
    return obj;
}
```

#### String Parsing (Escape Handling)
```
Input: "Hello\nWorld\u0041"

Processing:
  H-e-l-l-o-\-n-...  (see backslash)
  |
  v
  Check next char:
    'n' -> newline (0x0A)
    't' -> tab (0x09)
    '"' -> quote (0x22)
    'u' -> parse 4 hex digits for unicode
```

#### Number Parsing
```
Input: -123.456e-7

States:
  [START] --'-'--> [NEG]
  [NEG] --digit--> [INT]
  [INT] --digit--> [INT] (loop)
  [INT] --'.'--> [FRAC_START]
  [FRAC_START] --digit--> [FRAC]
  [FRAC] --'e'/'E'--> [EXP_START]
  [EXP_START] --'+'/'-'--> [EXP_SIGN]
  [EXP_START] --digit--> [EXP]
  [EXP_SIGN] --digit--> [EXP]
  [EXP] --digit--> [EXP] (loop)
  [EXP] --other--> [END]
```

### Implementation Stages
1. **Stage 1**: Lexer - tokenize JSON input
2. **Stage 2**: Parser - recursive descent for simple values
3. **Stage 3**: Objects and arrays
4. **Stage 4**: Proper Unicode/UTF-8 support
5. **Stage 5**: Error handling with line/column info
6. **Stage 6**: Serializer (tree back to JSON string)
7. **Stage 7**: DOM-style API (getters/setters)

---

## 7. Version Control System (Git-lite)

### Overview
Build a simplified version control system with content-addressable storage, commit history, and basic diff capabilities.

### Core Concepts
- **Content-Addressable Storage**: SHA-1 hash as filename
- **Object Types**: Blob (file), Tree (directory), Commit (snapshot)
- **Directed Acyclic Graph (DAG)**: Commit history structure
- **Diff Algorithm**: Myer's diff or similar
- **Index/Staging Area**: Pre-commit file state

### Architecture

#### Repository Structure
```
.mygit/                 (or .vcs/)
├── objects/            # Content-addressable object store
│   ├── ab/
│   │   └── cd1234...  # Blob/tree/commit objects
│   └── ef/
│       └── 567890...
├── refs/               # References (branches, tags)
│   ├── heads/
│   │   └── master     # Contains commit hash
│   └── tags/
├── HEAD                # Current branch/ref
├── index               # Staging area
└── config              # Repository configuration
```

#### Object Model

**Blob (File Content):**
```
Header: "blob <size>\0"
Content: raw file bytes
SHA-1: hash(header + content)
Stored: objects/ab/cd1234... where ab are first 2 chars
```

**Tree (Directory):**
```
Format: "tree <size>\0<entries>"

Entry format:
  <mode> <name>\0<20-byte-sha>

Example:
  100644 file.txt\0<sha1>
  100755 script.sh\0<sha2>
  040000 subdir\0<sha3-tree>
```

**Commit:**
```
Format: "commit <size>\0<content>"

Content:
  tree <root-tree-sha>
  parent <parent-commit-sha>  (optional, multiple for merge)
  author <name> <email> <timestamp>
  committer <name> <email> <timestamp>
  
  <message>
```

#### Storage Operations

**Object Store (hash-object):**
```
hash_object(data, type):
    1. Create header: "<type> <length>\0"
    2. Concatenate: full = header + data
    3. Compute SHA-1 hash
    4. Compress with zlib
    5. Store at objects/<hash[0:2]>/<hash[2:]>
    6. Return hash
```

**Read Object:**
```
read_object(hash):
    1. path = objects/<hash[0:2]>/<hash[2:]>
    2. Read file
    3. Decompress with zlib
    4. Parse header for type and size
    5. Return type and content
```

#### Index/Staging Area

**Purpose:** Track what will be in the next commit

**Format:**
```c
typedef struct {
    char path[256];       // File path
    char sha1[20];        // Blob hash
    uint32_t mode;        // File permissions
    uint32_t stage;       // For merge conflicts
} index_entry_t;

typedef struct {
    index_entry_t *entries;
    size_t count;
    size_t capacity;
} index_t;
```

**Operations:**
- `add <file>`: Hash file, add/update index entry
- `remove <file>`: Remove from index
- `write-index`: Serialize to .mygit/index
- `read-index`: Deserialize from disk

#### Commit Flow
```
1. Load index
2. Convert index entries to tree objects:
   - Group by directory
   - Create tree for each directory (bottom-up)
   - Store trees, get root tree hash
3. Create commit object:
   - Parent: HEAD commit hash (if exists)
   - Tree: root tree hash
   - Author/Committer: from config or env
   - Message: user provided
4. Update HEAD reference to new commit
5. Clear index (optional)
```

#### Diff Algorithm (Myers' Algorithm)

**Problem:** Find shortest edit script between two sequences

**Concept:**
```
A: a b c a b b a
B: c b a b a c

Find Longest Common Subsequence (LCS):
  a b c a b   b a
      c b a b a   c

Edit script:
  - Delete a (pos 0)
  - Delete b (pos 1)
  + Insert c (pos 0)
  ... etc
```

**Algorithm (simplified):**
```
For edit distance d = 0 to max:
    For k = -d to d in steps of 2:
        Choose best path (from k-1 or k+1)
        Extend path while A[x] == B[y]
        If reached end of both: done
```

### Implementation Stages
1. **Stage 1**: Hash object (blob storage)
2. **Stage 2**: Tree objects (directories)
3. **Stage 3**: Commit objects with parent chain
4. **Stage 4**: Index/staging area
5. **Stage 5**: `add` and `commit` commands
6. **Stage 6**: `log` command (traverse history)
7. **Stage 7**: `diff` command
8. **Stage 8**: Branching (multiple refs)
9. **Stage 9**: Basic checkout

---

## 8. Thread Pool Library

### Overview
Build a reusable thread pool library for concurrent task execution with work queues, proper synchronization, and graceful shutdown.

### Core Concepts
- **POSIX Threads (pthreads)**: Thread creation and management
- **Mutexes**: Mutual exclusion for shared data
- **Condition Variables**: Thread signaling and waiting
- **Work Queues**: Task scheduling data structure
- **Thread Safety**: Safe concurrent access patterns

### Architecture

#### System Components
```
+---------------------+
|    Thread Pool      |
|  +---------------+  |
|  |  Work Queue   |  |  <--- Thread-safe queue
|  |  [task][task] |  |
|  +---------------+  |
|         |           |
|    +----+----+      |
|    |    |    |      |
|   [W1] [W2] [W3]   |  <--- Worker threads
|    |    |    |      |
+----+----+----+------+
     |    |    |
     v    v    v
   Execute Tasks
```

#### Thread Pool Structure
```c
typedef struct {
    // Worker threads
    pthread_t *workers;
    size_t num_workers;
    
    // Work queue
    task_t *queue;
    size_t queue_head;
    size_t queue_tail;
    size_t queue_capacity;
    size_t queue_count;
    
    // Synchronization
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;   // Signal when tasks available
    pthread_cond_t queue_not_full;    // Signal when space available
    
    // State
    int shutdown;                     // Flag for graceful shutdown
    int started;                      // Number of active workers
} threadpool_t;

typedef struct {
    void (*function)(void *);
    void *argument;
} task_t;
```

#### Worker Thread Lifecycle
```
Worker Thread:
    |
    v
Initialize
    |
    v
+--------> Lock queue mutex
|              |
|              v
|      Wait on condition:
|        - queue_not_empty OR
|        - shutdown flag
|              |
|              v
|      If shutdown: goto Cleanup
|              |
|              v
|      Dequeue task
|              |
|              v
|      Unlock mutex
|              |
|              v
|      Execute task
|              |
|              v
+--------- Loop back

Cleanup:
    Update started count
    Unlock mutex
    Exit thread
```

#### Task Submission Flow
```c
int threadpool_submit(threadpool_t *pool, 
                      void (*func)(void *), 
                      void *arg) {
    pthread_mutex_lock(&pool->queue_mutex);
    
    // Wait if queue is full (blocking submit)
    while (pool->queue_count == pool->queue_capacity && !pool->shutdown) {
        pthread_cond_wait(&pool->queue_not_full, &pool->queue_mutex);
    }
    
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->queue_mutex);
        return -1;
    }
    
    // Add task to queue
    pool->queue[pool->queue_tail].function = func;
    pool->queue[pool->queue_tail].argument = arg;
    pool->queue_tail = (pool->queue_tail + 1) % pool->queue_capacity;
    pool->queue_count++;
    
    // Signal waiting workers
    pthread_cond_signal(&pool->queue_not_empty);
    
    pthread_mutex_unlock(&pool->queue_mutex);
    return 0;
}
```

#### Synchronization Primitives

**Mutex:**
- Protects shared state (queue, counters)
- Always locked before accessing shared data
- Released as soon as possible

**Condition Variables:**
- `queue_not_empty`: Workers wait here when no tasks
- `queue_not_full`: Submitters wait here when queue full
- Always used with associated mutex
- Signal vs Broadcast: Signal wakes one, Broadcast wakes all

#### Graceful Shutdown

**Immediate Shutdown:**
1. Set shutdown flag
2. Broadcast `queue_not_empty` to wake all workers
3. Workers check flag and exit immediately
4. Join all threads
5. Free resources

**Graceful Shutdown:**
1. Set shutdown flag
2. Broadcast to wake workers
3. Workers finish current task, then exit
4. Join all threads
5. Free resources

### Implementation Stages
1. **Stage 1**: Basic thread pool with fixed workers
2. **Stage 2**: Thread-safe work queue with mutex
3. **Stage 3**: Condition variables for efficient waiting
4. **Stage 4**: Graceful shutdown mechanism
5. **Stage 5**: Dynamic thread pool (grow/shrink based on load)
6. **Stage 6**: Task priorities (multiple queues)
7. **Stage 7**: Thread-local storage and work stealing

### Advanced Features (Optional)

**Work Stealing:**
- Each worker has local queue
- Workers pull from own queue
- If empty, steal from other workers' queues
- Reduces contention on central queue

**Thread Affinity:**
- Bind threads to specific CPU cores
- Improves cache locality
- Use `pthread_setaffinity_np()`

---

## 9. TCP Chat Server

### Overview
Build a multi-client chat server supporting private messages, chat rooms, and real-time message broadcasting using non-blocking I/O.

### Core Concepts
- **Non-blocking Sockets**: O_NONBLOCK flag, non-blocking I/O
- **I/O Multiplexing**: select(), poll(), or epoll()
- **Protocol Design**: Message framing and parsing
- **Client State Management**: Connection and user state tracking
- **Broadcasting**: Efficient message distribution

### Architecture

#### Server Structure
```
Main Thread (Event Loop)
    |
    v
epoll_wait() for events
    |
    +-- New Connection --> Accept & Add to epoll
    |
    +-- Client Data --> Read & Process Message
    |
    +-- Client Disconnect --> Cleanup & Notify

Client State:
    - Socket FD
    - Username
    - Current room
    - Read buffer (partial messages)
    - Write buffer (pending outgoing)
```

#### Message Protocol

**Fixed Header + Variable Body:**
```
+--------+--------+--------+--------+--------+--------+--------+
|  MAGIC |  TYPE  | LENGTH                | CHECKSUM          |
|  1B    |  1B    |  4B (network order)   | 2B                |
+--------+--------+--------+--------+--------+--------+--------+
| PAYLOAD (LENGTH bytes)                                        |
+---------------------------------------------------------------+

Magic: 0xAB (protocol identifier)
Type:  Message type enum
Length: Payload size
Checksum: CRC16 of payload
```

**Message Types:**
```c
typedef enum {
    MSG_LOGIN,         // Client -> Server: Set username
    MSG_JOIN_ROOM,     // Client -> Server: Join room
    MSG_LEAVE_ROOM,    // Client -> Server: Leave room
    MSG_CHAT,          // Bidirectional: Chat message
    MSG_PRIVATE,       // Client -> Server: Private message
    MSG_USER_LIST,     // Server -> Client: List users
    MSG_ROOM_LIST,     // Server -> Client: List rooms
    MSG_ERROR,         // Server -> Client: Error message
    MSG_PING,          // Keepalive
    MSG_PONG           // Keepalive response
} msg_type_t;
```

#### Non-blocking I/O

**Why Non-blocking?**
- Single thread can handle many connections
- No thread-per-connection overhead
- Scales to thousands of connections

**Setting Non-blocking:**
```c
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

**Reading (Non-blocking):**
```c
ssize_t n = read(fd, buffer, size);
if (n > 0) {
    // Process data
} else if (n == 0) {
    // Connection closed
} else { // n < 0
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available now, try later
    } else {
        // Error
    }
}
```

#### I/O Multiplexing with epoll

```c
// Create epoll instance
int epoll_fd = epoll_create1(0);

// Add server socket to epoll
struct epoll_event ev;
ev.events = EPOLLIN;  // Watch for readable
ev.data.fd = server_fd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

// Event loop
struct epoll_event events[MAX_EVENTS];
while (1) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    
    for (int i = 0; i < nfds; i++) {
        if (events[i].data.fd == server_fd) {
            // New connection
            int client_fd = accept(server_fd, ...);
            set_nonblocking(client_fd);
            
            ev.events = EPOLLIN | EPOLLET;  // Edge-triggered
            ev.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            
            add_client(client_fd);
        } else {
            // Client data
            handle_client_data(events[i].data.fd);
        }
    }
}
```

#### Message Parsing State Machine

```
[START] --read 1B--> Check MAGIC
                         |
                         v (valid)
                    [READ_TYPE] --read 1B--> Get TYPE
                                                  |
                                                  v
                                           [READ_LENGTH] --read 4B--> Get LENGTH
                                                                          |
                                                                          v
                                                                   [READ_PAYLOAD] 
                                                                   --read LENGTH bytes--> 
                                                                   Store payload
                                                                          |
                                                                          v
                                                                   [READ_CHECKSUM] 
                                                                   --read 2B--> 
                                                                   Validate CRC
                                                                          |
                                                                          v (valid)
                                                                   [DISPATCH]
```

#### Data Structures

```c
// Client connection
typedef struct client {
    int fd;
    char username[32];
    char current_room[32];
    
    // Read state
    uint8_t read_buffer[4096];
    size_t read_len;
    parse_state_t parse_state;
    
    // Write queue (for partial writes)
    struct msg_buffer *write_queue;
    
    struct client *next;
} client_t;

// Chat room
typedef struct room {
    char name[32];
    client_t *members;
    struct room *next;
} room_t;

// Server state
typedef struct {
    int server_fd;
    int epoll_fd;
    client_t *clients;
    room_t *rooms;
} server_t;
```

### Implementation Stages
1. **Stage 1**: Blocking sockets, single client
2. **Stage 2**: select() with multiple clients
3. **Stage 3**: Non-blocking sockets with epoll
4. **Stage 4**: Message framing protocol
5. **Stage 5**: Username/login system
6. **Stage 6**: Chat rooms
7. **Stage 7**: Private messaging
8. **Stage 8**: Write buffering (handle partial sends)

---

## 10. ELF Binary Parser

### Overview
Build a parser for ELF (Executable and Linkable Format) files to read headers, sections, symbols, and program segments.

### Core Concepts
- **ELF Format**: Executable file structure on Linux/Unix
- **Memory Mapping**: mmap() for reading files
- **Data Structures**: ELF headers, section headers, program headers
- **Endianness**: Handling both little and big endian
- **32/64-bit Support**: Handling both architectures

### Architecture

#### ELF File Structure
```
+------------------+
| ELF Header       |  (64 bytes for 64-bit)
| - Entry point    |
| - Section header |
|   offset         |
+------------------+
| Program Headers  |  (Loadable segments)
| - Load addresses |
| - File/Mem sizes |
+------------------+
| Section Data     |
| - .text (code)   |
| - .data (data)   |
| - .rodata        |
| - .symtab        |
| - .strtab        |
| - etc.           |
+------------------+
| Section Headers  |  (Metadata about sections)
| - Name, type     |
| - Size, offset   |
+------------------+
```

#### Key Data Structures

**ELF Header (64-bit):**
```c
typedef struct {
    uint8_t  e_ident[16];    // Magic, class, endian, etc.
    uint16_t e_type;         // ET_REL, ET_EXEC, ET_DYN
    uint16_t e_machine;      // Target architecture
    uint32_t e_version;
    uint64_t e_entry;        // Entry point address
    uint64_t e_phoff;        // Program header offset
    uint64_t e_shoff;        // Section header offset
    uint32_t e_flags;
    uint16_t e_ehsize;       // ELF header size
    uint16_t e_phentsize;    // Program header entry size
    uint16_t e_phnum;        // Number of program headers
    uint16_t e_shentsize;    // Section header entry size
    uint16_t e_shnum;        // Number of section headers
    uint16_t e_shstrndx;     // Section name string table index
} Elf64_Ehdr;

// e_ident breakdown:
// [0-3]: Magic 0x7F 'E' 'L' 'F'
// [4]:   Class (32/64-bit)
// [5]:   Data encoding (little/big endian)
// [6]:   ELF version
// [7]:   OS/ABI
// [8-15]: Padding
```

**Section Header:**
```c
typedef struct {
    uint32_t sh_name;        // Section name (index in .shstrtab)
    uint32_t sh_type;        // SHT_NULL, SHT_PROGBITS, SHT_SYMTAB, etc.
    uint64_t sh_flags;       // SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR
    uint64_t sh_addr;        // Address in memory
    uint64_t sh_offset;      // Offset in file
    uint64_t sh_size;        // Section size
    uint32_t sh_link;        // Link to another section
    uint32_t sh_info;        // Additional info
    uint64_t sh_addralign;   // Alignment
    uint64_t sh_entsize;     // Entry size (for tables)
} Elf64_Shdr;
```

**Symbol Table Entry:**
```c
typedef struct {
    uint32_t st_name;        // Symbol name (index in .strtab)
    uint8_t  st_info;        // Type and binding
    uint8_t  st_other;       // Visibility
    uint16_t st_shndx;       // Section index
    uint64_t st_value;       // Symbol value/address
    uint64_t st_size;        // Symbol size
} Elf64_Sym;

// st_info breakdown:
// Lower 4 bits: Type (STT_NOTYPE, STT_OBJECT, STT_FUNC, etc.)
// Upper 4 bits: Binding (STB_LOCAL, STB_GLOBAL, STB_WEAK)
```

#### Parsing Flow

```
1. Open file and mmap() into memory
   ptr = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

2. Read ELF Header
   Verify magic: ptr[0] == 0x7F, ptr[1-3] == "ELF"
   Determine: 32/64-bit, little/big endian

3. Read Program Headers
   phdr_table = ptr + ehdr.e_phoff
   For i in 0..ehdr.e_phnum-1:
       phdr = phdr_table[i]
       Print segment info

4. Read Section Headers
   shdr_table = ptr + ehdr.e_shoff
   shstrtab = ptr + shdr_table[ehdr.e_shstrndx].sh_offset
   
   For i in 0..ehdr.e_shnum-1:
       shdr = shdr_table[i]
       name = shstrtab + shdr.sh_name
       Print section info

5. Parse Specific Sections
   Find .symtab section
   strtab = section referenced by .symtab.sh_link
   
   For each symbol in .symtab:
       name = strtab + symbol.st_name
       Print symbol info
```

#### Section Types of Interest

| Section | Type | Description |
|---------|------|-------------|
| .text | SHT_PROGBITS | Executable code |
| .data | SHT_PROGBITS | Initialized data |
| .rodata | SHT_PROGBITS | Read-only data |
| .bss | SHT_NOBITS | Uninitialized data |
| .symtab | SHT_SYMTAB | Symbol table |
| .strtab | SHT_STRTAB | String table |
| .rel.* | SHT_REL | Relocation entries |
| .dynamic | SHT_DYNAMIC | Dynamic linking info |

### Implementation Stages
1. **Stage 1**: Read and validate ELF header
2. **Stage 2**: Parse program headers (segments)
3. **Stage 3**: Parse section headers
4. **Stage 4**: Print section names using .shstrtab
5. **Stage 5**: Parse symbol table (.symtab)
6. **Stage 6**: Parse string table (.strtab)
7. **Stage 7**: Handle both 32-bit and 64-bit ELFs
8. **Stage 8**: Handle endianness conversion

### Tools to Compare
- `readelf -a <binary>` - Display all information
- `objdump -x <binary>` - Display headers
- `nm <binary>` - Display symbol table

---

## 11. Regular Expression Engine

### Overview
Build a regular expression engine supporting basic operators (concatenation, alternation, Kleene star) using NFAs and DFAs.

### Core Concepts
- **Finite Automata**: NFA (Nondeterministic) and DFA (Deterministic)
- **Thompson's Construction**: Regex to NFA conversion
- **Subset Construction**: NFA to DFA conversion
- **Epsilon Closures**: Computing reachable states
- **Pattern Matching**: Running automata against input

### Architecture

#### Regex to NFA (Thompson's Construction)

**Basic Building Blocks:**

*Character 'a':*
```
  (0) --a--> (1)
```

*Concatenation AB:*
```
  [NFA for A] --epsilon--> [NFA for B]
```

*Alternation A|B:*
```
      +--epsilon--> [NFA for A] --epsilon--+
(0) --+                                      +--> (f)
      +--epsilon--> [NFA for B] --epsilon--+
```

*Kleene Star A*:**
```
           +--epsilon--+
           |           |
(0) --epsilon--> [NFA for A] --epsilon-->(f)
           |                           ^
           +-----------epsilon---------+
```

**Construction Algorithm:**
```
Parse regex into AST
    |
    v
Traverse AST recursively:
    - Literal: Create 2-state NFA
    - Concat: Connect NFA(A) end to NFA(B) start
    - Union: Create new start/end, add epsilon branches
    - Star: Add epsilon loops
    |
    v
Return complete NFA
```

#### NFA Structure

```c
typedef struct nfa_state {
    int id;
    struct {
        char c;                    // Input character or \0 for epsilon
        struct nfa_state *state;   // Target state
    } transitions[2];              // NFA has at most 2 transitions
    int num_transitions;
} nfa_state_t;

typedef struct {
    nfa_state_t *start;
    nfa_state_t *accept;
} nfa_t;
```

#### NFA to DFA (Subset Construction)

**Key Idea:** Each DFA state represents a set of NFA states

**Epsilon Closure:**
```
epsilon_closure(states):
    stack = states
    closure = states
    
    while stack not empty:
        state = pop(stack)
        for each transition on epsilon from state:
            if target not in closure:
                add to closure
                push(target)
    
    return closure
```

**Construction Algorithm:**
```
DFA = empty
start_state = epsilon_closure({NFA.start})
add start_state to DFA
mark start_state as unvisited

while unvisited states in DFA:
    T = pick unvisited state
    mark T as visited
    
    for each input symbol 'a':
        U = epsilon_closure(move(T, 'a'))
        if U not empty:
            if U not in DFA:
                add U to DFA
                mark U as unvisited
            add transition T --'a'--> U to DFA
```

#### DFA Structure

```c
typedef struct dfa_state {
    int id;
    int nfa_states[MAX_NFA_STATES];  // Set of NFA states
    int num_nfa_states;
    int is_accepting;
    struct dfa_state *transitions[256];  // One per ASCII char
} dfa_state_t;

typedef struct {
    dfa_state_t *start;
    dfa_state_t **states;
    int num_states;
} dfa_t;
```

#### Matching Algorithm (DFA)

```
match(dfa, input):
    state = dfa.start
    
    for each char c in input:
        state = state.transitions[c]
        if state is NULL:
            return false
    
    return state.is_accepting
```

### Implementation Stages
1. **Stage 1**: Regex tokenizer (handle (, ), |, *, +, ?)
2. **Stage 2**: Regex parser (build AST with proper precedence)
3. **Stage 3**: Thompson's construction (AST to NFA)
4. **Stage 4**: Epsilon closure computation
5. **Stage 5**: NFA to DFA conversion
6. **Stage 6**: DFA matching
7. **Stage 7**: Add character classes [a-z], . (any)
8. **Stage 8**: Add anchors ^ and $

### Supported Syntax

| Pattern | Meaning |
|---------|---------|
| `a` | Match literal 'a' |
| `.` | Match any character |
| `ab` | Concatenation (a then b) |
| `a\|b` | Alternation (a or b) |
| `a*` | Zero or more |
| `a+` | One or more (aa*) |
| `a?` | Zero or one |
| `[abc]` | Character class |
| `[^abc]` | Negated class |
| `(ab)` | Grouping |

---

## 12. Huffman File Compressor

### Overview
Build a file compression tool using Huffman coding, a lossless compression algorithm that assigns variable-length codes based on character frequency.

### Core Concepts
- **Huffman Coding**: Prefix-free optimal code construction
- **Binary Trees**: Huffman tree construction
- **Priority Queues (Min-Heap)**: Building tree efficiently
- **Bit Manipulation**: Packing variable-length codes
- **File I/O**: Binary file reading/writing

### Architecture

#### Compression Flow
```
Input File
    |
    v
[1] Frequency Analysis
    - Count byte frequencies
    |
    v
[2] Build Huffman Tree
    - Use min-heap/priority queue
    |
    v
[3] Generate Codes
    - Traverse tree, assign codes
    |
    v
[4] Write Header
    - File signature
    - Tree structure or code table
    - Original file size
    |
    v
[5] Encode Data
    - Replace each byte with its code
    - Pack bits into bytes
    |
    v
Output File (.huf)
```

#### Decompression Flow
```
Input File (.huf)
    |
    v
[1] Read Header
    - Verify signature
    - Reconstruct tree/table
    - Get original size
    |
    v
[2] Decode Data
    - Read bits one at a time
    - Traverse tree until leaf
    - Output byte, restart at root
    - Repeat until original size reached
    |
    v
Output File (original)
```

#### Huffman Tree Construction

**Algorithm:**
```
1. Create leaf node for each byte with frequency > 0
   - Store in min-heap by frequency

2. While heap has more than 1 node:
   a. left = extract_min()
   b. right = extract_min()
   c. Create new internal node:
      - frequency = left.freq + right.freq
      - left child = left
      - right child = right
   d. Insert new node into heap

3. Root = extract_min() (remaining node)
```

**Example:**
```
Frequencies: A=45, B=13, C=12, D=16, E=9, F=5

Step 1: Create leaves
        (A,45) (B,13) (C,12) (D,16) (E,9) (F,5)

Step 2: Combine F(5)+E(9)=14
        (A,45) (B,13) (C,12) (D,16) (14)
                                        /\
                                      (E,9)(F,5)

Step 3: Combine C(12)+B(13)=25
        (A,45) (D,16) (14) (25)
                          /\      /\
                        (C,12)(B,13)

Step 4: Combine 14+16=30
        (A,45) (25) (30)
                      /\
                    (14)(D,16)
                    /\
                  (E,9)(F,5)

Continue until single tree remains...
```

#### Code Generation

```
Traverse tree recursively:
    At each left branch: append '0' to code
    At each right branch: append '1' to code
    At leaf node: store code for that character

Result (example):
    A: 0
    B: 101
    C: 100
    D: 111
    E: 1101
    F: 1100
```

#### Bit Packing

**Challenge:** Codes are variable length (not byte-aligned)

**Solution:** Use a bit buffer
```c
typedef struct {
    uint8_t buffer;      // Current byte being built
    int bit_count;       // Number of bits in buffer (0-7)
    FILE *output;        // Output file
} bit_writer_t;

void write_bit(bit_writer_t *w, int bit) {
    w->buffer = (w->buffer << 1) | (bit & 1);
    w->bit_count++;
    
    if (w->bit_count == 8) {
        fwrite(&w->buffer, 1, 1, w->output);
        w->buffer = 0;
        w->bit_count = 0;
    }
}

void flush_bits(bit_writer_t *w) {
    if (w->bit_count > 0) {
        w->buffer <<= (8 - w->bit_count);  // Pad with zeros
        fwrite(&w->buffer, 1, 1, w->output);
    }
}
```

#### File Format

```
+----------------------------------+
| Signature: "HUF\0" (4 bytes)     |
+----------------------------------+
| Original File Size (8 bytes)     |
+----------------------------------+
| Frequency Table (256 * 8 bytes)  |
| - Array of 256 64-bit counts     |
+----------------------------------+
| Compressed Data                  |
| - Variable length bit stream     |
+----------------------------------+
```

#### Data Structures

```c
// Huffman tree node
typedef struct huff_node {
    uint8_t byte;              // Byte value (valid if leaf)
    uint64_t freq;             // Frequency
    int is_leaf;               // 1 if leaf node
    struct huff_node *left;
    struct huff_node *right;
} huff_node_t;

// Code table entry
typedef struct {
    uint32_t code;             // Binary code
    int length;                // Code length in bits
} huff_code_t;

// Priority queue node
typedef struct {
    huff_node_t *node;
} heap_node_t;
```

### Implementation Stages
1. **Stage 1**: Frequency counting
2. **Stage 2**: Min-heap implementation
3. **Stage 3**: Build Huffman tree
4. **Stage 4**: Generate codes from tree
5. **Stage 5**: Bit packing for encoding
6. **Stage 6**: Bit reading for decoding
7. **Stage 7**: File header with tree info
8. **Stage 8**: Full compress/decompress

### Optimization: Canonical Huffman Coding
Instead of storing the entire tree, store code lengths and reconstruct:
1. Count how many codes of each length
2. Assign codes in order
3. Reduces header size significantly

---

## 13. Build System (Make Clone)

### Overview
Build a simplified Make clone that reads build rules from a file, constructs a dependency graph, and executes commands in the correct order.

### Core Concepts
- **Dependency Graph**: Directed graph of build targets
- **Topological Sort**: Determining build order
- **File Timestamps**: Checking if rebuild is needed
- **Parallel Execution**: Running independent jobs concurrently
- **Pattern Rules**: Implicit rules for file types

### Architecture

#### Build File Format (Makefile)
```makefile
# Variables
CC = gcc
CFLAGS = -Wall -g
OBJDIR = obj

# Explicit rules
target: dependencies
<tab>command

# Example
app: main.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c header.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Pattern rules
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Phony targets
.PHONY: clean
clean:
	rm -f *.o app
```

#### Parsing Pipeline
```
Makefile
    |
    v
Lexer: Tokenize lines
    - Lines ending with \ continue
    - Ignore comments (#)
    - Separate rules vs commands
    |
    v
Parser: Build AST
    - Variable assignments
    - Rule definitions
    - Command lines
    |
    v
Dependency Graph
    - Nodes: Targets
    - Edges: Dependencies
```

#### Data Structures

```c
// Variable
typedef struct {
    char *name;
    char *value;
} variable_t;

// Rule
typedef struct rule {
    char **targets;        // Can have multiple targets
    size_t num_targets;
    
    char **dependencies;
    size_t num_dependencies;
    
    char **commands;
    size_t num_commands;
    
    int is_phony;         // .PHONY target
} rule_t;

// Build graph node
typedef struct node {
    char *name;
    rule_t *rule;         // NULL if no rule (source file)
    struct node **deps;   // Dependency nodes
    size_t num_deps;
    
    // For topological sort
    int visited;
    int in_progress;      // For cycle detection
} graph_node_t;
```

#### Dependency Graph Construction

```
build_graph(targets, rules):
    graph = empty hash map
    
    for each target:
        node = create_or_get_node(graph, target)
        rule = find_rule(rules, target)
        
        if rule exists:
            for each dependency in rule:
                dep_node = create_or_get_node(graph, dependency)
                add edge: node -> dep_node
                
                # Recursively build subgraph
                build_subgraph(dep_node, rules, graph)
    
    return graph
```

#### Topological Sort

**Algorithm (DFS-based):**
```
topological_sort(node, sorted, visiting):
    if node in visiting:
        error("Circular dependency detected")
    
    if node already in sorted:
        return
    
    add node to visiting
    
    for each dependency in node.deps:
        topological_sort(dependency, sorted, visiting)
    
    remove node from visiting
    add node to sorted

# Result is in reverse order
reverse(sorted)
```

#### Build Execution

**Sequential Build:**
```
build(target):
    graph = build_dependency_graph(target)
    order = topological_sort(graph)
    
    for each node in order:
        if needs_rebuild(node):
            execute_commands(node.rule)

needs_rebuild(node):
    if node has no rule (source file):
        return false  # Assume source exists
    
    if target file doesn't exist:
        return true
    
    target_time = mtime(node.name)
    
    for each dependency in node.deps:
        if mtime(dependency) > target_time:
            return true
    
    return false
```

**Parallel Build:**
```
build_parallel(target, num_jobs):
    graph = build_dependency_graph(target)
    
    # Track state: PENDING, READY, RUNNING, DONE
    ready_queue = nodes with no dependencies
    running = empty set
    
    while not all done:
        # Start new jobs
        while running.count < num_jobs and ready_queue not empty:
            node = ready_queue.pop()
            start_job(node.command)
            add node to running
        
        # Wait for job to complete
        node = wait_for_any_job()
        remove node from running
        mark node as DONE
        
        # Update ready queue
        for each dependent of node:
            if all dependencies done:
                add dependent to ready_queue
```

#### Variable Substitution

```
Substitute variables in string:
    Find $(VAR) or ${VAR} patterns
    Replace with variable value
    Handle recursive expansion

Special variables:
    $@ - Target name
    $< - First dependency
    $^ - All dependencies
    $? - Dependencies newer than target
    $* - Stem in pattern rules
```

### Implementation Stages
1. **Stage 1**: Parse simple variable assignments
2. **Stage 2**: Parse rules with single target
3. **Stage 3**: Parse commands and build graph
4. **Stage 4**: Topological sort
5. **Stage 5**: Timestamp checking and rebuilding
6. **Stage 6**: Execute commands
7. **Stage 7**: Variable substitution
8. **Stage 8**: Pattern rules (%)
9. **Stage 9**: Parallel job execution

---

## 14. Virtual Machine / Bytecode Interpreter

### Overview
Build a stack-based virtual machine that executes bytecode instructions, featuring arithmetic operations, control flow, and function calls.

### Core Concepts
- **Bytecode**: Intermediate instruction format
- **Stack Machine**: Operations work on operand stack
- **Instruction Set**: Design of VM operations
- **Call Stack**: Function call management
- **Instruction Pointer**: Program counter

### Architecture

#### VM Components
```
+------------------+
| Instruction      |  IP (Instruction Pointer)
| Pointer          |
+------------------+
|                  |
| Call Stack       |  Frames with local variables
| [frame][frame]   |
|                  |
+------------------+
|                  |
| Operand Stack    |  Working data stack
| [value][value]   |
|                  |
+------------------+
|                  |
| Constant Pool    |  String literals, constants
| [const][const]   |
|                  |
+------------------+
|                  |
| Heap             |  Objects, dynamic allocation
|                  |
+------------------+
```

#### Instruction Set Design

**Stack Operations:**
```
PUSH_CONST idx    # Push constant[idx] to stack
POP               # Remove top of stack
DUP               # Duplicate top value
SWAP              # Swap top two values
```

**Arithmetic:**
```
ADD               # Pop b, pop a, push a+b
SUB               # Pop b, pop a, push a-b
MUL               # Pop b, pop a, push a*b
DIV               # Pop b, pop a, push a/b
MOD               # Pop b, pop a, push a%b
NEG               # Pop a, push -a
```

**Comparisons:**
```
EQ                # Pop b, pop a, push a==b
NE                # Pop b, pop a, push a!=b
LT                # Pop b, pop a, push a<b
GT                # Pop b, pop a, push a>b
LE                # Pop b, pop a, push a<=b
GE                # Pop b, pop a, push a>=b
```

**Control Flow:**
```
JUMP offset       # IP = IP + offset
JUMP_IF_TRUE off  # Pop, if true: IP = IP + offset
JUMP_IF_FALSE off # Pop, if false: IP = IP + offset
```

**Variables:**
```
LOAD_LOCAL idx    # Push local[idx]
STORE_LOCAL idx   # Pop to local[idx]
LOAD_GLOBAL idx   # Push global[idx]
STORE_GLOBAL idx  # Pop to global[idx]
```

**Functions:**
```
CALL idx argc     # Call function[idx] with argc args
RETURN            # Return from function
RETURN_VALUE      # Pop value, return it
```

**I/O:**
```
PRINT             # Pop and print value
PRINT_STRING idx  # Print string constant
READ              # Read input, push to stack
```

#### Bytecode Format

```
+----------------------------------+
| Magic: "VM\0\0" (4 bytes)        |
+----------------------------------+
| Version (4 bytes)                |
+----------------------------------+
| Constant Pool Size (4 bytes)     |
+----------------------------------+
| Constant Pool                    |
| - Type tag (1 byte)              |
| - Data (variable)                |
+----------------------------------+
| Code Size (4 bytes)              |
+----------------------------------+
| Bytecode Instructions            |
+----------------------------------+
```

#### Data Structures

```c
// VM Value type
typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_STRING,
    VAL_NULL
} value_type_t;

typedef struct {
    value_type_t type;
    union {
        int64_t int_val;
        double float_val;
        int bool_val;
        char *string_val;
    } as;
} value_t;

// Call frame
typedef struct {
    uint8_t *return_ip;      // Return address
    value_t *locals;         // Local variables array
    int num_locals;
    value_t *stack_base;     // Base of operand stack for this frame
} frame_t;

// VM State
typedef struct {
    uint8_t *bytecode;       // Program code
    size_t bytecode_size;
    
    uint8_t *ip;             // Instruction pointer
    
    value_t *constants;      // Constant pool
    size_t num_constants;
    
    value_t *globals;        // Global variables
    size_t num_globals;
    
    frame_t *frames;         // Call stack
    int frame_count;
    int frame_capacity;
    
    value_t *stack;          // Operand stack
    int stack_top;
    int stack_capacity;
} vm_t;
```

#### Execution Loop

```c
void run(vm_t *vm) {
    while (vm->ip < vm->bytecode + vm->bytecode_size) {
        uint8_t opcode = *vm->ip++;
        
        switch (opcode) {
            case OP_PUSH_CONST: {
                uint16_t idx = read_uint16(vm->ip);
                vm->ip += 2;
                push(vm, vm->constants[idx]);
                break;
            }
            
            case OP_ADD: {
                value_t b = pop(vm);
                value_t a = pop(vm);
                push(vm, add_values(a, b));
                break;
            }
            
            case OP_JUMP: {
                int16_t offset = read_int16(vm->ip);
                vm->ip += 2;
                vm->ip += offset;
                break;
            }
            
            case OP_CALL: {
                uint16_t func_idx = read_uint16(vm->ip);
                vm->ip += 2;
                uint8_t argc = *vm->ip++;
                call_function(vm, func_idx, argc);
                break;
            }
            
            case OP_RETURN: {
                return_from_function(vm);
                break;
            }
            
            // ... more opcodes
        }
    }
}
```

### Implementation Stages
1. **Stage 1**: Basic VM structure and execution loop
2. **Stage 2**: Stack operations (push, pop, arithmetic)
3. **Stage 3**: Constant pool and variables
4. **Stage 4**: Control flow (jumps)
5. **Stage 5**: Functions and call stack
6. **Stage 6**: Bytecode assembler (text to binary)
7. **Stage 7**: Compiler from simple language to bytecode

### Sample Assembly
```asm
; Calculate factorial of 5
    PUSH_CONST 1      ; constant[1] = 5
    STORE_LOCAL 0     ; n = 5
    PUSH_CONST 0      ; constant[0] = 1
    STORE_LOCAL 1     ; result = 1

loop:
    LOAD_LOCAL 0
    PUSH_CONST 2      ; constant[2] = 0
    LE
    JUMP_IF_TRUE end  ; if n <= 0, goto end
    
    LOAD_LOCAL 1
    LOAD_LOCAL 0
    MUL
    STORE_LOCAL 1     ; result = result * n
    
    LOAD_LOCAL 0
    PUSH_CONST 0
    SUB
    STORE_LOCAL 0     ; n = n - 1
    
    JUMP loop

end:
    LOAD_LOCAL 1
    PRINT             ; Print result
    HALT
```

---

## 15. DNS Resolver

### Overview
Build a DNS resolver that queries DNS servers to resolve domain names to IP addresses, implementing recursive resolution with caching.

### Core Concepts
- **DNS Protocol**: UDP-based query/response protocol
- **Message Format**: DNS packet structure
- **Record Types**: A, AAAA, CNAME, NS, MX, etc.
- **Recursive Resolution**: Following CNAME and NS chains
- **Caching**: Storing results to avoid repeated queries
- **UDP Sockets**: Connectionless network I/O

### Architecture

#### DNS Resolution Flow
```
User Query: resolve("www.example.com")
    |
    v
Check Local Cache
    |-- Hit: Return cached result
    |-- Miss: Continue
    |
    v
Send Query to Recursive DNS Server
    (e.g., 8.8.8.8, 1.1.1.1)
    |
    v
Parse Response
    |
    +-- Direct Answer: Cache and return
    +-- CNAME: Follow alias (recursive call)
    +-- Referral: Query authoritative server
    |
    v
Return IP Address(es)
```

#### DNS Message Format

**Header (12 bytes):**
```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           ID          |QR|   Opcode  |AA|TC|RD|RA|   Z    |RCode|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           QDCOUNT     |           ANCOUNT                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           NSCOUNT     |           ARCOUNT                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

ID:      16-bit identifier
QR:      0=Query, 1=Response
Opcode:  0=Standard query
AA:      Authoritative Answer
TC:      Truncated
RD:      Recursion Desired
RA:      Recursion Available
RCODE:   Response code (0=No error)
QDCOUNT: Number of questions
ANCOUNT: Number of answers
NSCOUNT: Number of authority records
ARCOUNT: Number of additional records
```

**Question Section:**
```
+----------------------------------+
| QNAME: Variable length           |
| - Labels encoded as: len + chars |
| - Terminated by 0 byte           |
| - Compression pointers supported |
+----------------------------------+
| QTYPE: 2 bytes (A=1, NS=2, etc.) |
+----------------------------------+
| QCLASS: 2 bytes (IN=1)           |
+----------------------------------+

Example "www.example.com":
  3 w w w 7 e x a m p l e 3 c o m 0
  |_____| |_________| |_____| |
    3       7           3     end
```

**Resource Record Format:**
```
+----------------------------------+
| NAME: Variable (domain name)     |
+----------------------------------+
| TYPE: 2 bytes                    |
+----------------------------------+
| CLASS: 2 bytes                   |
+----------------------------------+
| TTL: 4 bytes (cache lifetime)    |
+----------------------------------+
| RDLENGTH: 2 bytes                |
+----------------------------------+
| RDATA: Variable (IP for A, etc.) |
+----------------------------------+
```

#### Data Structures

```c
// DNS Header
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header_t;

// DNS Question
typedef struct {
    char *qname;
    uint16_t qtype;
    uint16_t qclass;
} dns_question_t;

// DNS Resource Record
typedef struct {
    char *name;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t *rdata;
    
    // Parsed data
    char *rdata_str;         // String representation
} dns_record_t;

// DNS Message
typedef struct {
    dns_header_t header;
    dns_question_t *questions;
    dns_record_t *answers;
    dns_record_t *authorities;
    dns_record_t *additionals;
} dns_message_t;

// Cache entry
typedef struct {
    char *domain;
    uint16_t type;
    char **results;
    int num_results;
    time_t expires;          // TTL expiration
} cache_entry_t;
```

#### Name Compression

DNS uses compression to avoid repeating domain names:
```
Pointer format (2 bytes):
  1 1 | 14-bit offset
  | |
  | +-- Reserved bits (must be 1)
  +---- Indicates pointer

Example message:
  |www|example|com|0|    <- Full name at offset 12
  ...
  |www|example|com|0|    <- Repeated at offset 35
  
With compression:
  |www|example|com|0|    <- Full name at offset 12
  ...
  |0xc0|0x0c|            <- Pointer to offset 12
```

#### Query Building

```c
int build_query(const char *domain, uint16_t type, 
                uint8_t *buffer, size_t buf_size) {
    dns_header_t *header = (dns_header_t *)buffer;
    
    // Set header
    header->id = htons(random() & 0xFFFF);
    header->flags = htons(0x0100);  // RD=1 (recursion desired)
    header->qdcount = htons(1);
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;
    
    // Encode domain name
    uint8_t *ptr = buffer + sizeof(dns_header_t);
    ptr += encode_name(domain, ptr);
    
    // QTYPE and QCLASS
    *(uint16_t *)ptr = htons(type);    ptr += 2;
    *(uint16_t *)ptr = htons(1);       ptr += 2;  // IN=Internet
    
    return ptr - buffer;  // Total message size
}
```

#### Response Parsing

```c
dns_message_t *parse_response(uint8_t *buffer, size_t len) {
    dns_message_t *msg = malloc(sizeof(dns_message_t));
    uint8_t *ptr = buffer;
    
    // Parse header
    msg->header.id = ntohs(*(uint16_t *)ptr);      ptr += 2;
    msg->header.flags = ntohs(*(uint16_t *)ptr);   ptr += 2;
    // ... parse rest of header
    
    int num_questions = ntohs(msg->header.qdcount);
    int num_answers = ntohs(msg->header.ancount);
    
    // Parse questions
    for (int i = 0; i < num_questions; i++) {
        ptr += parse_name(buffer, ptr, &msg->questions[i].qname);
        msg->questions[i].qtype = ntohs(*(uint16_t *)ptr);  ptr += 2;
        msg->questions[i].qclass = ntohs(*(uint16_t *)ptr); ptr += 2;
    }
    
    // Parse answers
    for (int i = 0; i < num_answers; i++) {
        ptr += parse_name(buffer, ptr, &msg->answers[i].name);
        // Parse rest of record...
    }
    
    return msg;
}
```

#### Caching System

```c
typedef struct {
    cache_entry_t *entries;
    int count;
    int capacity;
} dns_cache_t;

void cache_add(dns_cache_t *cache, const char *domain, 
               uint16_t type, dns_record_t *records, int count) {
    // Check if exists
    cache_entry_t *entry = cache_lookup(cache, domain, type);
    
    if (entry) {
        // Update existing
        free_cache_entry(entry);
    } else {
        // Create new
        entry = &cache->entries[cache->count++];
    }
    
    entry->domain = strdup(domain);
    entry->type = type;
    entry->num_results = count;
    entry->results = malloc(count * sizeof(char *));
    
    // Find minimum TTL
    uint32_t min_ttl = UINT32_MAX;
    for (int i = 0; i < count; i++) {
        if (records[i].ttl < min_ttl)
            min_ttl = records[i].ttl;
        entry->results[i] = strdup(records[i].rdata_str);
    }
    
    entry->expires = time(NULL) + min_ttl;
}
```

### Implementation Stages
1. **Stage 1**: UDP socket creation
2. **Stage 2**: DNS message structure and byte order
3. **Stage 3**: Build A record query
4. **Stage 4**: Parse simple response
5. **Stage 5**: Handle name compression
6. **Stage 6**: Support multiple record types (A, AAAA, CNAME)
7. **Stage 7**: Implement caching
8. **Stage 8**: Recursive resolution (follow CNAME chains)
9. **Stage 9**: Handle truncated responses (switch to TCP)

---

## General Implementation Tips

### Project Structure
```
project_name/
├── src/
│   ├── main.c
│   ├── module1.c
│   ├── module2.c
│   └── utils.c
├── include/
│   ├── module1.h
│   ├── module2.h
│   └── utils.h
├── tests/
│   ├── test_module1.c
│   └── test_module2.c
├── Makefile
└── README.md
```

### Debugging Tools
- **GDB**: Step through code, examine variables
- **Valgrind**: Memory leak detection, buffer overflow
- **AddressSanitizer**: Runtime memory error detection
- **GDB Dashboard**: Visual debugging interface

### Testing Strategy
1. Unit tests for individual functions
2. Integration tests for complete workflows
3. Edge cases: empty input, maximum sizes
4. Error handling: invalid input, resource exhaustion
5. Fuzz testing for parsers and network code

### Common Pitfalls
- **Memory leaks**: Always free what you allocate
- **Buffer overflows**: Use bounds checking
- **Integer overflow**: Check before arithmetic
- **Undefined behavior**: Avoid UB (uninitialized vars, etc.)
- **Resource exhaustion**: Handle malloc failures
- **Endianness**: Convert network byte order
- **Signal safety**: Be careful in signal handlers

### Learning Resources
- **"The C Programming Language"** (K&R) - Classic
- **"Computer Systems: A Programmer's Perspective"** - Deep systems knowledge
- **"The Linux Programming Interface"** - System calls
- **Beej's Guide to Network Programming** - Free online
- **man pages**: `man 2 <syscall>`, `man 3 <function>`

---

Happy coding! Build these projects in order, as each teaches skills used in the next.
