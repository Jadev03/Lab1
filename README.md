Lab 1: Linked List Concurrency Experiments
==========================================

This lab contains three C programs that implement a sorted singly linked list and perform concurrent operations using different synchronization mechanisms. Each program allows you to run experiments to measure the performance of the list under various operation mixes and thread counts.

## Files and Functionalities

### 1. `serialList.c`
**Functionality:**
- Implements a sorted singly linked list with `Member`, `Insert`, and `Delete` operations.
- All operations are performed serially (single-threaded, no concurrency).
- Measures the time taken to perform a mix of operations on the list.
- Allows the user to specify:
	- `n`: Initial number of unique nodes in the list
	- `m`: Number of operations to perform
	- Fractions for `Member`, `Insert`, and `Delete` operations (must sum to 1)
	- `runcount`: Number of times to repeat the experiment
- Prints the time taken for each run in CSV format.

**How to Compile:**
```sh
gcc -o serialList.exe serialList.c
```

**How to Run:**
```sh
./serialList.exe
```
or on Windows:
```sh
serialList.exe
```

---

### 2. `mutextList.c`
**Functionality:**
- Implements the same sorted linked list, but supports concurrent access using a single global mutex (`pthread_mutex_t`).
- Multiple threads perform operations on the list, but all operations are protected by the mutex (coarse-grained locking).
- User specifies:
	- `n`: Initial number of unique nodes
	- `m`: Number of operations
	- Fractions for `Member`, `Insert`, and `Delete` operations
	- `numThreads`: Number of threads to use
	- `runcount`: Number of times to repeat the experiment
- Each thread performs a share of the total operations.
- Prints the time taken for each run.

**How to Compile:**
```sh
gcc -o mutextList.exe mutextList.c -lpthread
```

**How to Run:**
```sh
./mutextList.exe
```
or on Windows:
```sh
mutextList.exe
```

---

### 3. `readwriteList.c`
**Functionality:**
- Implements the sorted linked list with concurrent access using a read-write lock (`pthread_rwlock_t`).
- `Member` operations acquire a read lock (allowing concurrent reads), while `Insert` and `Delete` acquire a write lock (exclusive access).
- Uses a mutex to protect random number generation for thread safety.
- User specifies:
	- `n`: Initial number of unique nodes
	- `m`: Number of operations
	- Fractions for `Member`, `Insert`, and `Delete` operations
	- `numThreads`: Number of threads
	- `runcount`: Number of times to repeat the experiment
- Each thread performs a share of the total operations.
- Prints the time taken for each run.

**How to Compile:**
```sh
gcc -o readwrite.exe readwriteList.c -lpthread
```

**How to Run:**
```sh
./readwrite.exe
```
or on Windows:
```sh
readwrite.exe
```

---

## Summary Table

| File             | Concurrency | Synchronization      | Notes                                  |
|------------------|-------------|---------------------|----------------------------------------|
| serialList.c     | No          | None                | Single-threaded baseline                |
| mutextList.c     | Yes         | Mutex (coarse)      | All operations protected by one mutex   |
| readwriteList.c  | Yes         | Read-Write Lock     | Concurrent reads, exclusive writes      |

---

## Notes
- All programs require user input for parameters at runtime.
- Output is printed in CSV format for easy analysis.
- Make sure to compile with `-lpthread` for the multithreaded programs.
- For best results, run the programs from a terminal/command prompt.

---

**Author:**
Lab1 Experiment Documentation
