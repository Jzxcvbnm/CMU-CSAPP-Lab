/* Cache simulator:
	 Simulates 4-way set-associative cache, using LRU for replacement policy
	 Cache parameters configured at compile time */


#include "cache.h"
#include "defs.h"
#include "math.h"

/* cache parameters */
#define CACHE_SIZE			16384	/*	16KB cache				*/
#define SET_ASSOCIATIVITY	1		/*	4-way set associative	*/
#define BLOCK_SIZE			32		/*	32-byte lines			*/



/* By default, configured for 16KB cache with 32B block size */
static int block_bits = 5;
static int set_bits = 9;		//log(CACHE_SIZE/SET_ASSOCIATIVITY/BLOCK_SIZE);

#define MAX_SET_COUNT (1 << 10)


//#define BLOCK_SIZE (1<<(block_bits))
#define SET_COUNT (1<<(set_bits))
#define SET_MASK (SET_COUNT-1)

/* Get cache index from address */
#define GET_INDEX(addr) (SET_MASK & ((unsigned) (addr) >> block_bits))
/* Get cache tag from address */
#define GET_TAG(addr) ((unsigned) (addr) >> (block_bits + set_bits))

static unsigned tags[MAX_SET_COUNT][SET_ASSOCIATIVITY];

static int read_misses = 0;
static int reads = 0;
static int write_misses = 0;
static int writes = 0;



void set_cache_measurement_enabled(int enabled) {
	return;
}



/* Reset cache statistics without clearing cache */
void clear_cache_statistics()
{
	read_misses = 0;
	reads = 0;
	write_misses = 0;
	writes = 0;
}

/* Create empty cache with #sets = 2^s_bits, block size = 2^b_bits */
void reset_cache(void)
{
	clear_cache_statistics();
	reset_cache2(set_bits, block_bits);
}

void reset_cache2(int s_bits, int b_bits)
{
	int i, j;
	set_bits = s_bits;
	block_bits = b_bits;
	for (i = 0; i < SET_COUNT; i++) {
		for( j=0; j<SET_ASSOCIATIVITY; j++) {
			tags[i][j] = -1;
		}
	}
	clear_cache_statistics();
}


int get_read_count()
{
	return reads;
}

int get_write_count()
{
	return writes;
}

int get_read_miss_count()
{
	return read_misses;
}

int get_write_miss_count()
{
	return write_misses;
}

double get_read_miss_rate()
{
	return (double) read_misses / (double) reads;
}
double get_write_miss_rate()
{
	return (double) write_misses / (double) writes;
}

double get_miss_rate()
{
	return (double) (read_misses + write_misses) / (double) (reads+ writes);
}

static void cache_read(int *addr)
{
	int i, j;
	unsigned tindex = GET_INDEX(addr);
	unsigned tag = GET_TAG(addr);
	reads++;
	for(i=0; i<SET_ASSOCIATIVITY; i++) {
		if(tags[tindex][i] == tag) {
			for( j=i ; j>0 ; j-- ) {
				tags[tindex][j] = tags[tindex][j-1];
			}
			tags[tindex][0] = tag;
			return;
		}
	}
	read_misses++;
	for(j=SET_ASSOCIATIVITY-1; j>0; j--) {
		tags[tindex][j] = tags[tindex][j-1];
	}
	tags[tindex][0] = tag;
	return;
}

static void cache_write(int *addr)
{
	int i, j;
	unsigned tindex = GET_INDEX(addr);
	unsigned tag = GET_TAG(addr);
	writes++;
	for(i=0; i<SET_ASSOCIATIVITY; i++) {
		if(tags[tindex][i] == tag) {
			for( j=i ; j>0 ; j-- ) {
				tags[tindex][j] = tags[tindex][j-1];
			}
			tags[tindex][0] = tag;
			return;
		}
	}
	write_misses++;
	for(j=SET_ASSOCIATIVITY-1; j>0; j--) {
		tags[tindex][j] = tags[tindex][j-1];
	}
	tags[tindex][0] = tag;
	return;
}

void cache_copy(pixel* daddr, pixel* saddr)
{
	cache_read((int*) saddr);
	cache_write((int*) daddr);
	*daddr = *saddr;
}

#define ps pixel*
void cache_smooth(ps dst,ps s1,ps s2,ps s3,ps s4,ps s5,ps s6,ps s7,ps s8,ps s9)
{
	int red=0, green=0, blue=0, i;
	pixel*  src[] = {s1, s2, s3, s4, s5, s6, s7, s8, s9};
	pixel p;
	for( i=0; i < 5; i++) {
		cache_read((int*)src[i]);
		red += src[i]->red;
		green += src[i]->green;
		blue += src[i]->blue;
	}
	cache_write((int*)dst);
	p.red = red/5;
	p.green = green/5;
	p.blue = blue/5;
	*dst = p;
}
#undef ps







