#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "getopt.h"
#include "debugmalloc.h"

#define NUMTESTCASES 8

static void run_test_case(int n) {
	switch(n) {
	case 1: { /* no error, just a basic test */
				char *str = (char *) MALLOC(12);
				strcpy(str, "123456789");
				FREE(str);
				printf("Size: %d\n", AllocatedSize());
				PrintAllocatedBlocks();
			}
		break;
	case 2: { /* should overflow by 1 */
				char *str = (char *) MALLOC(8);
				strcpy(str, "12345678");
				FREE(str);
			}
		break;
	case 3: { /* should overflow by 1, harder to catch
				because of alignment */
				char *str = (char *) MALLOC(2);
				strcpy(str, "12");
				FREE(str);
			}
		break;
	case 4: { /* memory leak */
				void *ptr = MALLOC(4), *ptr2 = MALLOC(6);
				FREE(ptr);
				printf("Size: %d\n", AllocatedSize());
				PrintAllocatedBlocks();
			}
		break;
	case 5: {
				void *ptr = MALLOC(4);
				FREE(ptr);
				FREE(ptr);
			}
		break;
	case 6: {
				char *ptr = (char *) MALLOC(4);
				*((int *) (ptr - 8)) = 8 + (1 << 31);
				FREE(ptr);
			}
		break;
	case 7: {
				char ptr[5];
				FREE(ptr);
			}
		break;
	case 8: {
				int i;
				int *intptr = (int *) MALLOC(6);
				char *str = (char *) MALLOC(12);
				
				for(i = 0; i < 6; i++) {
					intptr[i] = i;
				}
				if (HeapCheck() == -1) {
					printf("\nCaught Errors\n");
				}
			}
	default:
		;
	}
}

static void usage(char *cmd) {
    printf("Usage: %s [-h] [-t <testnum>]\n", cmd);
    printf("  -h     Prints this message\n");
	printf("  -t <n> Runs n-th test\n");
    exit(1);
}


int main(int argc, char *argv[]) {
	char c;
	int tracenum = 0;
	
	while ((c = getopt(argc, argv, "ht:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			break;
		case 't':
			if ((tracenum = atoi(optarg)) > NUMTESTCASES || tracenum <= 0) {
				usage(argv[0]);
			}
			break;
		default:
			usage(argv[0]);
		}
	}
	
	if (tracenum) {
		run_test_case(tracenum);
	}
	else {
		usage(argv[0]);
	}

	return 0;
}
