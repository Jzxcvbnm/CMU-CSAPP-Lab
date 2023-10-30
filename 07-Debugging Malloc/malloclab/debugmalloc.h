#ifndef DEBUGMALLOC_H
#define DEBUGMALLOC_H

#include <stdlib.h>

/* Macros that will call the wrapper functions with
	the current filename and line number */

#define MALLOC(s)	MyMalloc(s, __FILE__, __LINE__)
#define FREE(p)		MyFree(p, __FILE__, __LINE__)

/* Wrappers for malloc and free.  You will implement
	these functions. */

void *MyMalloc(size_t size, char *filename, int linenumber);
void MyFree(void *ptr, char *filename, int linenumber);


/* Required function for detecting memory leaks */

int AllocatedSize(); /* returns number of bytes allocated */


/* Optional functions if you wish to implement the global list */

void PrintAllocatedBlocks();
int HeapCheck(); /* returns 0 if all blocks have not been corrupted,
					-1 if an error is detected */


#endif
