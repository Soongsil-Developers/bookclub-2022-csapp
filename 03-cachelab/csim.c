#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>


/*
    getopt로 인자들 파싱
    파싱한 인자들로 가상 캐시 구조 생성
    Cache = [Set0, Set1, ..., SetA]    (A : set associativity)
    SetN  = [Line0, Line2, ..., LineE] (E : number of cache line per set)
    LineM = (valid_bit, tag, timestamp)

    m-1        s+b          b               0
         tag      set index    block offset
    Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>
    Options:
    -h         Print this help message.
    -v         Optional verbose flag.
    -s <num>   Number of set index bits.
    -E <num>   Number of lines per set.
    -b <num>   Number of block offset bits.
    -t <file>  Trace file.

    Examples:
    linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace
    linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace
*/

typedef struct {
  int tag;
  bool valid;
  int time; 
} line_t;

typedef struct {
  line_t *lines;
} set_t;

typedef struct {
  set_t *sets;
  size_t set_num;  
  size_t line_num;
} cache_t; 

cache_t cache = {};

int index_bits = 0; // s
int block_bits = 0; // b

size_t hit_count = 0;
size_t miss_count = 0;
size_t eviction_count = 0;

void simulate(int addr);
void update_cache(set_t *set, size_t line_no);

int main(int argc, char *argv[]) {
    //
    // Parse parameter
    //
    FILE *file = 0;
    for (int opt; (opt = getopt(argc, argv, "s:E:b:t:")) != -1;) {
        switch (opt) {
        case 's':
            index_bits = atoi(optarg); // Number of set index bits
            cache.set_num = 2 << index_bits; // Number of sets
            break;
        case 'E':
            cache.line_num = atoi(optarg); // Number of lines per set (set associativity)
            break;
        case 'b':
            block_bits = atoi(optarg); // Number of block bits
            break;
        case 't': // Input filename
            if (!(file = fopen(optarg, "r"))) { return 1; }
            break;
        default:
            // Unknown option
            return 1;
        }
    }
    if ((!index_bits) || (!cache.line_num) || 
        (!block_bits) || (!file)) { 
        return 1; 
    } 

    cache.sets = malloc(sizeof(set_t) * cache.set_num);
    for (int i = 0; i < cache.set_num; i++) {
        cache.sets[i].lines = calloc(sizeof(line_t), cache.line_num);
    } 
    char o;
    int addr;

    while (fscanf(file, " %c %x%*c%*d", &o, &addr) != EOF) {
    //the address field specifies a 64-bit hexadecimal memory access
        if (o == 'I') { 
        continue;
        }

        simulate(addr);
        if ('M' == o) { 
        simulate(addr); 
        } 
    }

    fclose(file);

    for (int i = 0; i < cache.set_num; i++) { 
        free(cache.sets[i].lines); 
    }
    free(cache.sets);

    printSummary(hit_count, miss_count, eviction_count);

    return 0;
}



void simulate(int addr) {
    int tag = ((addr >> (index_bits + block_bits)) & 0xffffffff);
    int set_index = ((addr>>block_bits)&(0x7fffffff >> (31-index_bits)));
    set_t *set = &cache.sets[set_index];

  /*check if cache hit*/
  for (int i = 0; i < cache.line_num; i++) {
    line_t* line = &set->lines[i];

    // Check if the cache line is valid
    if (!(line->valid)) { 
	continue; 
    }
    // Compare tag bits
    if ((line->tag) != tag) { 
	continue; 
    }

    /*cache hit*/
    hit_count++;
    update_cache(set, i); // i is the number in lines
    return;
  }
/**********************/

  /*it is cache miss*/
  miss_count++;

  /*check for cache emtpy line*/
  for (int i = 0; i < cache.line_num; i++) {
    line_t* line = &set->lines[i];

    if (line->valid) { 
	continue; 
    }

    line->valid = true;
    line->tag = tag;
    update_cache(set, i); //i is the number in lines 
    return;
  }
  /****************************/


  /*if it is neither cache hit or miss, we need to evict*/
  eviction_count++;



  /*look for least recently used cache line*/
  for (int i = 0; i < cache.line_num; i++) {
    line_t* line = &set->lines[i];

    if (line->time) { 
	continue; //if time is not zero, it is not LRU! move on!
    }

    line->valid = true;
    line->tag = tag;
    update_cache(set, i);
    return;
  }
  /********************************************/
}
void update_cache(set_t *set, size_t line_number) {
    line_t *line = &set->lines[line_number];

    for (int i = 0; i < cache.line_num; i++) {
        line_t *temp = &set->lines[i];
        if (!(temp->valid)) { 
        continue; 
        }
        if ((temp->time) <= (line->time)) { 
        continue; 
        }

        --(temp->time);
    }

    (line->time) = (cache.line_num - 1); 
}