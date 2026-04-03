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

On Windows PowerShell, use `.\cache_sim.exe` instead of `./cache_sim`.

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

### Example Commands

```bash
./cache_sim 4 2 samples/basic.txt
./cache_sim 4 2 samples/block_b4.txt --block-size 4
./cache_sim 4 2 samples/basic.txt --classify
./cache_sim 2 2 samples/l2_demo.txt --l2 8 4
./cache_sim 2 1 samples/capacity.txt --classify
./cache_sim 4 2 samples/basic.txt --block-size 2 --l2 8 4 --classify
```

## Output

- **`cache_sim_output`** (in the current working directory): **only** per-reference lines, one per line, in homework format. This matches the assignment (“each line … a single memory reference”).
- **Terminal (stdout)**: configuration banner, a short confirmation that the file was written, and a **--- Summary ---** block (hits/misses, optional L2 stats and 3C breakdown).

### Per-reference line formats

**L1 only**

```
<addr> : HIT
<addr> : MISS
```

**With L2** (only on L1 miss)

```
<addr> : MISS | L2: HIT
<addr> : MISS | L2: MISS
```

**With `--classify`** (only on L1 miss)

```
<addr> : MISS (COMPULSORY)
<addr> : MISS (CAPACITY)
<addr> : MISS (CONFLICT)
```

Flags can be combined (e.g. `MISS | L2: HIT (CONFLICT)`).

## Sample I/O (all cases)

Reference traces live under [`samples/`](samples/). The outputs below were produced by this implementation; percentages in the summary may show more or fewer decimal places depending on the platform.

---

### 1. Basic (matches homework handout)

**Input** — `samples/basic.txt` (same trace as `memory_reference_file` in the repo root):

```
1 3 5 1 3 1
```

**Command**

```bash
./cache_sim 4 2 samples/basic.txt
```

**`cache_sim_output`**

```
1 : MISS
3 : MISS
5 : MISS
1 : MISS
3 : MISS
1 : HIT
```

**Stdout (summary excerpt)**

```
--- Summary ---
Total references : 6
L1 Hits          : 1 (16.6667%)
L1 Misses        : 5 (83.3333%)
```

---

### 2. Multi-word blocks (`--block-size`)

**Input** — `samples/block_b4.txt`:

```
0 1 4 0
```

Words `0` and `1` fall in the same block when `block_size = 4`; `4` is the next block.

**Command**

```bash
./cache_sim 4 2 samples/block_b4.txt --block-size 4
```

**`cache_sim_output`**

```
0 : MISS
1 : HIT
4 : MISS
0 : HIT
```

**Stdout (summary excerpt)**

```
--- Summary ---
Total references : 4
L1 Hits          : 2 (50%)
L1 Misses        : 2 (50%)
```

---

### 3. Miss classification — compulsory + conflict (`--classify`)

**Input** — `samples/basic.txt`

**Command**

```bash
./cache_sim 4 2 samples/basic.txt --classify
```

**`cache_sim_output`**

```
1 : MISS (COMPULSORY)
3 : MISS (COMPULSORY)
5 : MISS (COMPULSORY)
1 : MISS (CONFLICT)
3 : MISS (CONFLICT)
1 : HIT
```

**Stdout (summary excerpt)**

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

---

### 4. Miss classification — compulsory + capacity (`--classify`)

**Input** — `samples/capacity.txt`:

```
0 2 4 0
```

**Command** — direct-mapped L1 with two sets (`2` entries, `1`-way): repeated distinct blocks map to the same set, so the final `0` misses even though a fully associative cache of the same total size would also miss (capacity).

```bash
./cache_sim 2 1 samples/capacity.txt --classify
```

**`cache_sim_output`**

```
0 : MISS (COMPULSORY)
2 : MISS (COMPULSORY)
4 : MISS (COMPULSORY)
0 : MISS (CAPACITY)
```

**Stdout (summary excerpt)**

```
--- Summary ---
Total references : 4
L1 Hits          : 0 (0%)
L1 Misses        : 4 (100%)
Miss Breakdown:
  Compulsory : 3
  Capacity   : 1
  Conflict   : 0
```

---

### 5. L1 + L2 (`--l2`)

**Input** — `samples/l2_demo.txt`:

```
0 1 2 0
```

L1 is small (`2` entries, `2`-way); L2 is larger. After filling both, block `0` can remain in L2 when evicted from L1, so the last access is an **L1 miss** and **L2 hit**.

**Command**

```bash
./cache_sim 2 2 samples/l2_demo.txt --l2 8 4
```

**`cache_sim_output`**

```
0 : MISS | L2: MISS
1 : MISS | L2: MISS
2 : MISS | L2: MISS
0 : MISS | L2: HIT
```

**Stdout (summary excerpt)**

```
--- Summary ---
Total references : 4
L1 Hits          : 0 (0%)
L1 Misses        : 4 (100%)
L2 Accesses      : 4
L2 Hits          : 1
L2 Misses        : 3
```

---

### 6. All optional features together

**Input** — `samples/basic.txt`

**Command**

```bash
./cache_sim 4 2 samples/basic.txt --block-size 2 --l2 8 4 --classify
```

With `block_size = 2`, words `1`, `3`, and `5` map to three distinct blocks; the later repeats hit in L1, so L2 is not queried on those hits.

**`cache_sim_output`**

```
1 : MISS | L2: MISS (COMPULSORY)
3 : MISS | L2: MISS (COMPULSORY)
5 : MISS | L2: MISS (COMPULSORY)
1 : HIT
3 : HIT
1 : HIT
```

**Stdout (summary excerpt)**

```
--- Summary ---
Total references : 6
L1 Hits          : 3 (50%)
L1 Misses        : 3 (50%)
L2 Accesses      : 3
L2 Hits          : 0
L2 Misses        : 3
Miss Breakdown:
  Compulsory : 3
  Capacity   : 0
  Conflict   : 0
```

---

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
