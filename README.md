# Summative Project - Linux-Systems Programming
**Student Name:** Ramadhani Shafii Wanjenja    
**Date:** November 26, 2025

## Project Overview
This project demonstrates proficiency in systems programming through five comprehensive tasks:
1. ELF Binary Reverse Engineering Analysis
2. Assembly x86 Programming (Sensor Log Counter)
3. C Extension for Python (Temperature Statistics)
4. Multi-threaded Producer-Consumer System (Barista-Waiter)
5. Client-Server Exam Platform with Socket Programming

---

## Question 1: ELF Binary Analysis [6pts]

### Overview
Reverse engineering analysis of an ELF binary using `objdump`, `strace`, and `gdb`.

### Tools Required
- `objdump` (for disassembly)
- `strace` (for system call tracing)
- `gdb` (for debugging and execution analysis)

### Execution Steps

```bash
# 1. Disassemble the binary
objdump -d binary_file > disassembly.txt
objdump -t binary_file > symbols.txt

# 2. Trace system calls
strace -o strace_output.txt ./binary_file

# 3. Debug with gdb
gdb ./binary_file
```

### Expected Analysis Output
- **Control Flow:** Documented jumps, branches, and function calls
- **System Calls:** File operations, memory allocations, I/O operations
- **Memory Access:** Stack operations, heap allocations, global variables

### Deliverables
- `Q1_Analysis_Report.md` - Complete reverse engineering report
- `disassembly.txt` - objdump output
- `strace_output.txt` - System call trace

**Note:** Requires the actual ELF binary file to complete analysis.

---

## Question 2: Assembly Program - Sensor Log Counter [2pts]

### Overview
Assembly program that reads `sensor_log.txt` and counts total lines including empty ones.

### Files
- `sensor_counter.asm` - Main assembly source code
- `sensor_log.txt` - Input file (sample provided)

### Compilation and Execution

```bash
# Compile with NASM
nasm -f elf64 sensor_counter.asm -o sensor_counter.o

# Link
ld sensor_counter.o -o sensor_counter

# Run
./sensor_counter
```

### Expected Output
```
Total sensor readings: X
```
Where X is the total number of lines in sensor_log.txt

### Sample sensor_log.txt
```
23.5
24.1

22.8
25.3

23.9
```

### Technical Details
- **File Handling:** Uses Linux system calls (open, read, close)
- **Algorithm:** Counts newline characters + handles last line without newline
- **Memory:** 4KB buffer for file contents
- **Time Complexity:** O(n) where n is file size

---

## Question 3: C Extension for Python - Temperature Statistics [5pts]

### Overview
High-performance C extension providing statistical functions for temperature data analysis.

### Files
- `temp_stats.c` - C extension source code
- `setup.py` - Build configuration
- `test.py` - Demonstration script

### Compilation and Installation

```bash
# Build the extension
python3 setup.py build

# Install (may require sudo)
python3 setup.py install

# Or for development
python3 setup.py develop
```

### Testing

```bash
# Run test suite
python3 test.py
```

### Expected Output
```
========================================
Temperature Statistics C Extension - Test Suite
========================================

Sample Temperature Data (24-hour readings):
Temperatures: [22.5, 23.1, 22.8, ...]
Total readings: 24

------------------------------------------------------------
Test 1: count_readings()
------------------------------------------------------------
Result: 24 readings
Expected: 24 readings
Status: PASS

------------------------------------------------------------
Test 2: min_temp()
------------------------------------------------------------
Result: 20.8°C
Expected: 20.8°C
Status: PASS

[... additional tests ...]

All tests completed successfully!
```

### Functions Provided
1. **min_temp(list)** - Returns minimum temperature
   - Time: O(n), Space: O(1)
   
2. **max_temp(list)** - Returns maximum temperature
   - Time: O(n), Space: O(1)
   
3. **avg_temp(list)** - Returns average temperature
   - Time: O(n), Space: O(1)
   
4. **variance_temp(list)** - Returns sample variance
   - Time: O(n), Space: O(1)
   
5. **count_readings(list)** - Returns total count
   - Time: O(1), Space: O(1)

### Performance Benefits
- 10-100x faster than pure Python for large datasets
- Direct memory access with no interpreter overhead
- Optimized with -O3 compilation flag

---

## Question 4: Producer-Consumer - Barista-Waiter System [6pts]

### Overview
Multi-threaded simulation of a coffee shop order system with proper synchronization.

### Files
- `barista_waiter.c` - Complete implementation

### Compilation and Execution

```bash
# Compile with pthread library
gcc -pthread -o barista_waiter barista_waiter.c

# Run simulation
./barista_waiter
```

### Expected Output
```
========================================
Coffee Shop Order System Simulation
========================================
Configuration:
  - Max queue size: 8 drinks
  - Barista prep time: 4 seconds/drink
  - Waiter serve time: 3 seconds/drink
  - Total drinks to process: 15
========================================

[BARISTA] Preparing drink #1... (takes 4 seconds)
[BARISTA] Drink #1 ready! Added to queue. Queue size: 1/8
[WAITER] Picked up drink #1 from queue. Queue size: 0/8
[WAITER] Serving drink #1 to customer... (takes 3 seconds)
[BARISTA] Preparing drink #2... (takes 4 seconds)
[BARISTA] Drink #2 ready! Added to queue. Queue size: 1/8
[WAITER] Drink #1 served successfully! Total served: 1
...
[BARISTA] Queue is full (8 drinks). Waiting for space...
...
[BARISTA] Finished preparing all 15 drinks. Going on break.
[WAITER] All drinks served. Shift complete!

========================================
Simulation Complete!
========================================
Summary:
  - Drinks prepared: 15
  - Drinks served: 15
  - Final queue size: 0
========================================
```

### Synchronization Mechanisms
- **Mutex:** `queue_mutex` - Protects shared queue access
- **Condition Variables:**
  - `queue_not_full` - Signals when space becomes available
  - `queue_not_empty` - Signals when drinks are ready
  
### Thread Safety Features
- No race conditions
- Proper wait/signal patterns
- Atomic queue operations
- Deadlock prevention

---

## Question 5: Client-Server Exam System [6pts]

### Overview
TCP-based online examination platform supporting up to 4 concurrent students with authentication and real-time feedback.

### Files
- `exam_server.c` - Central server implementation
- `exam_client.c` - Student client program

### Compilation

```bash
# Compile server
gcc -pthread -o exam_server exam_server.c

# Compile client
gcc -o exam_client exam_client.c
```

### Execution

**Terminal 1 - Start Server:**
```bash
./exam_server
```

**Terminals 2-5 - Start Clients:**
```bash
./exam_client
```

### Expected Output

**Server Output:**
```
========================================
Online Examination Platform - Server
========================================
Configuration:
  - Port: 8080
  - Max concurrent clients: 4
  - Number of questions: 4
========================================

[SERVER] Listening on port 8080...

[SERVER] New connection from 127.0.0.1:54321
[SERVER] Student student_001 authenticated successfully
[SERVER] Sent question 1 to student_001
[SERVER] student_001 answered question 1: B (Correct)
[SERVER] Sent question 2 to student_001
[SERVER] student_001 answered question 2: A (Incorrect)
...
[SERVER] student_001 completed exam. Score: 3/4
[SERVER] Client student_001 disconnected
```

**Client Output:**
```
========================================
Online Examination Platform - Client
========================================

Connecting to exam server...
Connected to server successfully!

Enter your username (e.g., student_001): student_001

AUTH_SUCCESS:Welcome student_001!

=== Active Students ===
1. student_001
2. student_002
=======================

Starting exam...
========================================

Question 1:

=== EXAM QUESTION ===
What is the time complexity of binary search?
A) O(n)
B) O(log n)
C) O(n^2)
D) O(1)
====================

Your answer (A/B/C/D): B

Server: Correct! The answer is B

----------------------------------------

[... more questions ...]

=== EXAM COMPLETE ===
Your Score: 3/4
Exam session ended. Thank you, student_001.
====================

========================================
Exam session ended. Thank you, student_001!
========================================
```

### Communication Protocol
1. **Authentication:** `USERNAME:student_id` → `AUTH_SUCCESS:Welcome!`
2. **Question Delivery:** Server sends formatted question
3. **Answer Submission:** `ANSWER:A` → `Server: Correct!` or `Server: Incorrect.`
4. **Session End:** Final score and completion message

### Concurrency Features
- Thread-per-client model
- Mutex-protected client list
- Independent client sessions
- Secure authentication check
- Prevents unauthorized access

### Security Features
- Username-based authentication
- Active user tracking
- Unauthorized user prevention
- Graceful disconnection handling

---

## General Compilation Requirements

### Prerequisites
```bash
# GCC compiler
gcc --version

# NASM assembler (for Question 2)
nasm --version

# Python 3 development headers (for Question 3)
sudo apt-get install python3-dev

# GNU Debugger
gdb --version
```

### All Programs
```bash
# Question 2
nasm -f elf64 sensor_counter.asm -o sensor_counter.o
ld sensor_counter.o -o sensor_counter

# Question 3
python3 setup.py build && python3 setup.py install

# Question 4
gcc -pthread -o barista_waiter barista_waiter.c

# Question 5
gcc -pthread -o exam_server exam_server.c
gcc -o exam_client exam_client.c
```

---

## Testing Checklist

- [ ] Question 1: ELF binary analyzed with all three tools
- [ ] Question 2: Assembly program counts lines correctly
- [ ] Question 3: All 5 C functions work and test.py passes
- [ ] Question 4: Simulation runs without deadlock or race conditions
- [ ] Question 5: Multiple clients can connect and take exam simultaneously

---

## Known Issues and Limitations

1. **Question 2:** Maximum file size 4KB (can be increased by modifying buffer size)
2. **Question 3:** Requires Python 3.6+
3. **Question 5:** Server runs on localhost (127.0.0.1) - modify for network use

---

## References

- Linux System Call Documentation: `man 2 syscall_name`
- Python C API: https://docs.python.org/3/c-api/
- POSIX Threads: `man pthreads`
- Socket Programming: `man 7 socket`

---

## Contact

For questions or issues:
- Email: r.wanjenja@alustudent.com


**Submission Date:** November 26, 2025