/**
 * CS3339 Homework 3 - Cache Simulator (header)
 *
 * Set-associative cache with LRU, optional multi-word blocks,
 * L2 hierarchy and 3C miss classification (see cache_sim.cpp).
 */

#ifndef CACHE_SIM_H_
#define CACHE_SIM_H_

#include <vector>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

struct CacheEntry {
    bool valid    = false;
    int  tag      = -1;
    int  lru_time = 0;
};

class Cache {
public:
    Cache(int num_entries, int associativity, int block_size = 1);

    bool access(int word_addr);
    void reset();

    int numEntries() const;
    int associativity() const;
    int numSets() const;
    int blockSize() const;

private:
    int num_entries_;
    int associativity_;
    int block_size_;
    int num_sets_;
    std::vector<std::vector<CacheEntry>> sets_;
    int global_time_;

    static int findVictim(const std::vector<CacheEntry>& set);
};

enum class MissType { HIT, COMPULSORY, CAPACITY, CONFLICT };

std::vector<MissType> classifyMisses(
    const std::vector<int>&  refs,
    const std::vector<bool>& actual_hits,
    int num_entries,
    int block_size);

const char* missTypeStr(MissType mt);

#endif  // CACHE_SIM_H_
