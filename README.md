# CS3339 Homework 3 – Cache Simulator

A configurable cache simulator written in C++ that supports set-associative caches with LRU replacement, multi-word blocks, multi-level caches (L1 + L2), and miss classification.

## Team Members

- Ly Huu Nhan To (Harry)

## How to Build

### Prerequisites

- `g++` (or another C++ compiler)

### Compile

```bash
g++ -o cache_sim cache_sim.cpp
```

This produces the executable `cache_sim` (on Windows you may get `cache_sim.exe`).

To remove the binary and output file, delete `cache_sim` / `cache_sim.exe` and `cache_sim_output` manually if needed.

## How to Run

### Basic Usage

```
./cache_sim <num_entries> <associativity> <memory_reference_file>
```

| Argument                | Description                                          |
| ----------------------- | ---------------------------------------------------- |
| `num_entries`           | Total number of cache entries (must be power of two) |
| `associativity`         | Ways per set (must evenly divide `num_entries`)      |
| `memory_reference_file` | Path to a file with space-separated word addresses   |

### Optional Flags (Extra Credit)

| Flag                     | Description                                        |
| ------------------------ | -------------------------------------------------- |
| `--block-size N`         | Set block size to N words (default: 1)             |
| `--l2 <entries> <assoc>` | Enable L2 cache with given size and associativity  |
| `--classify`             | Classify each miss as Compulsory/Capacity/Conflict |

### Examples

**Basic (matches sample from homework spec):**

```bash
./cache_sim 4 2 memory_reference_file
```

**With 4-word blocks:**

```bash
./cache_sim 8 2 memory_reference_file --block-size 4
```

**With L2 cache:**

```bash
./cache_sim 4 2 memory_reference_file --l2 16 4
```

**Full extra credit (all features):**

```bash
./cache_sim 4 2 memory_reference_file --block-size 4 --l2 16 4 --classify
```

## Output

The simulator writes results to a file named `cache_sim_output` in the current directory.

### Basic output format

```
<addr> : HIT
<addr> : MISS
```

### With L2 cache

```
<addr> : MISS | L2: HIT
<addr> : MISS | L2: MISS
```

### With miss classification

```
<addr> : MISS (COMPULSORY)
<addr> : MISS (CAPACITY)
<addr> : MISS (CONFLICT)
```

A summary section is appended after all per-reference lines:

```
--- Summary ---
Total references : 6
L1 Hits          : 1 (16.6667%)
L1 Misses        : 5 (83.3333%)
Miss Breakdown:
  Compulsory : 3
  Capacity   : 0
  Conflict   : 2
```

## Sample I/O

**Input file** (`memory_reference_file`):

```
1 3 5 1 3 1
```

**Invocation:**

```bash
./cache_sim 4 2 memory_reference_file
```

**Output file** (`cache_sim_output`):

```
1 : MISS
3 : MISS
5 : MISS
1 : MISS
3 : MISS
1 : HIT
```

## Design Notes

### Cache Organization

- `num_sets = num_entries / associativity`
- Address decomposition (for block size B):
  - `block_address = word_address / B`
  - `set_index = block_address % num_sets`
  - `tag = block_address / num_sets`

### Replacement Policy

LRU (Least Recently Used) is implemented with a per-entry timestamp. On a miss, the invalid slot (if any) or the slot with the oldest timestamp is evicted.

### Miss Classification (3C Model)

- **Compulsory**: First access to a block in the entire trace (cold miss).
- **Capacity**: Block was accessed before, but misses even in a fully-associative cache of the same total size.
- **Conflict**: Block was accessed before, would hit in a fully-associative cache – caused by limited associativity.

Classification is implemented by running a parallel simulation with a fully-associative shadow cache of equal capacity.

### Multi-level Cache

On an L1 miss, L2 is consulted. Data is brought into whichever level(s) experience a miss. L2 uses the same block size as L1.

## Known Bugs / Limitations

- Memory addresses are treated as **word addresses** (not byte addresses), consistent with the homework specification.
- All addresses must be non-negative integers.
- The simulator does not model write policies (write-back / write-through) or dirty bits; it is a read-only access trace simulator.
- L2 write-allocate / non-allocate policy is not modeled explicitly; L2 is always updated on an L1 miss.
