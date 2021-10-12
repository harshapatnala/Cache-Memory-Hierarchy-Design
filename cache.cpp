
#include <iostream>
#include <cstdlib>
#include <cmath>
#include "cache.h"
using namespace std;

cache::cache(string name, unsigned long int cs, unsigned long int bs, unsigned long int a, unsigned long int vc_size) { //Constructor
    cache_size = cs;
    block_size = bs;
    assoc = a;
    cache_level = name;
    vc_blocks = vc_size;

    next_level_ptr = NULL;
    num_sets = (unsigned int)(cache_size)/(block_size*assoc);
    num_index_bits = (int)log2(num_sets);
    reads = read_misses = writes = write_misses = write_backs = mem_traffic= swaps = swap_requests = 0;	//Initialize all stats to zero

    index_extract = 0;
    for(int i=0; i< num_index_bits; i++) {
        index_extract <<= 1;
        index_extract |= 1;

    }
    cache_array = new cache_metadata* [num_sets];	//Create 2D Array based on num_sets and associativity.
    for(unsigned int i=0; i<num_sets; i++) {
        cache_array[i] = new cache_metadata[assoc];	//Initialize all metadata.
        for (unsigned int j=0; j<assoc; j++) {
            cache_array[i][j].tag=0;
            cache_array[i][j].valid_bit = false;
            cache_array[i][j].dirty_bit = false;
            cache_array[i][j].lru_counter = j;
        }
    }

    if(vc_blocks >0) {  //Create a 1D Array based on Number of Blocks (Fully Associative).
        vc_exists = true;
        victim_cache = new cache_metadata[vc_blocks];
        for(unsigned int i=0; i < vc_blocks; i++) {
            victim_cache[i].tag = 0;
            victim_cache[i].valid_bit = false;
            victim_cache[i].dirty_bit = false;
            victim_cache[i].lru_counter = i;
        }
    }
    else vc_exists = false;
}

bool cache::block_exists(unsigned long int set_number, unsigned long int tag_bits) { //Search if the block exists in cache
    for(unsigned int j=0; j <assoc; j++) {
        if(cache_array[set_number][j].valid_bit) {
            if(cache_array[set_number][j].tag == tag_bits) {
                return true;
            }
        }
    }
    return false;
}

cache_metadata* cache::get_victim_block(unsigned long int set_number) {
    unsigned int lru_way = 0;
    if (assoc >1) {
        for(unsigned int i=1; i < assoc; i++) {
            if(cache_array[set_number][i].lru_counter > cache_array[set_number][lru_way].lru_counter) {
                lru_way = i;
            }
        }
    }
    return &(cache_array[set_number][lru_way]);
}
unsigned int cache::get_victim_way(unsigned long int set_number) {
    unsigned int lru_way=0;
    if(assoc >1) {
        for (unsigned int i=1; i<assoc; i++) {
            if(cache_array[set_number][i].lru_counter > cache_array[set_number][lru_way].lru_counter) {
                lru_way = i;
            }
        }
    }
    return lru_way;
}
void cache::allocate_block(unsigned long int set_number, unsigned long int tag_bits, unsigned int way) {
    cache_array[set_number][way].tag = tag_bits;
    cache_array[set_number][way].valid_bit = true;
    cache_array[set_number][way].dirty_bit = false;

}

void cache::set_dirty_bit (unsigned long int set_number, unsigned int way) {
    cache_array[set_number][way].dirty_bit = true;
}


void cache::lru_update(unsigned long int set_number, unsigned int victim_way) {
    if(assoc >1) {
        for (unsigned int i = 0; i < assoc; i++) {
            if (i != victim_way) {
                if (cache_array[set_number][i].lru_counter < cache_array[set_number][victim_way].lru_counter) {
                    cache_array[set_number][i].lru_counter++;
                }
            }
        }
    }
    cache_array[set_number][victim_way].lru_counter = 0;
}

bool cache::block_vc_exists (unsigned long int block_address) {
    for(unsigned int i=0; i <vc_blocks; i++) {
        if (victim_cache[i].valid_bit) {
            if (victim_cache[i].tag == block_address) return true;
        }
    }
    return false;
}
cache_metadata* cache::get_vc_victim_block () {
    unsigned int lru_way=0;
    for (unsigned int i=1; i < vc_blocks; i++) {
        if(victim_cache[i].lru_counter > victim_cache[lru_way].lru_counter) lru_way=i;
    }
    return &(victim_cache[lru_way]);
}

unsigned int cache::get_vc_victim_way() {
    unsigned int lru_way=0;
    for(unsigned int i=1; i <vc_blocks; i++) {
        if(victim_cache[i].lru_counter > victim_cache[lru_way].lru_counter) lru_way = i;
    }
    return lru_way;
}
void cache::send_block_to_vc(unsigned long int vc_block_address, unsigned int way, bool db) {
    victim_cache[way].tag = vc_block_address;
    victim_cache[way].valid_bit = true;
    victim_cache[way].dirty_bit = db;
}

void cache::vc_lru_update(unsigned int way_num) {
    for(unsigned int i=0; i < vc_blocks; i++) {
        if(i != way_num) {
            if (victim_cache[i].lru_counter < victim_cache[way_num].lru_counter) {
                victim_cache[i].lru_counter++;
            }
        }
    }
    victim_cache[way_num].lru_counter = 0;
}

void cache::swap_blocks(unsigned long evict_block_address, unsigned long int vc_block_address) {
    unsigned long int temp_tag;
    bool temp_db = false;
    temp_tag = vc_block_address >> num_index_bits;
    unsigned int vc_way=0;
    for(unsigned int i=0; i <vc_blocks; i++) {
        if(victim_cache[i].tag == vc_block_address) {
            temp_db = victim_cache[i].dirty_bit;
            vc_way = i;
            break;
        }
    }
    unsigned int cache_way = 0;
    unsigned long int set_number = evict_block_address & index_extract;
    unsigned long int tag_bits = evict_block_address >> num_index_bits;

    for(unsigned int i=0; i <assoc; i++) {
        if(cache_array[set_number][i].tag == tag_bits) {
            cache_way = i;
            break;
        }
    }
    victim_cache[vc_way].tag = evict_block_address;
    victim_cache[vc_way].dirty_bit = cache_array[set_number][cache_way].dirty_bit;
    victim_cache[vc_way].valid_bit = true;

    cache_array[set_number][cache_way].tag = temp_tag;
    cache_array[set_number][cache_way].dirty_bit = temp_db;
    cache_array[set_number][cache_way].valid_bit = true;

}


void cache::read_request(unsigned long int block_address) {
    reads++;

    cache_metadata* victim_block;
    unsigned long int set_number = block_address & index_extract;
    unsigned long int tag_bits = block_address >> num_index_bits;
    unsigned long int victim_block_address;
    unsigned int victim_way;

    if(block_exists(set_number, tag_bits)) { //JUST UPDATE LRU IF CACHE HIT.
        unsigned int way=0;
        if (assoc >1) {
            for (unsigned int i = 0; i < assoc; i++) {
                if (cache_array[set_number][i].tag == tag_bits) way = i;
            }
        }
        lru_update(set_number, way);
    }
    else {  // CACHE MISS IF BLOCK DOESN'T EXIST.
        read_misses++;
        victim_block = get_victim_block(set_number);
        victim_way = get_victim_way(set_number);
        victim_block_address = ((victim_block->tag) << num_index_bits) | set_number;

        if (vc_exists) {  // WHEN VICTIM CACHE EXISTS
            if(victim_block->valid_bit) { // Check if block to be replaced/evicted at cache is Valid. If Valid check in Victim Cache for possible swap.
                swap_requests++;
                if(block_vc_exists(block_address)) {
                    swap_blocks(victim_block_address, block_address); //Swap with VC if requested block exists in VC. Swap the block to be evicted at L1 with requested block at VC.
                    swaps++;
                    unsigned int way = 0;
                    for (unsigned int i=0; i < vc_blocks; i++) {
                        if(victim_cache[i].tag == victim_block_address) {
                            way=i;
                            break;
                        }
                    }
                    vc_lru_update(way); //Update both L1 and VC LRUs.
                    lru_update(set_number, victim_way);
                }
                else {  //When requested block doesn't exist in VC.
                    cache_metadata* vc_victim_block;
                    unsigned int vc_victim_way;
                    vc_victim_block = get_vc_victim_block();
                    vc_victim_way = get_vc_victim_way();//Find the lru block in VC.
                    if(next_level_ptr != NULL) { // Perform requests and write backs at L2 if it exists.
                        if((vc_victim_block->valid_bit) && (vc_victim_block->dirty_bit)) { //Write back the LRU block in VC to L2 if it's valid and dirty.
                            write_backs++;
                            next_level_ptr->write_request((vc_victim_block->tag));
                        }
                        send_block_to_vc(victim_block_address, vc_victim_way, (victim_block->dirty_bit)); //Now allocate the replacing block at cache in VC, and update LRUs
                        vc_lru_update(vc_victim_way);
                        next_level_ptr->read_request(block_address); //Request the actual block from L2.
                        allocate_block(set_number, tag_bits, victim_way);   //Allocate the block at L1.
                        lru_update(set_number, victim_way);
                    }
                    else { //If L2 doesn't exist, then just allocate at L1, and send replacing block to VC (Replacing block is still valid).
                        if((vc_victim_block->valid_bit) && (vc_victim_block->dirty_bit)) {
                            write_backs++;
                            mem_traffic++;
                        }
                        send_block_to_vc(victim_block_address, vc_victim_way, (victim_block->dirty_bit));
                        vc_lru_update(vc_victim_way);
                        allocate_block(set_number, tag_bits, victim_way);
                        mem_traffic++;
                        lru_update(set_number, victim_way);
                    }
                }
            }
            else { // When block replacing at cache is invalid, then just allocate the block (No need to invoke VC) appropriately at L1 and/or L2.
                if(next_level_ptr != NULL) {
                    next_level_ptr->read_request(block_address);
                    allocate_block(set_number, tag_bits, victim_way);
                    lru_update(set_number, victim_way);
                }
                else {
                    allocate_block(set_number, tag_bits, victim_way);
                    lru_update(set_number, victim_way);
                    mem_traffic++;
                }
            }
        }

        else { //WHEN VC DOESN'T EXIST
            if (next_level_ptr != NULL) {
                if (victim_block->valid_bit & victim_block->dirty_bit) {
                    write_backs++;
                    next_level_ptr->write_request(victim_block_address);
                }
                next_level_ptr->read_request(block_address);
                allocate_block(set_number, tag_bits, victim_way);
                lru_update(set_number, victim_way);
            }
            else {
                if (victim_block->valid_bit && victim_block->dirty_bit) {
                    write_backs++;
                    mem_traffic++;
                }
                allocate_block(set_number, tag_bits, victim_way);
                lru_update(set_number, victim_way);
                mem_traffic++;
            }

        }
    }

}



void cache::write_request(unsigned long int block_address) {
    writes++;
    unsigned long int tag_bits = block_address >> num_index_bits;
    unsigned long int set_number = block_address & index_extract;
    unsigned long int victim_block_address;
    cache_metadata* victim_block;
    unsigned int victim_way;

    if(block_exists(set_number, tag_bits)) {
        unsigned int way=0;
        if(assoc >1) {
            for(unsigned int i=0; i < assoc; i++) {
                if(cache_array[set_number][i].tag == tag_bits) way=i;
            }
        }
        set_dirty_bit(set_number, way);
        lru_update(set_number, way);
    }
    else {
        write_misses++;
        victim_block = get_victim_block(set_number);
        victim_way = get_victim_way(set_number);
        victim_block_address = ((victim_block->tag) << num_index_bits) | set_number;
        if (vc_exists) { //WHEN VICTIM CACHE EXISTS.
            if (victim_block->valid_bit) { //Check if block to be evicted at cache is valid. If valid, then check in VC for possible swap.
                swap_requests++;
                if (block_vc_exists(block_address)) {
                    swap_blocks(victim_block_address, block_address);
                    swaps++;
                    unsigned int way = 0;
                    for (unsigned int i = 0; i < vc_blocks; i++) {
                        if (victim_cache[i].tag == victim_block_address) {
                            way = i;
                            break;
                        }
                    }
                    vc_lru_update(way);
                    lru_update(set_number, victim_way);
                    set_dirty_bit(set_number, victim_way);
                }
                else { // When requested block doesn't exist in VC.
                    cache_metadata *vc_victim_block;
                    unsigned int vc_victim_way;
                    vc_victim_block = get_vc_victim_block();
                    vc_victim_way = get_vc_victim_way();
                    if (next_level_ptr != NULL) { //Perform requests and write backs at L2 if it exists.
                        if (vc_victim_block->valid_bit && vc_victim_block->dirty_bit) {
                            write_backs++;
                            next_level_ptr->write_request((vc_victim_block->tag));
                        }
                        send_block_to_vc(victim_block_address, vc_victim_way, (victim_block->dirty_bit));
                        vc_lru_update(vc_victim_way);
                        next_level_ptr->read_request(block_address);
                        allocate_block(set_number, tag_bits, victim_way);
                        set_dirty_bit(set_number, victim_way);
                        lru_update(set_number, victim_way);
                    }
                    else {  //If L2 doesn't exist, then just allocate at L1 and send replacing block to VC (Replacing block is still valid).
                        if (vc_victim_block->valid_bit && vc_victim_block->dirty_bit) {
                            write_backs++;
                            mem_traffic++;
                        }
                        send_block_to_vc(victim_block_address, vc_victim_way, (victim_block->dirty_bit));
                        vc_lru_update(vc_victim_way);
                        allocate_block(set_number, tag_bits, victim_way);
                        mem_traffic++;
                        set_dirty_bit(set_number, victim_way);
                        lru_update(set_number, victim_way);
                    }
                }
            }
            else {  //When replacing block at cache is invalid, then just allocate block the block (No need to invoke VC) appropriately at L1 and/or L2.
                if (next_level_ptr != NULL) {
                    next_level_ptr->read_request(block_address);
                    allocate_block(set_number, tag_bits, victim_way);
                    set_dirty_bit(set_number, victim_way);
                    lru_update(set_number, victim_way);
                }
                else {
                    allocate_block(set_number, tag_bits, victim_way);
                    set_dirty_bit(set_number, victim_way);
                    lru_update(set_number, victim_way);
                    mem_traffic++;
                }
            }

        }
        else { // WHEN VC DOESN'T EXIST.
            if (next_level_ptr != NULL) {
                if (victim_block->valid_bit && victim_block->dirty_bit) {
                    write_backs++;
                    next_level_ptr->write_request(victim_block_address);
                }
                next_level_ptr->read_request(block_address);
                allocate_block(set_number, tag_bits, victim_way);
                set_dirty_bit(set_number, victim_way);
                lru_update(set_number, victim_way);
            }
            else {
                if (victim_block->valid_bit && victim_block->dirty_bit) {
                    write_backs++;
                    mem_traffic++;
                }
                allocate_block(set_number, tag_bits, victim_way);
                set_dirty_bit(set_number, victim_way);
                lru_update(set_number, victim_way);
                mem_traffic++;
            }
        }
    }
}

void cache::print_stats() {
    cout<< "===== "<<cache_level<<" contents =====\n";
    for(unsigned int i=0; i < num_sets; i++) {
        cout << dec << "set " << i << "  :\t";
        for (unsigned int j = 0; j < assoc; j++) {
            for (unsigned int k = 0; k < assoc; k++) {
                if (cache_array[i][k].lru_counter == j) {
                    cout << hex << cache_array[i][k].tag << " ";
                    if (cache_array[i][k].dirty_bit) cout << "D\t";
                    else cout <<" \t";
                }
            }
        }
        cout << '\n';
    }
    if(vc_exists) {
		cout <<'\n';
        cout << "===== VC contents =====" << '\n';
        cout << "set 0:" << '\t';
        for (unsigned int i = 0; i < vc_blocks; i++) {
            for (unsigned int j = 0; j < vc_blocks; j++) {
                if (victim_cache[j].lru_counter == i) {
                    cout << hex << victim_cache[j].tag << " ";
                    if (victim_cache[j].dirty_bit) cout << "D ";
                    else cout <<"  ";
                }
            }
        }
        cout <<'\n';
    }
	cout <<'\n';
}