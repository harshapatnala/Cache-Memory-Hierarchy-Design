#ifndef PROJ1_CACHE_H
#define PROJ1_CACHE_H
#include <cstdio>
#include <iostream>
#include <cstdlib>
using namespace std;

typedef struct {
    bool valid_bit;	//Valid bit
    unsigned int tag;	//Tag
    unsigned int lru_counter;	//Counter
    bool dirty_bit;	//Dirty bit

}cache_metadata;

class cache {

public:
    std::string cache_level;
    unsigned int cache_size, block_size, assoc, num_sets, vc_blocks; //Cache parameters
    unsigned int index_extract;
    unsigned int reads, writes, read_misses, write_misses, write_backs, mem_traffic, swaps, swap_requests; //Cache stats
    int num_index_bits;
    bool vc_exists;
    cache_metadata** cache_array; // Cache Contents
    cache_metadata* victim_cache; //Victim Cache

    cache* next_level_ptr;

    cache(string , unsigned long int, unsigned long int, unsigned long int, unsigned long int); //Declare constructor
    virtual ~cache() {delete cache_array;}	//Destructor


    //Methods
    bool block_exists(unsigned long int set_number, unsigned long int tag_bits);
    cache_metadata* get_victim_block (unsigned long int set_number);
    unsigned int get_victim_way(unsigned long int set_number);
    void allocate_block(unsigned long int set_number, unsigned long int tag_bits, unsigned int way);
    bool block_vc_exists(unsigned long int block_address);
    void swap_blocks(unsigned long int evict_block_address, unsigned long int vc_block_address);
    cache_metadata* get_vc_victim_block();
    unsigned int get_vc_victim_way();
    void send_block_to_vc(unsigned long int vc_block_address, unsigned int way, bool bit);
    void vc_lru_update(unsigned int way_num);
    void lru_update(unsigned long int set_number, unsigned int victim_way);
    void read_request(unsigned long int block_address);
    void write_request(unsigned long int block_address);
    void set_dirty_bit(unsigned long int set_number, unsigned int way);
    void print_stats();
};

#endif //PROJ1_CACHE_H
