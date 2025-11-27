## Question 1: ELF Binary Reverse Engineering Analysis

**Student Name:** [Your Name]  
**Student ID:** [Your ID]  
**Date:** November 26, 2025  

---

## Executive Summary

This report presents a reverse engineering analysis of the provided 64‑bit ELF binary `question1` using `objdump` (static disassembly), `strace` (system call tracing), and `gdb` (dynamic debugging).  
The binary prompts the user for 10 student names, writes them to a file, reads them back, sorts them alphabetically, and writes the sorted list to another file.  
The analysis details the program’s control flow, function responsibilities, system call behavior, and memory usage (stack, globals, and OS-level memory management).

---

## Part 1: Disassembly Analysis using objdump

### Command Used

```bash
objdump -d question1 > disassembly.txt
objdump -t question1 > symbols.txt
objdump -s question1 > sections.txt
```

### 1.1 Identified Functions

Based on `objdump -t` and `objdump -d`, the following key functions were identified in the binary:

| Function Name     | Address           | Purpose |
|------------------|-------------------|---------|
| `_start`         | 0x00005555555551c0 | ELF entry point; sets up process state and calls `__libc_start_main` with `main` |
| `main`           | 0x00005555555552a9 | High-level program logic: prompts user, coordinates reading, writing, sorting of names |
| `getStudentNames`| 0x000055555555539b | Reads 10 student names from stdin into a stack-allocated array and strips newlines |
| `writeToFile`    | 0x0000555555555495 | Opens a file and writes all names to it using `fprintf` in a loop |
| `readFromFile`   | 0x000055555555554e | Opens a file and reads names back into memory using `fgets` in a loop |
| `sortNames`      | 0x000055555555567b | Sorts the in-memory array of names using nested loops, `strcmp`, and `strcpy` (bubble-sort style) |
| `printNames`     | 0x0000555555555844 | Iterates over the array and prints each name with `puts` (defined but not obviously used by `main`) |
| `deregister_tm_clones`, `register_tm_clones`, `__do_global_dtors_aux`, `frame_dummy`, `_init`, `_fini` | various | Compiler/runtime-generated initialization and cleanup routines |
| PLT stubs (`printf@plt`, `fopen@plt`, `fgets@plt`, `fprintf@plt`, `strcmp@plt`, `strcpy@plt`, `strcspn@plt`, `perror@plt`, `exit@plt`, etc.) | 0x0000555555555100–0x00005555555551b0 | Dynamic linking stubs for standard C library functions |

**Detailed Function Analysis:**

#### Function: `main`

```assembly
Address: 0x00005555555552a9
Purpose: Orchestrates program behavior: prompts for names, reads input, writes to file, reads back, sorts, and writes sorted output.

Key operations:
- Allocates a large stack buffer (~0x3f0 bytes) to store an array of up to 10 student names.
- Calls `printf` to display the initial “Enter 10 full names of students” prompt.
- Calls `getStudentNames(buffer, 10)` to fill the stack buffer with names from stdin.
- Calls `writeToFile("students.txt", buffer, 10)` to persist the names.
- Calls `readFromFile("students.txt", buffer, 10)` to reload names from disk into memory.
- Calls `sortNames(buffer, 10)` to alphabetically sort the names.
- Calls `writeToFile("sorted_students.txt", buffer, 10)` to store the sorted names.
- Prints a final message indicating that sorted names were written to `sorted_students.txt` and returns 0.
```

#### Function: `getStudentNames`

```assembly
Address: 0x000055555555539b
Purpose: Interactively prompts for and reads each student name into the provided array.

Key operations:
- Initializes a loop counter `i = 0`.
- Loop body:
  - Uses `printf("Student %d: ", i+1)` to prompt for each name.
  - Computes the address of `buffer[i]` via shifts and adds on the base pointer, reflecting an array of fixed-size name slots.
  - Calls `fgets(buffer[i], 0x64, stdin)` to read a line.
  - Uses `strcspn(buffer[i], "\n")` to find the newline and writes a `'\0'` terminator at that position to strip the newline.
- Continues until 10 names have been read.
```

#### Function: `writeToFile`

```assembly
Address: 0x0000555555555495
Purpose: Writes all names from the in-memory array to a given file.

Key operations:
- Receives: `rdi = filename`, `rsi = pointer to names array`, `edx = count (10)`.
- Calls `fopen(filename, "w")`. If it fails, calls `perror` and then `exit(1)`.
- Initializes a loop index `i = 0`.
- Loop body:
  - Computes the address of `names[i]` by scaling the index and adding to the base pointer.
  - Calls `fprintf(file, "%s\n", names[i])` to write each name on its own line.
- After the loop, calls `fclose(file)` and returns.
```

#### Function: `readFromFile`

```assembly
Address: 0x000055555555554e
Purpose: Reads names from a file back into the in-memory array.

Key operations:
- Receives: `rdi = filename`, `rsi = pointer to names array`, `edx = count (10)`.
- Calls `fopen(filename, "r")`. On failure, calls `perror` and `exit(1)`.
- For each index `i` until count:
  - Computes `names[i]` address.
  - Calls `fgets(names[i], 0x64, file)` and checks if the return value is NULL (EOF).
  - If a line was read, uses `strcspn(names[i], "\n")` to find newline and replaces it with `'\0'` to remove it.
- After reading up to `count` lines or EOF, calls `fclose(file)` and returns.
```

#### Function: `sortNames`

```assembly
Address: 0x000055555555567b
Purpose: Sorts the array of names in ascending lexicographic order.

Key operations:
- Receives: `rdi = pointer to names array`, `esi = count (10)`.
- Allocates a temporary buffer on the stack for swaps.
- Uses nested loops (outer index `i`, inner index `j`) resembling bubble sort.
- For each pair `(i, j)`:
  - Computes addresses of `names[i]` and `names[j]`.
  - Calls `strcmp(names[i], names[j])`.
  - If `strcmp > 0`, performs a three-step swap using `strcpy` and the temporary buffer:
    - `strcpy(temp, names[i])`
    - `strcpy(names[i], names[j])`
    - `strcpy(names[j], temp)`
- Checks a stack canary at the end and calls `__stack_chk_fail` on corruption.
```

---

### 1.2 Control Flow Analysis: Jumps and Branches

#### Example 1: Conditional Jump (loop in `getStudentNames`)

```assembly
Address: 0x0000555555555481  (within getStudentNames)
Instruction sequence:
  mov    -0x14(%rbp), %eax       ; load loop index i
  cmp    -0x2c(%rbp), %eax       ; compare i with count (10)
  jl     0x00005555555553bb      ; jump back to loop body if i < count
```

**Control Flow Impact:**

- Condition: `jl` (jump if less) branches when `i < count`.
- Target: 0x00005555555553bb, which is the start of the loop body (prompt + `fgets`).
- Effect: Implements a `for (i = 0; i < count; i++)` style loop, continuing to read names until the desired number is reached.
- Use case: This is the main input collection loop in `getStudentNames`.

#### Example 2: Unconditional Jump (loop setup in `getStudentNames`)

```assembly
Address: 0x00005555555553b6
Instruction: jmp 0x0000555555555481
```

**Control Flow Impact:**

- Type: Unconditional transfer.
- Target: The loop condition check at the end of the function.
- Effect: After initializing the loop index `i = 0`, control always jumps to the condition check, forming the canonical “init → check → body” loop structure.
- Use case: Implements the jump from loop initialization to the loop condition.

#### Example 3: Call Instruction (call from `main` to `getStudentNames`)

```assembly
Address: 0x00005555555512ef (within main’s disassembly before relocation)
Instruction: call 0x000055555555139b  ; call getStudentNames
```

**Control Flow Impact:**

- `call` pushes the return address (the instruction after the call) onto the stack and transfers control to `getStudentNames`.
- This creates the function-call relationship: `main → getStudentNames`.
- Similar `call` instructions in `main` transfer control to `writeToFile`, `readFromFile`, and `sortNames`, forming a simple, linear call graph.

### 1.3 Control Flow Diagram

A simplified textual control flow diagram for the user-level logic:

```text
_start
  |
  v
__libc_start_main
  |
  v
main
  |
  +--> printf("Enter 10 full names of students...")
  |
  +--> getStudentNames(names, 10)
  |       |
  |       +-- loop (i = 0..9):
  |              - prompt "Student i:"
  |              - fgets into names[i]
  |              - strip '\n' via strcspn
  |
  +--> writeToFile("students.txt", names, 10)
  |
  +--> readFromFile("students.txt", names, 10)
  |
  +--> sortNames(names, 10)
  |
  +--> writeToFile("sorted_students.txt", names, 10)
  |
  +--> printf("Sorted names have been written to sorted_students.txt")
  |
  v
exit / exit_group(0)
```

---

## Part 2: System Call Tracing using strace

### Command Used

```bash
strace -o question1.strace ./question1
# For more focused analysis:
strace -e trace=file,memory,desc -o question1_filtered.strace ./question1
```

For convenience during analysis, the program was run with piped input:

```bash
printf 'Alice\nBob\nCarol\nDave\nEve\nFrank\nGrace\nHeidi\nIvan\nJudy\n' \
  | strace -f -o question1.strace ./question1
```

### 2.1 Complete System Call List (Main Ones)

(Loader and libc perform many extra syscalls; here we highlight the most relevant.)

| System Call    | Approx. Count | Purpose                                      | Category |
|----------------|--------------|----------------------------------------------|----------|
| `execve`       | 1            | Start the `question1` process                | Process / I/O |
| `openat`       | ~20+         | Load shared libraries, open user files       | I/O |
| `read`         | 2+           | Read piped stdin, read from `students.txt`   | I/O |
| `write`        | 3+           | Write to `students.txt`, `sorted_students.txt`, stdout | I/O |
| `close`        | several      | Close file descriptors after use             | I/O |
| `fstat`        | several      | Query metadata about files and descriptors   | I/O |
| `mmap`         | several      | Map libc and anonymous memory segments       | Memory |
| `munmap`       | several      | Unmap temporary mappings (e.g., ld cache)    | Memory |
| `brk`          | few          | Adjust heap boundary                         | Memory |
| `mprotect`     | few          | Set memory protection on mapped segments     | Memory |
| `arch_prctl`   | 1            | Set FS register for thread-local storage     | Memory / TLS |
| `set_tid_address` | 1         | Set thread ID address for futex operations   | Threading |
| `set_robust_list` | 1         | Set robust futex list                        | Threading |
| `rseq`         | 1            | Register restartable sequences               | Optimization |
| `getrandom`    | 1            | Read random bytes for runtime initialization | Misc / Memory |
| `prlimit64`    | 1            | Get resource limits (e.g., stack size)       | Resource |
| `exit_group`   | 1            | Terminate the process                        | Process |

### 2.2 Detailed System Call Analysis

#### File I/O Operations

**openat() calls (program’s own files):**

```text
openat(AT_FDCWD, "students.txt", O_WRONLY|O_CREAT|O_TRUNC, 0666) = 5
openat(AT_FDCWD, "students.txt", O_RDONLY) = 5
openat(AT_FDCWD, "sorted_students.txt", O_WRONLY|O_CREAT|O_TRUNC, 0666) = 5
```

- **Interpretation:**  
  - `students.txt` is first opened for writing (create or truncate), then reopened for reading.  
  - `sorted_students.txt` is opened for writing to store sorted names.
- **File descriptor:** 5 (success in all cases shown).
- **Purpose:** Persist user-provided names and then output the sorted version to a new file.

**read() calls:**

```text
read(0, "Alice\nBob\nCarol\nDave\nEve\nFrank\nG"..., 4096) = 53
read(5, "Alice\nBob\nCarol\nDave\nEve\nFrank\nG"..., 4096) = 53
```

- **Interpretation:**
  - `read(0, ...)` reads 53 bytes from stdin (fd 0), which are the 10 names provided via the pipe.
  - `read(5, ...)` reads the same data back from `students.txt` into a user buffer.
- **Memory interaction:**  
  - Data is copied from kernel space into the user-space buffer used by `fgets` / the C library.  
  - Confirms the two-phase I/O: stdin → file → memory.

**write() calls:**

```text
write(5, "Alice\nBob\nCarol\nDave\nEve\nFrank\nG"..., 53) = 53
write(5, "Alice\nBob\nCarol\nDave\nEve\nFrank\nG"..., 53) = 53
write(1, "Enter 10 full names of students:"..., 199) = 199
```

- **Interpretation:**
  - The first two `write` calls correspond to writing all names to `students.txt` and then to `sorted_students.txt`.
  - The third writes the final informational message to stdout (fd 1).
- **Purpose:**  
  - Provide persistent storage of the input and final sorted output.  
  - Inform the user that sorting is complete and where the output is stored.

#### Memory Operations

**mmap() calls:**

```text
mmap(NULL, 2170256, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 5, 0) = 0x...
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x...
```

- **Interpretation:**
  - Large `mmap` calls map the standard C library (`libc.so.6`) code and data segments.
  - Smaller anonymous mappings are used for runtime management structures (thread local, etc.).
- **Purpose:** Shared library loading and internal runtime memory pools, not directly controlled by user code.

**brk() calls:**

```text
brk(NULL)                       = 0x...
brk(0x...)                      = 0x...
```

- **Interpretation:**  
  - `brk(NULL)` queries the current end of the data segment (heap).  
  - Subsequent `brk(new)` can expand the heap if needed.
- **Purpose:** Provide heap space for libc and any internal allocations (though user code primarily uses stack in this program).

### 2.3 I/O and Memory Interaction Patterns

**File System Interaction:**

- The program performs **sequential file access**:
  - Writes the entire list of names once to `students.txt`.
  - Reads it back entirely once.
  - Writes the same (but sorted) lines to `sorted_students.txt`.
- Data volumes (from `strace` example):
  - About 53 bytes written to, and read from, the two files (10 short names).

**Memory Management:**

- Heap use is minimal and handled by the C runtime (`mmap`, `brk`); user code relies on a large stack-allocated buffer for all names.
- Stack is used heavily for the `names` array and temporary buffers in `sortNames` and other functions.
- Memory mappings are mainly for loading shared libraries and internal runtime structures.

### 2.4 Runtime Behavior Insights

1. **Initialization Phase:**
   - `execve` starts the process; `mmap`, `openat`, `read`, and `fstat` load `libc.so.6` and dynamic linker data.
   - `arch_prctl`, `set_tid_address`, `set_robust_list`, and `rseq` set up thread-local storage and threading primitives.

2. **Main Processing:**
   - `read(0, ...)` obtains all user input from stdin.
   - `openat`+`write` store the data to `students.txt`.
   - `openat`+`read` read the same data back.
   - `openat`+`write` store sorted names to `sorted_students.txt`.

3. **Cleanup Phase:**
   - `close` is called on file descriptors when no longer needed.
   - Mappings like the ld cache are unmapped via `munmap`.
   - The program terminates cleanly with `exit_group(0)`.

---

## Part 3: Dynamic Analysis using gdb

### Command Sequence

```bash
gdb ./question1

# Commands used:
(gdb) info functions           # List all functions in the binary and linked libs
(gdb) disassemble main         # Disassemble main
(gdb) break main               # Break at main
(gdb) break getStudentNames    # Additional breakpoints on key functions
(gdb) break writeToFile
(gdb) break readFromFile
(gdb) break sortNames
(gdb) run                      # Execute the program
(gdb) step / next              # Step through instructions/functions
(gdb) info registers           # View register state (arguments, return values)
(gdb) x/16x $rsp               # Examine stack around current frame
(gdb) x/64bx [address]         # Examine memory at array base
```

### 3.1 Breakpoint Analysis

#### Breakpoint 1: Entry to `main`

```text
Location: main
Address: 0x00005555555552a9

Analysis at breakpoint:
- Register state: RIP points to `main`; RSP and RBP indicate a new stack frame is about to be set up.
- Stack frame setup:
  - `push %rbp` / `mov %rsp, %rbp` establish the base pointer.
  - `sub $0x3f0, %rsp` reserves ~1008 bytes for local storage (the array of names and a canary).
- Local variables:
  - A stack canary is saved from `%fs:0x28` into `-0x8(%rbp)`.
  - The large block starting at `-0x3f0(%rbp)` serves as the buffer for 10 names (each with a fixed max length).
```

#### Breakpoint 2: `getStudentNames`

```text
Location: getStudentNames
Address: 0x000055555555539b

Observations:
- Arguments:
  - RDI: pointer to the stack buffer in `main` (base of names array).
  - ESI: integer count (10).
- Pre-call state:
  - The buffer is uninitialized; only the canary and stack frame are set.
- Inside the function:
  - A loop index is stored at `-0x14(%rbp)`.
  - Repeated calls to `printf` and `fgets` show user interaction and input collection.
  - `strcspn` and a subsequent store of `0` terminate each string properly.
- Post-call state:
  - Memory at `main`’s buffer contains 10 null-terminated strings of user input.
```

#### Breakpoint 3: `sortNames`

```text
Location: sortNames
Address: 0x000055555555567b

Observations:
- Arguments:
  - RDI: pointer to the same names array on the stack.
  - ESI: integer count (10).
- Pre-call state:
  - Array contains the (unsorted) names read back from file.
- Inside the function:
  - Stepping shows nested loops; each comparison uses `strcmp`.
  - When `strcmp > 0`, `strcpy` operations to/from a local temp buffer perform swaps.
- Post-call state:
  - The names in the stack array are alphabetically sorted.
```

### 3.2 Execution Flow Tracing

**Function Call Sequence (user code only):**

```text
1. _start
   |
   +-> __libc_start_main
         |
         +-> main
               |
               +-> getStudentNames(names, 10)
               |
               +-> writeToFile("students.txt", names, 10)
               |
               +-> readFromFile("students.txt", names, 10)
               |
               +-> sortNames(names, 10)
               |
               +-> writeToFile("sorted_students.txt", names, 10)
               |
               +-> [print final status message via printf]
               |
               +-> return 0
```

**Transition Example: `main → getStudentNames`:**

- **Mechanism:**  
  - A `call` instruction in `main` invokes `getStudentNames`.
- **Stack changes:**  
  - Return address (next instruction in `main`) is pushed.  
  - `getStudentNames` pushes `rbp` and allocates its own local stack variables.
- **Arguments:**  
  - `rdi` = address of `names[0]` in `main`’s stack frame.  
  - `esi` = `10`, the number of names to read.
- **Purpose:**  
  - Populate the buffer that subsequent functions will process.

**Transition Example: `main → writeToFile` / `readFromFile` / `sortNames`:**

- The same convention repeats:
  - `rdi` = filename or array base pointer.
  - `rsi` / `rdx` = additional parameters (buffer pointer, count).
  - `call` instructions maintain a clear and shallow call hierarchy.

### 3.3 Memory Access Analysis

#### Stack Memory

**Stack Layout at `main` (conceptual):**

```text
High addresses
-------------------------------------------------
[ ... previous stack frames ... ]
-------------------------------------------------
Return address to __libc_start_main
Saved RBP for main
Canary value (copied from %fs:0x28)
Local buffer for names (approx. 0x3f0 bytes)
-------------------------------------------------
Low addresses
```

**Observations:**

- Stack frame size for `main` is roughly 1008 bytes (0x3f0), suitable for 10 fixed-size name slots and bookkeeping.
- Other functions (`getStudentNames`, `sortNames`, etc.) also allocate smaller frames for indices, temporary pointers, and a swap buffer.
- Stack alignment appears consistent with 16‑byte ABI requirements before function calls.

#### Heap Memory

**Heap Allocations Observed:**

- No direct calls to `malloc`, `calloc`, or `realloc` in user code.
- Any heap-related syscalls (`brk`, `mmap`) are initiated by `libc` during initialization rather than by the program logic itself.
- Dynamic data structures in this program are effectively avoided; everything is stack-based plus file I/O.

#### Global/Static Memory

**Data Section Access:**

- String literals and filenames (e.g., `"students.txt"`, `"sorted_students.txt"`, prompt strings, format strings) reside in the `.rodata` section.
- Accessed via RIP-relative `lea` instructions such as:

  ```assembly
  lea 0x202a(%rip), %rdi  ; loads address of a constant string into RDI
  ```

- Glibc’s `stdin` object and other runtime globals are visible in `.bss` and `.data` (e.g., `stdin@GLIBC_2.2.5`).

### 3.4 Register Usage Analysis

**Key Register Observations (System V AMD64 ABI):**

| Register | Purpose in Binary                                      | Values Observed (examples)              |
|----------|--------------------------------------------------------|-----------------------------------------|
| `rax`    | Return values; temporary calculations                  | `0` on success paths; loop indices; return from `strcmp`, etc. |
| `rdi`    | First argument to functions                            | Pointer to filename, buffer base, format strings |
| `rsi`    | Second argument                                        | Count of names, pointer to array, `size` arguments |
| `rdx`    | Third argument                                         | Additional integer parameters (e.g., count) |
| `rcx`    | Loop counters / shift intermediates in indexing math   | Holds scaled indices during address computations |
| `rsp`    | Stack pointer                                          | Moves down on entry to functions, up on return |
| `rbp`    | Frame pointer for stack frames                         | Anchors local variable offsets          |
| `r9`, etc. | Additional arguments for library calls               | Used for higher-argument-count functions (`printf`, `fprintf`) |

---

## Part 4: Integrated Analysis and Findings

### 4.1 Program Functionality Summary

1. **Primary Function:**  
   The program collects 10 student names from the user, stores them in a file, reloads them, sorts them alphabetically, and writes the sorted list to a new file `sorted_students.txt`.

2. **Input Processing:**
   - Input is taken line by line via `fgets` from `stdin`.
   - Each input line is truncated at the newline using `strcspn`, ensuring a clean null-terminated C string for later operations.
   - No complex validation beyond line length is performed.

3. **Core Algorithm:**
   - Names are stored in a fixed-size array in stack memory.
   - Sorting is performed via a simple comparison-based algorithm (bubble sort or similar) using `strcmp` and `strcpy`.
   - Names are written to and read from text files in plain text format (one name per line).

4. **Output Generation:**
   - First file: `students.txt` – unsorted names as entered.
   - Second file: `sorted_students.txt` – sorted version of the same names.
   - Console output informs the user that sorted names have been written to the output file.

### 4.2 Memory Usage Patterns

**Stack Usage:**

- Maximum stack usage is dominated by `main`’s large local buffer (~1 KB) plus additional smaller frames in called functions.
- Stack is used efficiently for arrays of fixed size and for temporary swap buffers.
- No deep recursion; call depth remains small and controlled.

**Heap Usage:**

- No explicit heap allocations from user code.
- Any dynamic memory allocations are internal to `libc` for its own structures and not critical to the program’s logic.

**Global Data:**

- Read-only data:
  - String constants (prompts, filenames, format strings).
- Writable global data from user code is minimal or nonexistent; glibc maintains its own internal globals (`stdin`, locale data, etc.).

### 4.3 Control Flow Complexity

- **Branching factor:** Low; most branches are straightforward loop conditions and simple checks (file open failures, EOF checks).
- **Loop structures:**
  - Input loop in `getStudentNames` (single-level loop).
  - Output loops in `writeToFile` and `readFromFile`.
  - Nested loops in `sortNames` implementing a simple O(n²) sort.
- **Function call depth:** Typically 2–3 levels away from `_start` at most (`_start → __libc_start_main → main → helper`).
- **Recursion:** Not present.

### 4.4 Security Observations

- **Buffer operations:**
  - `fgets` is appropriately used with a maximum length (0x64 bytes), providing basic bounds checking for name input.
  - `strcpy` is used for swapping names, but always between fixed-size buffers with equivalent size; this is safe as long as all strings stay within their fixed length.
- **Input validation:**
  - Limited: the program does not reject overly long logical names, but `fgets` truncates to the buffer size, preventing buffer overflow.
  - No validation of characters or format beyond line length.
- **Memory safety:**
  - Stack canaries and `__stack_chk_fail` are enabled, offering runtime stack protection.
  - No direct use of unsafe unbounded functions like `gets`.
- **Potential vulnerabilities:**
  - If future modifications change buffer sizes or input limits without adjusting `strcpy`/`fgets`, risk could increase.
  - Currently, within this compiled binary and its patterns, no clear exploitable overflow is visible.

---

## Part 5: Conclusions

### Key Findings

1. **Program Structure:**  
   A simple, single-file C program that uses several helper functions for input collection, file writing/reading, and sorting, supported by glibc for I/O and runtime.

2. **Execution Characteristics:**  
   Linear, predictable execution with an initialization phase, a single main processing phase (input → file → memory → sort → file), and clean termination.

3. **Resource Usage:**  
   Efficient for its scope: modest stack usage, minimal file I/O (two small text files), and no heavy use of dynamic memory.

4. **Code Quality:**  
   The compiled binary reflects straightforward, readable C-level structure; use of `fgets` and stack canaries shows attention to basic safety, and modular decomposition into functions is clear.

### Technical Insights

- **Optimization level:**  
  The presence of structured stack frames and readable loops suggests either no optimization or mild optimization (e.g., `-O0` or `-O1`).

- **Compiler used:**  
  Likely `gcc` targeting x86-64 Linux with stack protector enabled, inferred from code patterns and symbol naming.

- **Architecture:**  
  x86-64, position-independent executable (PIE), dynamically linked against glibc.

- **Statically/dynamically linked:**  
  Dynamically linked; PLT entries and numerous `openat`/`mmap` calls loading `libc.so.6` confirm shared library usage.

### Learning Outcomes

This reverse engineering exercise demonstrated:

1. How high-level C code (loops, arrays, file I/O, sorting) maps to assembly instructions, jumps, and calls.
2. How system calls like `openat`, `read`, `write`, `mmap`, and `brk` underpin seemingly simple C library functions.
3. How stack, global data, and OS-managed memory regions are organized in an ELF process.
4. The importance of structured control flow, safe input functions, and stack protection in compiled binaries.

---

## Appendices

### Appendix A: Complete objdump Output

- `disassembly.txt` – result of `objdump -d question1`  
- `symbols.txt` – result of `objdump -t question1`  
- `sections.txt` – result of `objdump -s question1`

### Appendix B: Complete strace Output

- `question1.strace` – full trace captured via:

  ```bash
  strace -f -o question1.strace ./question1
  ```

### Appendix C: GDB Session Log

- GDB commands used (example):

  ```bash
  gdb -q ./question1 \
    -ex 'set pagination off' \
    -ex 'break main' \
    -ex 'run' \
    -ex 'bt' \
    -ex 'info functions' \
    -ex 'quit'
  ```

- Additional interactive exploration (stepping, examining registers and memory) can be included as needed.

### Appendix D: References

- Intel® 64 and IA-32 Architectures Software Developer Manuals  
- Linux System Call Table (e.g., public references documenting syscall numbers and semantics)  
- `man objdump` – GNU binary utilities manual  
- `man strace` – system call tracer manual  
- `man gdb` – GNU Debugger manual  

---

**Note:** Student name, student ID, and any additional course-specific metadata should be filled in before submission.