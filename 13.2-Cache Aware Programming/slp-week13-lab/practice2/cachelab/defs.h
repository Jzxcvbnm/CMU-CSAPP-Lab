/*

  defs.h

  Useful definitions for Exercise 5.

  DO NOT MODIFY ANYTHING IN THIS FILE

*/

#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdlib.h>

#define PIXEL(i,j,n) ((i%n)*(n)+(j%n))

#define COPY(daddr, saddr) cache_copy(daddr, saddr)
#define SMOOTH(d, s1, s2, s3, s4, s5, s6, s7, s8, s9) cache_smooth(d,s1,s2,s3,s4,s5,s6,s7,s8,s9)
#define READ(saddr) cache_read(saddr)
#define WRITE(daddr) cache_write(daddr)

typedef struct {
   unsigned short red	: 8;
   unsigned short green	: 8;
   unsigned short blue	: 8;
   unsigned short alpha	: 8;
} pixel;


typedef void (*lab_test_func) (int, pixel*, pixel*);

void rotate(int, pixel *, pixel *);

void register_rotate_functions(void);
void add_rotate_function(lab_test_func, char*);
void register_smooth_functions(void);
void add_smooth_function(lab_test_func, char*);

#endif /* _DEFS_H_ */
