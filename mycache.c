#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MISSPENALTY 100

struct d_cache{
    int tag;
    int valid;
    int time;
    int dirty;
    int wrmode;
    int evmode;
};

int l_total, l_miss, s_total, s_miss;
int time_count;

struct d_cache *dp;

void simulation(int c_size, int b_size, int assoc, int dir, int nomatch);
void read_data(int addr, int c_size, int b_size, int assoc);
void write_data(int addr, int c_size, int b_size, int assoc);
int evict(int set, int assoc, int mode);

char* filename;
void error() {
    printf("error!\n");
    abort();
}

int main(char* argc, char* argv[])
{
    if (argv[6]==NULL) error();
    int set = atoi(argv[1]);
    int block = atoi(argv[2]);
    int size = atoi(argv[3]);
    int wrmode;
    int nomatch;

    // write-through = 0
    // write-back = 1
    if (!strcmp(argv[5], "write-through")) {
        if (!strcmp(argv[4], "no-write-allocate")) wrmode = 0;
        else error();
    }
    if (!strcmp(argv[5], "write-back")) {
        if (!strcmp(argv[4], "write-allocate")) wrmode =1;
        else error();
    }
    // fifo = 0
    // lru = 1
    if (!strcmp(argv[6], "fifo")) nomatch = 0;
    else if (!strcmp(argv[6], "lru")) nomatch = 1;
    else error();
    
    simulation(set, block, size, wrmode, nomatch);
}

void simulation(int c_size, int b_size, int assoc, int wrmode, int nomatch) {

    l_total=l_miss=s_total=s_miss=0;
    char mode;
    int addr;
    int num = c_size/b_size;
    int cycle;

    dp = (struct d_cache*)calloc(num, sizeof(struct d_cache));
    dp->wrmode = wrmode;
    dp->evmode = nomatch;

    for (int i=0; i<520000; i++) {
        scanf("%c %x", &mode, &addr);
        if (mode == 'l') {
            read_data(addr, c_size, b_size, assoc);
            l_total++;
        }
        else if (mode == 's') {
            write_data(addr, c_size, b_size, assoc);
            s_total++;          
        }
        time_count++;
    }

    free(dp);
    cycle = (l_total-l_miss) + (s_total-s_miss) + MISSPENALTY*(l_miss+s_miss);
    
    printf("Total loads: %d\n", l_total);
    printf("Total stores: %d\n", s_total);
    printf("Load hits: %d\n", l_total-l_miss); 
    printf("Load misses: %d\n", l_miss);
    printf("Store hits: %d\n", s_total-s_miss);
    printf("Store misses: %d\n", s_miss);
    printf("Total cycles: %d\n", cycle);
}

void read_data(int addr, int c_size, int b_size, int assoc){
    int num_of_sets, set;
    int i, ev=0, avail=10;
    struct d_cache *p;
    
    num_of_sets = c_size/(b_size*assoc);
    set = (addr/b_size) % num_of_sets;
    
    for(i=0; i<assoc; i++){
        p = &dp[set*assoc + i];
        if(p->valid==1 && p->tag==(addr/b_size)/num_of_sets){
            p->time = time_count;
            return;
        }
        else if(p->valid==0){
            if(i < avail)
                avail = i;
        }
    }
    l_miss++;
    if(avail==10){
        ev = evict(set, assoc, p->evmode);
        p = &dp[set*assoc + ev];
        
        if(p->dirty)
            l_total++;
        
        p->valid = 1;
        p->time = time_count;
        p->tag = (addr/b_size)/num_of_sets;
        p->dirty = 0;
        
    }
    else{
        p = &dp[set*assoc + avail];
        
        p->valid = 1;
        p->time = time_count;
        p->tag = (addr/b_size)/num_of_sets;
        p->dirty = 0;
    }
}

void write_data(int addr, int c_size, int b_size, int assoc){
    int num_of_sets, set;
    int i, ev=0, avail=10;
    struct d_cache *p;
    
    num_of_sets = c_size/(b_size*assoc);
    set = (addr/b_size) % num_of_sets;
    
    for(i=0; i<assoc; i++){
        p = &dp[set*assoc + i];
        if(p->valid==1 && p->tag==(addr/b_size)/num_of_sets){
            p->time = time_count;
            p->dirty = 1;
            return;
        }
        else if(p->valid==0){
            if(i < avail)
                avail = i;
        }
    }
    s_miss++;

    if(avail==10){
        ev = evict(set, assoc, p->evmode);
        p = &dp[set*assoc + ev];
        
        if(p->dirty)
            s_total++;
        
        p->valid = 1;
        p->time = time_count;
        p->tag = (addr/b_size)/num_of_sets;
        p->dirty = 1;
    }
    else{
        p = &dp[set*assoc + avail];
        p->valid = 1;
        p->time = time_count;
        p->tag = (addr/b_size)/num_of_sets;
        p->dirty = 1;
    }
}

int evict(int set, int assoc, int mode){
    if (mode) {
        int i, tmp_time=0;
        int min = time_count+1, min_i=0;
        
        for(i=0; i<assoc; i++){
            tmp_time = dp[set*assoc + i].time;
            if(min > tmp_time){
                min = tmp_time;
                min_i = i;
            }
        }
        return min_i;
    }
    else {
        // fifo ±¸Çö
    }
}
