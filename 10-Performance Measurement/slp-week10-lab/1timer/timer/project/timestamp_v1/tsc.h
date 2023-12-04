/*
	tsc.h

	Interface to the IA-32 timestamp register instruction,
	RDTSC.
 */

#if defined(__STDC__)
#define ARGS( prototype ) prototype
#else
#define ARGS( prototype )
#endif

/* defined if Visual C++ */
#if defined(_MSC_VER)
#define UINT64_T unsigned __int64
#define UINT32_T unsigned __int32
#elif defined(__unix)
#define UINT64_T uint64_t
#define UINT32_T uint32_t
#else
#define UINT64_T uint64_t
#define UINT32_T uint32_t
#endif

/*
	__cplusplus was in the draft ANSI C++ standard, but for some reason
    got dropped.  Regardless, it appears in most C++ compilers, by which
	I mean gcc and Visual C++.
 */
#if defined(__cplusplus)
extern "C" {
#endif

typedef UINT64_T tsc_t;

/*
	int tsc_valid()

	Determine whether the (Intel) processor has the RDTSC instruction
	for reading the timestamp counter.  Return zero to mean "no";
	else, return nonzero.

	If tsc_valid() is false, programs should not call 
	tsc_get().
 */
int tsc_valid( ARGS(void) );

/*
	tsc_t tsc_get()

	Retrieve the current value of the timestamp counter.
 */
tsc_t tsc_get( ARGS(void) );

#if defined(__cplusplus)
}
#endif
