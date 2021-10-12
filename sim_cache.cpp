#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace std;
#include "cache.h"
int main(int argc, char* argv[]) {

    if(argc !=8) {
    	printf("Error: Expected inputs:7 Given inputs: %d\n", argc-1);
	exit(EXIT_FAILURE);	
    }
    unsigned long int block_size = strtoul(argv[1], NULL, 10);
    unsigned long int l1_size = strtoul(argv[2], NULL, 10);
    unsigned long int l1_assoc = strtoul(argv[3], NULL, 10);
    unsigned long int vc_num_blocks = strtoul(argv[4], NULL, 10);
    unsigned long int l2_size = strtoul(argv[5], NULL, 10);
    unsigned long int l2_assoc = strtoul(argv[6], NULL, 10);

    int num_i = log2(block_size);
    FILE *FP;
    char *trace_file;
    trace_file = argv[7];

    FP = fopen(trace_file, "r");
    if (FP == NULL) {
        printf("ERROR: UNABLE TO OPEN FILE %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    char str[2];
    char rw;
    unsigned long int addr;
    unsigned long int b_addr;

    cache L1("L1", l1_size, block_size, l1_assoc, vc_num_blocks);
    if(l2_size >0) {
        cache L2("L2", l2_size, block_size, l2_assoc, 0);
        L1.next_level_ptr = &L2;

        while (fscanf(FP, "%s %lx", str, &addr) != EOF) {
            rw = str[0];
            if (rw == 'r') {
                b_addr = addr >> num_i;
                L1.read_request(b_addr);
            } else if (rw == 'w') {
                b_addr = addr >> num_i;
                L1.write_request(b_addr);
            }
        }

        printf("  ===== Simulator configuration =====\n"
               "  BLOCKSIZE:                        %lu\n"
               "  L1_SIZE:                          %lu\n"
               "  L1_ASSOC:                         %lu\n"
               "  VC_NUM_BLOCKS:                    %lu\n"
               "  L2_SIZE:                          %lu\n"
               "  L2_ASSOC:                         %lu\n"
               "  trace_file:                       %s\n"
               " \n", block_size, l1_size, l1_assoc, vc_num_blocks, l2_size, l2_assoc, trace_file);

        float swap_rate = (float) L1.swap_requests / (float) (L1.reads + L1.writes);
        float l1_miss_rate = (float) (L1.read_misses + L1.write_misses - L1.swaps) / (float) (L1.reads + L1.writes);
        float l2_miss_rate = (float) L2.read_misses / (float) L2.reads;

        L1.print_stats();
        L2.print_stats();

        printf("===== Simulation results =====\n"
               "a. number of L1 reads:             %u\n"
               "b. number of L1 read misses:       %u\n"
               "c. number of L1 writes:            %u\n"
               "d. number of L1 write misses:      %u\n"
               "e. number of swap requests:        %u\n"
               "f. swap request rate:              %.4f\n"
               "g. number of swaps:                %u\n"
               "h. combined L1+VC miss rate:       %.4f\n"
               "i. number writebacks from L1/VC:   %u\n"
               "j. number of L2 reads:             %u\n"
               "k. number of L2 read misses:       %u\n"
               "l. number of L2 writes:            %u\n"
               "m. number of L2 write misses:      %u\n"
               "n. L2 miss rate:                   %.4f\n"
               "o. number of writebacks from L2:   %u\n"
               "p. total memory traffic:           %u\n", L1.reads, L1.read_misses, L1.writes, L1.write_misses,
               L1.swap_requests, swap_rate, L1.swaps, l1_miss_rate, L1.write_backs, L2.reads, L2.read_misses, L2.writes,
               L2.write_misses, l2_miss_rate, L2.write_backs, (L1.mem_traffic + L2.mem_traffic));
    }
    else {
        while (fscanf(FP, "%s %lx", str, &addr) != EOF) {
            rw = str[0];
            if (rw == 'r') {
                b_addr = addr >> num_i;
                L1.read_request(b_addr);
            } else if (rw == 'w') {
                b_addr = addr >> num_i;
                L1.write_request(b_addr);
            }
        }

        printf("  ===== Simulator configuration =====\n"
               "  BLOCKSIZE:                        %lu\n"
               "  L1_SIZE:                          %lu\n"
               "  L1_ASSOC:                         %lu\n"
               "  VC_NUM_BLOCKS:                    %lu\n"
               "  L2_SIZE:                          %lu\n"
               "  L2_ASSOC:                         %lu\n"
               "  trace_file:                       %s\n"
               " \n", block_size, l1_size, l1_assoc, vc_num_blocks, l2_size, l2_assoc, trace_file);

        float swap_rate = (float) L1.swap_requests / (float) (L1.reads + L1.writes);
        float l1_miss_rate = (float) (L1.read_misses + L1.write_misses - L1.swaps) / (float) (L1.reads + L1.writes);
        float l2_miss_rate = 0;
        unsigned int l2_reads, l2_read_misses, l2_writes, l2_write_misses, l2_write_backs;
        l2_reads = l2_read_misses = l2_writes = l2_write_misses = l2_write_backs = 0;

        L1.print_stats();

        printf("===== Simulation results =====\n"
               "a. number of L1 reads:             %u\n"
               "b. number of L1 read misses:       %u\n"
               "c. number of L1 writes:            %u\n"
               "d. number of L1 write misses:      %u\n"
               "e. number of swap requests:        %u\n"
               "f. swap request rate:              %.4f\n"
               "g. number of swaps:                %u\n"
               "h. combined L1+VC miss rate:       %.4f\n"
               "i. number writebacks from L1/VC:   %u\n"
               "j. number of L2 reads:             %u\n"
               "k. number of L2 read misses:       %u\n"
               "l. number of L2 writes:            %u\n"
               "m. number of L2 write misses:      %u\n"
               "n. L2 miss rate:                   %.4f\n"
               "o. number of writebacks from L2:   %u\n"
               "p. total memory traffic:           %u\n", L1.reads, L1.read_misses, L1.writes, L1.write_misses,
               L1.swap_requests, swap_rate, L1.swaps, l1_miss_rate, L1.write_backs, l2_reads, l2_read_misses, l2_writes,
               l2_write_misses, l2_miss_rate, l2_write_backs, L1.mem_traffic);

    }
    return 0;
}
