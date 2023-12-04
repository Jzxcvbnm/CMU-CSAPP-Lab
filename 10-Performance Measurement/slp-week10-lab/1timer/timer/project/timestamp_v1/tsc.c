/*
	tsc.c

	Assembly-level routines to read the processor timestamp counter.
 */
#include <stdio.h>
#include "tsc.h"

int tsc_valid( void ) {
	int is_valid = 0;

/* presumably Visual C++ specific */
#ifdef _MSC_VER

/* the "ID" bit on CFLAGS */
#define ID_BIT		21
/* the "supports processor counter" bitmask */
#define TSC_BITMASK	0x10

	__asm {
		/* alright, first we should check for the CPUID instruction */
		pushfd					/* put EFLAGS on the stack */
		mov eax, [esp]			/* copy over to eax */
		btc [esp], ID_BIT		/* flip the bit on the stack */
		popfd					/* put the stack back in EFLAGS */
		pushfd					/* and take it out again */
		test eax, [esp]			/* test for similarity */
		jz done 				/* identical: flip didn't take */
		mov eax, [esp]			/* copy over to eax */
		btc [esp], ID_BIT		/* flip the bit again */
		popfd					/* put the stack back in EFLAGS */
		pushfd					/* and take it out again again */
		test eax, [esp]			/* test for similarity */
		jz done 				/* identical: flip didn't take */

		mov eax, 0x1			/* the code for what we want */
		cpuid					/* get it */
		and edx, TSC_BITMASK	/* check only the bit we want */
		jz done					/* zero if it wasn't set, i.e. no capability */
		mov is_valid, 1			/* set is_valid */
		jmp done				/* goto done */
done:
		add esp, 4				/* clean up the stack */
	}

/* no other cases:  note that we "cleverly" compile to constant-zero */
#endif

	return is_valid;
}

tsc_t tsc_get( void ) {
/*
	union {
		UINT64_T whole;
		struct {
			UINT32_T high;
			UINT32_T low;
		} split;
	} store;
*/

#ifdef _MSC_VER

	__asm {
		rdtsc

		/* This nicely puts our returned value in storage, from which
	       we can conveniently return.  Note that standard calling
		   conventions have us returning 64 bit quantities in eax and
		   edx anyway, so we might as well skip the middle step.
		
		mov store.split.high, eax
		mov store.split.low, edx
		*/
	}
#endif
	 /*	fprintf( stderr, "dbg_ts: got %lx, %lx: %I64x\n", store.split.high,
				store.split.low, store.whole ); */
	 /* return store.whole; */
}