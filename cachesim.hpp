#ifndef CACHESIM_HPP
#define CACHESIM_HPP
#define CCOMPILER

#ifdef CCOMPILER
#include <stdint.h>
#else
#include <cstdint>
#endif

struct cache_stats_t {
    uint64_t accesses;
    uint64_t accesses_l2;
    uint64_t accesses_vc;
    uint64_t reads;
    uint64_t read_misses_l1;
    uint64_t read_misses_l2;
    uint64_t writes;
    uint64_t write_misses_l1;
    uint64_t write_misses_l2;
    uint64_t write_back_l1;
    uint64_t write_back_l2;
    uint64_t victim_hits;
    double   avg_access_time_l1;
};

void setup_cache(uint64_t c1, uint64_t b1, uint64_t s1, uint64_t v,
                 uint64_t c2, uint64_t b2, uint64_t s2);
void cache_access(char type, uint64_t arg, cache_stats_t* p_stats);
void complete_cache(cache_stats_t *p_stats);

static const uint64_t DEFAULT_C1 = 12;   /* 4KB Cache */
static const uint64_t DEFAULT_B1 = 5;    /* 32-byte blocks */
static const uint64_t DEFAULT_S1 = 3;    /* 8 blocks per set */
static const uint64_t DEFAULT_C2 = 15;   /* 32KB Cache */
static const uint64_t DEFAULT_B2 = 5;    /* 32-byte blocks */
static const uint64_t DEFAULT_S2 = 4;    /* 16 blocks per set */
static const uint64_t DEFAULT_V =  3;    /* 3 blocks in VC */

/** Argument to cache_access rw. Indicates a load */
static const char     READ = 'r';
/** Argument to cache_access rw. Indicates a store */
static const char     WRITE = 'w';

/** LRU maximum value. Used to avoid resetting the LRU block value as it gets higher than the maximum possible value*/
static const unsigned int LRU_MAX_VALUE =  255;
/** Value only used to get the first index where we could write data in the victim cache*/
static const unsigned int WRITABLE =  255;


/* ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
***********************
************************
*************************
** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/

/**  Block struture or structure of one block in one set */
struct block_struct {
    /** Indicates whether or not the cache block has been loaded with valid data. Set to 0 on power-up */
    unsigned int valid_bit : 1;
    /** Indicates whether the associated cache line has been changed since it was read in main memory */
    unsigned int dirty_bit : 1;
    /** Bits used for the LRU algorithm. The lowest value is the least recently used cache line */
    unsigned int LRU : 8; // If modified, also change the LRU max value
    /** Tag. May vary according to C1 and S1 (64-C1-S1). However, the highest possible value is 63, since offset >= 1
    Use Calloc or Malloc to set ... Not sure if can allocate 1-63 bits memory and use it correctly. Set to 63 for the moment Set to 64bits since all the rest is in 64 bits */
    unsigned long int tag : 64;
    /** Data elements. size 2^B1 bytes. */
    char *data;
};

/** Cache line structure */
struct cache_line_struct {
    /** Each cache line contains N1=2^S1 blocs. Each of those block are structure above */
    struct block_struct *blocks;
    /** Every other block will have the LRU set to 0 except for the last accessed block */
    unsigned long int last_accessed_block: 64;

};

/** Cache structure L1 and L2 */
struct cache_struct {
    /** A cache consist in 2^Index = 2^(C1-B1-S1) cache line*/
    struct cache_line_struct *cache_lines;
    /** number of cache lines. 2^Index may take values up to 64 bits (Impossible, but ...) */
    unsigned long int nb_cache_lines : 64;
    /** Number of cache blocks per line/Set */
    unsigned long int nb_cache_blocks_per_line : 64;
    /** Number of bytes per block */
    unsigned long int nb_bytes_per_data_block : 64;
};

/** Victim cache block structure */
struct victim_cache_block_struct {
    /** Tag. Size is 64 - B1. Highest value is 63 bits. Same conditions and problems as for "normal" cache*/
    unsigned long int tag : 64;
    /** Data 2^B1 bytes. */
    char *data;
    /** Bit used to Indicate if the line has been newly written. If set to 0, can write. If all line has this bit set to 1,
    remove the first line and set the rest of the bits to 0. This bit is used for FIFO. Helps avoiding moving all elements. */
    unsigned int writable: 1;

};

/** Victim cache line */
struct victim_cache_line_struct {
    /** Each line consist in only one cache block */
    struct victim_cache_block_struct *victim_cache_block;
};

/** Victim cache */
struct victim_cache_struct {
    /** The victim cache consist in V cache lines */
    struct victim_cache_line_struct *victim_cache_lines;
    /** V may takes values from 0 up to 4 */
    unsigned int nb_victim_cache_lines : 2;
    /** Number of blocks per line. Victim cache has only one block (set = 0) per cache line */
    unsigned int nb_victim_cache_blocks_per_line : 1;
    /** Number of bytes per block */
    unsigned long int nb_bytes_per_data_block : 64;
};

/** Masks used to get tag index and offset from memory addresses (Only used by main cache,
victim cache does not need this since data are place in main cache, not directly loaded from cache to CPU)*/

struct cache_mask_struct {
    //Maximum possibility is 64 bits = 8 bytes. Unsigned long int is 8 bytes
    // Set every elements to 63 bits to cover any mistakes, or to 64 bits to have the same bits lengths as defaults values (C, B, S)
    // 63 bits should be enough. If not, set to 64 bits
    //easier calculation if set to 64

    unsigned long int tag_mask : 64;
    unsigned long int index_mask : 64;
    unsigned long int offset_mask : 64;

    /** Number of 1 in the mask. maximum value is 64 -> 6 bits */
    unsigned int tag_mask_bit_length: 6;
    unsigned int index_mask_bit_length : 6;
    unsigned int offset_mask_bit_length : 6;
};

#endif /* CACHESIM_HPP */
