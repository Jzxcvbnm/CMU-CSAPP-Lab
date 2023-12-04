/* For cache simulator */
#ifndef CACHE_H
#define CACHE_H

#include "defs.h"


void set_cache_measurement_enabled(int enabled);

/* Create empty cache with #sets = 2^s_bits, block size = 2^b_bits */
void reset_cache(void);
void reset_cache2(int s_bits, int b_bits);

/* Reset cache statistics without clearing cache */
void clear_cache_statistics();

void cache_copy(pixel* daddr, pixel* saddr);
void cache_smooth(pixel* dst, pixel* s1, pixel* s2, pixel* s3, pixel* s4, pixel* s5, pixel* s6, pixel* s7, pixel* s8, pixel* s9);

int get_read_count();
int get_write_count();
int get_read_miss_count();
int get_write_miss_count();

double get_read_miss_rate();
double get_write_miss_rate();

double get_miss_rate();



#endif
