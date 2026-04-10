/**
 * CS3339 Homework 3 - Cache Simulator (implementation)
 *
 * Usage:
 *   ./cache_sim <num_entries> <associativity> <memory_reference_file>
 *               [--block-size N] [--l2 <entries> <assoc>] [--classify]
 */

#include "cache_sim.h"

// ============================================================
// Cache
// ============================================================

Cache::Cache(int num_entries, int associativity, int block_size)
    : num_entries_(num_entries),
      associativity_(associativity),
      block_size_(block_size),
      num_sets_(num_entries / associativity),
      sets_(num_entries / associativity, std::vector<CacheEntry>(associativity)),
      global_time_(0)
{
    assert(num_entries > 0);
    assert(associativity > 0);
    assert(block_size > 0);
    assert(num_entries % associativity == 0);
}

bool Cache::access(int word_addr)
{
    int block_addr = word_addr / block_size_;
    int set_index = block_addr % num_sets_;
    int tag = block_addr / num_sets_;

    ++global_time_;
    std::vector<CacheEntry> &set = sets_[set_index];

    for (CacheEntry &entry : set)
    {
        if (entry.valid && entry.tag == tag)
        {
            entry.lru_time = global_time_;
            return true;
        }
    }

    int victim = findVictim(set);
    set[victim].valid = true;
    set[victim].tag = tag;
    set[victim].lru_time = global_time_;

    return false;
}

void Cache::reset()
{
    for (auto &set : sets_)
        for (auto &e : set)
            e = CacheEntry{};
    global_time_ = 0;
}

int Cache::numEntries() const { return num_entries_; }
int Cache::associativity() const { return associativity_; }
int Cache::numSets() const { return num_sets_; }
int Cache::blockSize() const { return block_size_; }

int Cache::findVictim(const std::vector<CacheEntry> &set)
{
    for (int i = 0; i < (int)set.size(); ++i)
        if (!set[i].valid)
            return i;

    int victim = 0;
    for (int i = 1; i < (int)set.size(); ++i)
        if (set[i].lru_time < set[victim].lru_time)
            victim = i;
    return victim;
}

// ============================================================
// Miss classification (3C)
// ============================================================

std::vector<MissType> classifyMisses(
    const std::vector<int> &refs,
    const std::vector<bool> &actual_hits,
    int num_entries,
    int block_size)
{
    Cache fa(num_entries, num_entries, block_size);

    std::unordered_set<int> seen_blocks;
    std::vector<MissType> types;
    types.reserve(refs.size());

    for (int i = 0; i < (int)refs.size(); ++i)
    {
        bool fa_hit = fa.access(refs[i]);
        int block_addr = refs[i] / block_size;
        bool first_time = (seen_blocks.find(block_addr) == seen_blocks.end());

        if (actual_hits[i])
        {
            types.push_back(MissType::HIT);
        }
        else if (first_time)
        {
            types.push_back(MissType::COMPULSORY);
        }
        else if (!fa_hit)
        {
            types.push_back(MissType::CAPACITY);
        }
        else
        {
            types.push_back(MissType::CONFLICT);
        }

        seen_blocks.insert(block_addr);
    }
    return types;
}

const char *missTypeStr(MissType mt)
{
    switch (mt)
    {
    case MissType::HIT:
        return "HIT";
    case MissType::COMPULSORY:
        return "COMPULSORY";
    case MissType::CAPACITY:
        return "CAPACITY";
    case MissType::CONFLICT:
        return "CONFLICT";
    }
    return "UNKNOWN";
}

// ============================================================
// main
// ============================================================

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <num_entries> <associativity> <memory_reference_file>"
                  << " [--block-size N] [--l2 <entries> <assoc>] [--classify]\n";
        return 1;
    }

    int num_entries = std::stoi(argv[1]);
    int associativity = std::stoi(argv[2]);
    std::string ref_file = argv[3];

    int block_size = 1;
    bool use_l2 = false;
    int l2_entries = 0, l2_assoc = 0;
    bool classify = false;

    for (int i = 4; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--block-size" && i + 1 < argc)
        {
            block_size = std::stoi(argv[++i]);
        }
        else if (arg == "--l2" && i + 2 < argc)
        {
            use_l2 = true;
            l2_entries = std::stoi(argv[++i]);
            l2_assoc = std::stoi(argv[++i]);
        }
        else if (arg == "--classify")
        {
            classify = true;
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if (num_entries <= 0 || associativity <= 0 || num_entries % associativity != 0)
    {
        std::cerr << "Error: associativity must evenly divide num_entries.\n";
        return 1;
    }
    if (block_size <= 0)
    {
        std::cerr << "Error: block-size must be a positive integer.\n";
        return 1;
    }
    if (use_l2 && (l2_entries <= 0 || l2_assoc <= 0 || l2_entries % l2_assoc != 0))
    {
        std::cerr << "Error: invalid L2 cache parameters.\n";
        return 1;
    }

    std::ifstream fin(ref_file);
    if (!fin.is_open())
    {
        std::cerr << "Error: cannot open file \"" << ref_file << "\".\n";
        return 1;
    }
    std::vector<int> refs;
    {
        int addr;
        while (fin >> addr)
            refs.push_back(addr);
    }
    fin.close();

    if (refs.empty())
    {
        std::cerr << "Error: no memory references found in \"" << ref_file << "\".\n";
        return 1;
    }

    Cache l1(num_entries, associativity, block_size);
    Cache *l2 = use_l2 ? new Cache(l2_entries, l2_assoc, block_size) : nullptr;

    std::cout << "=== Cache Simulator Configuration ===\n";
    std::cout << "L1: " << num_entries << " entries, "
              << associativity << "-way associative, "
              << (num_entries / associativity) << " sets, "
              << "block size = " << block_size << " word(s)\n";
    if (use_l2)
        std::cout << "L2: " << l2_entries << " entries, "
                  << l2_assoc << "-way associative, "
                  << (l2_entries / l2_assoc) << " sets, "
                  << "block size = " << block_size << " word(s)\n";
    std::cout << "Total references: " << refs.size() << "\n";
    std::cout << "======================================\n\n";

    std::vector<bool> l1_hits(refs.size(), false);
    std::vector<bool> l2_hits(refs.size(), false);

    for (int i = 0; i < (int)refs.size(); ++i)
    {
        l1_hits[i] = l1.access(refs[i]);
        if (!l1_hits[i] && l2)
            l2_hits[i] = l2->access(refs[i]);
    }

    std::vector<MissType> miss_types;
    if (classify)
        miss_types = classifyMisses(refs, l1_hits, num_entries, block_size);

    const std::string out_file = "cache_sim_output";
    std::ofstream fout(out_file);
    if (!fout.is_open())
    {
        std::cerr << "Error: cannot create output file \"" << out_file << "\".\n";
        delete l2;
        return 1;
    }

    for (int i = 0; i < (int)refs.size(); ++i)
    {
        fout << refs[i] << " : ";
        if (l1_hits[i])
        {
            fout << "HIT";
        }
        else
        {
            fout << "MISS";
            if (use_l2)
                fout << " | L2: " << (l2_hits[i] ? "HIT" : "MISS");
            if (classify)
                fout << " (" << missTypeStr(miss_types[i]) << ")";
        }
        fout << "\n";
    }

    fout.close();

    std::cout << "Per-reference lines written to \"" << out_file << "\".\n";

    int l1_hit_count = 0;
    int l1_miss_count = 0;
    for (bool h : l1_hits)
        h ? ++l1_hit_count : ++l1_miss_count;

    // Summary on stdout only; cache_sim_output matches HW3 (one line per reference).
    std::cout << "\n--- Summary ---\n";
    std::cout << "Total references : " << refs.size() << "\n";
    std::cout << "L1 Hits          : " << l1_hit_count
              << " (" << (100.0 * l1_hit_count / refs.size()) << "%)\n";
    std::cout << "L1 Misses        : " << l1_miss_count
              << " (" << (100.0 * l1_miss_count / refs.size()) << "%)\n";

    if (use_l2)
    {
        int l2_hit_count = 0;
        for (bool h : l2_hits)
            if (h)
                ++l2_hit_count;
        int l2_miss_count = l1_miss_count - l2_hit_count;

        std::cout << "L2 Accesses      : " << l1_miss_count << "\n";
        std::cout << "L2 Hits          : " << l2_hit_count << "\n";
        std::cout << "L2 Misses        : " << l2_miss_count << "\n";
    }

    if (classify)
    {
        int n_comp = 0, n_cap = 0, n_conf = 0;
        for (int i = 0; i < (int)refs.size(); ++i)
        {
            if (!l1_hits[i])
            {
                switch (miss_types[i])
                {
                case MissType::COMPULSORY:
                    ++n_comp;
                    break;
                case MissType::CAPACITY:
                    ++n_cap;
                    break;
                case MissType::CONFLICT:
                    ++n_conf;
                    break;
                default:
                    break;
                }
            }
        }
        std::cout << "Miss Breakdown:\n";
        std::cout << "  Compulsory : " << n_comp << "\n";
        std::cout << "  Capacity   : " << n_cap << "\n";
        std::cout << "  Conflict   : " << n_conf << "\n";
    }

    delete l2;
    return 0;
}
