#include <stdlib.h>
#include <stdio.h>
#include "time.h"
#include "iostream.h"

void my_subroutine(long n)
{
    // this routine just does something that
    // takes some time so we can illustrate
    // timing a subroutine call:
    char s[16];
	for (long i = 0; i < n; i++) {
		_itoa(i, s, sizeof(s));
    }
}


int main(int argc, char* argv[])
{
    long n = 1000000;
    clock_t start = clock();
    my_subroutine(n);
    clock_t finish = clock();
    double duration = (double)(finish - start) / CLOCKS_PER_SEC;
    cout << "Time for " << n << " iterations: " <<
	duration << "s," << " precision is " << 
	CLOCKS_PER_SEC << " clocks per second." <<
	endl;
    return 0;
}

