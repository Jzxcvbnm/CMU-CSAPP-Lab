/*
	test.cpp

	A test program for the timestamp library.
 */
#include <iostream.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>
#include <winbase.h>

#include "tsc.h"

/*
	void spin( int thread_number, int iteration_count,
			   int report_mode, int threshold )

	Create a table of readings from the timestamp counter
	and print a report to stdout.

    report_mode should be either REP_DUMP, in which case all
	the entries should be output, or REP_SWITCH, in which case
	only the switches should be noted.  In the latter case,
	threshold will be used as a threshold for what constitutes
	a switch, unless it's 0, in which case the minimum threshold
	will be calculated.
 */
void spin( int process_number,
		   int iteration_count,
		   int report_mode,
		   int report_count,
		   int threshold );

/*
    int make_thread( ... )

    Create a thread to run spin(), passing it the given arguments,
	and return 0.  Or, fail and return 1.
  */
int make_thread( int process_number,
			     int iteration_count,
				 int report_mode,
				 int report_count,
				 int threshold );
	
/*
	Cruft needed to get threads working
 */
#ifdef _MSC_VER
#include <windows.h>
#include <winbase.h>
#include <process.h>

/*
	semaphore synchronization primitive.
 */
typedef HANDLE sem_t;
CRITICAL_SECTION output_lock;

/*
    Initialise a semaphore.
 */
void sem_init( sem_t *sem, int init, int max ) {
	*sem = CreateSemaphore( NULL, init, max, NULL );
}

/*
    Wait for a semaphore (the P() operation).
 */
void sem_wait( sem_t *sem ) {
	WaitForSingleObject( *sem, INFINITE );
}

/*
    Release a semaphore (the V() operation ).
 */
void sem_release( sem_t *sem ) {
	ReleaseSemaphore( *sem, 1, 0 );
}

/*
    The semaphore for threads returning.
 */
sem_t semaphore;
signed __int64 gstart;

struct spin_args {
	spin_args( int p, int i, int d, int r, int t ) {
		process_number = p;
		iteration_count = i;
		report_type = d;
		report_count = r;
		threshold = t;
	}
	int process_number;
	int iteration_count;
	int report_type;
	int report_count;
	int threshold;
};

/*
    This is the function we want _beginthread to spawn at.
 */
void __cdecl _msc_spin( void *param ) {
	spin_args args = *((spin_args *) param);
	delete (spin_args *) param;
	spin( args.process_number, args.iteration_count,
		  args.report_type, args.report_count, args.threshold );
	sem_release( &semaphore );
}

/*
    Create the thread.
 */
int make_thread( int p, int i, int o, int r, int t ) {
	spin_args *s = new spin_args( p, i, o, r, t );

	sem_wait( &semaphore );

	if (_beginthread( &_msc_spin, 0, s ) == -1) {
		sem_release( &semaphore );
		cerr << "...failed: " << errno << endl;
		return 1;
	}
	return 0;
}



#endif

/*
    ostream & operator<< ( ostream &out, UINT64_T num )

	Print a 64-bit unsigned integer to the given output stream.
  */
ostream &operator<<( ostream &out, UINT64_T num ) {
	if (num) {
		char buffer[20];
		int i = 19;

		buffer[19] = '\0';
		while (num) {
			buffer[--i] = ('0' + (int) (num % 10));
			num /= 10;
		}
		return out << buffer + i;
	}else return out << 0;
}

/*
    ostream & operator<< ( ostream &out, UINT64_T num )

	Print a 64-bit signed integer to the given output stream.
  */
ostream &operator<<( ostream &out, signed __int64 num ) {
	if (num) {
		char buffer[20];
		int i = 19;
		bool neg = false;
		if (num < 0) {
			neg = true;
			num *= -1;
		}		

		buffer[19] = '\0';
		while (num) {
			buffer[--i] = ('0' + (int) (num % 10));
			num /= 10;
		}
		if (neg) buffer[--i] = '-';
		return out << buffer + i;
	}else return out << 0;
}

void calc(void * param) {

	// Old algorithm, fails under certain OS's on mobile processors,
	// because of how Sleep() is implemented.

	int* iterations = (int*)param;
	tsc_t begin, end, diff;
	diff = 0;

	for (int i = 0; i < *iterations; i++) {
		begin = tsc_get();
		Sleep(1 * 1000);  // one second
		end = tsc_get();
		diff += (end - begin);
	}
	
	diff /= *iterations;
	
	cout << endl << "This machine's estimated clock rate is: " << diff << " cycles per second" << endl;
	
	delete iterations;
	sem_release( &semaphore );
}

void calc_est(void * param) {

	// New algorithm, doesn't use Sleep()

	tsc_t begin, end;
	int tbegin, tend;
	unsigned __int64 limit = 10000000;

	do {

		begin = tsc_get();
		tbegin = GetTickCount();
		float f, g, h;
		f = 0.0;
		g = 0.0;
		h = 0.0;
		for (unsigned __int64 i = 0; i < limit; i++) { 
			f += (signed __int64) i / 1000;
			g += float(f * 0.000123); 
			h += 3;
			g /= 2;
		}
		tend = GetTickCount();
		end = tsc_get();
		limit *= 10;
	
	} while (tend == tbegin);

	unsigned __int64 diff = (signed __int64(end - begin) / (tend - tbegin)) * 1000;
	
	cout << endl << "This machine's estimated clock rate is: " << diff << " cycles per second" << endl;

	sem_release( &semaphore );
}


void calc_processor_speed(int iterations) {

	sem_wait( &semaphore );

	if (_beginthread( &calc_est, 0,  new int(iterations) ) == -1) {
		sem_release( &semaphore );
		cerr << "...failed: " << errno << endl;
	}

	return;
}



#define REP_SWITCH		0
#define REP_DUMP		1
#define REP_DUMP_ADDR	2
#define REP_CSV         4

#define PCOUNT	1
#define ICOUNT	10000
#define REPORT	REP_SWITCH
#define THRESH  0
#define RCOUNT  1

/* this doesn't actually work if REPORT doesn't default to REP_SWITCH */
#define _DEF(p,i,r,t) "default: timestamp -i " #i " -t " #t" -p " #p
#define DEFAULTS( p, i, r, t ) _DEF(p,i,r,t)

const char usage[] =
	"timestamp [-s] | [-t threshold] [-i iterations] [-p processes] \n"
	"   -s outputs estimated processor speed in cycles per second\n"
	"   -t specifies 'report' mode;  only gaps exceeding the given\n"
	"      threshold will be reported.  A threshold of 0 means to\n"
	"      generate a minimal threshold.\n"
	"   -i sets the number of iterations to use\n"
	"   -p sets the number of threads to run at once\n"
	"Report output is of the form:\n"
	"  threadnumber, start, end\n"
	"where start is the timestamp of the iteration where the thread switched\n"
	"in and end is the timestamp of the iteration where the thread switched out\n"
	DEFAULTS( PCOUNT, ICOUNT, RCOUNT, THRESH );

int main( int argc, char *argv[] ) {
	int icount = ICOUNT;
	int pcount = PCOUNT;
	int report = REPORT;
	int thresh = THRESH;
	int rcount = RCOUNT;
	int scount = 0;
	
	gstart = 0;
		
	InitializeCriticalSection(&output_lock);

#ifdef _MSC_VER
	SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
#endif

	for (int a = 1; a < argc; ++a) {
		if (argv[a][0] == '-') {
			switch (argv[a][1]) {
			case 's':
				if (argv[++a])
					scount = atoi( argv[a] );
				
				if (scount < 1) scount = 1;
							
				break;
				
			case 'd':
				report |= REP_DUMP;
				break;
			case 'c':
				report &= ~REP_DUMP;
				report |= REP_CSV;
				break;

			case 'a':
				report |= REP_DUMP_ADDR;
				break;
			case 'i':
				if (argv[++a])
					icount = atoi( argv[a] );
				else {
					cerr << "Error parsing after -i" << endl;
					exit(1);
				}
				break;
			case 'p':
				if (argv[++a]) {
					pcount = atoi( argv[a] );
				}else {
					cerr << "Error parsing after -p" << endl;
					exit(1);
				}
				break;
			case 'r':
				if (argv[++a]) {
					rcount = atoi( argv[a] );
				}else {
					cerr << "Error parsing after -r" << endl;
					exit(1);
				}
				break;
			case 't':
				report &= ~REP_DUMP;
				if (argv[++a])
					thresh = atoi( argv[a] );
				else {
					cerr << "Error parsing after -t" << endl;
					exit(1);
				}
				break;

			default:
				cerr << usage << endl;
				exit(1);
			}
		}else {
			cerr << usage << endl;
			exit(1);
		}
	}

	if (pcount < 1) {
		cerr << "Invalid process count: " << pcount << endl;
		exit(1);
	}

	if (rcount < 0 || rcount > pcount) {
		cerr << "Invalid report count: " << rcount << endl;
		exit(1);
	}else if (rcount == 0) {
		rcount = pcount;
	}
	
	// Forced options for ex6 SSD6 (by dms)
	rcount = pcount;
	report = (report | REP_CSV);

	if (!tsc_valid()) {
		cerr << argv[0] <<
				":  processor does not support timestamp register" << endl;
		return 1;
	}else {
		
		if (scount > 0) {

			sem_init( &semaphore, 1, 1 );			
			calc_processor_speed(scount);
			sem_wait(&semaphore);

		} else {
		
			int outcount = 0;
			sem_init( &semaphore, pcount, pcount );
			for (int proc = 0; proc < pcount; ++proc) {
				make_thread( proc, icount, report, rcount, thresh );
				outcount++;
			}
			while (outcount) {
				sem_wait( &semaphore );
				outcount--;
			}

		}
		return 0;
	}
}

void spin( int proc, int icnt, int report, int rcount, int thresh ) {
	tsc_t *table = new tsc_t[icnt];
	if (!table) {
		cerr << "[" << proc << "]: couldn't allocate "
			 << icnt << "-entry table" << endl;
		return;
	}else {
		for (int i = 0; i < icnt; ++i)
			table[i] = 0;
	}

	for (int i = 0; i < icnt; ++i)   // Loop to read cycles 
		table[i] = tsc_get();        // into a table (array)
	
	if (proc >= rcount)
		return;

	if (report & REP_DUMP) {
		for (i = 0; i < icnt; ++i) {
			cout << "[" << proc << "] " << i << ": " << table[i];
			if (report & REP_DUMP_ADDR)
				cout << " " << &table[i];
			cout << endl;
		}
	}else {
		if (!thresh) {
			thresh = (int) (table[1] - table[0]);
			for (i = 2; i < icnt; ++i) {
				if (table[i] - table[i - 1] < thresh) {
					thresh = (int) (table[i] - table[i - 1]);
				}else if (table[i] - table[i - 1] == thresh) {
					break;
				}
			}
			//cout << "[" << proc << "] Calculated threshold is " << thresh << endl;
		}

		
		EnterCriticalSection(&output_lock);

		tsc_t start = table[0];
		tsc_t lst = table[0];

		if (gstart == 0) gstart = (signed __int64) table[0];


		if (!(report & REP_CSV)) {
			cout << "[" << proc << "] First timestamp is " << start
			 << ";  the following are offsets from this" << endl;
		} 
		
		for (i = 1; i < icnt; ++i) {
			if (table[i] - table[i - 1] > thresh) {
				
				if (!(report & REP_CSV)) {
				cout << "[" << proc << "] Ran " << lst - start << "-"
					<< table[i - 1] - start << " ("
					<< table[i-1] - lst << "), didn't run "
					<< table[i - 1] - start << "-" << table[i] - start << " ("
					<< table[i] - table[i - 1] << ") ";
				if (report & REP_DUMP_ADDR)
					cout << &table[i-1];
				cout << endl;
				} else {

					signed __int64 from, to;
					from = signed __int64(lst)-gstart;
					to = signed __int64( table[i-1])-gstart;

					cout << proc << "," << from << "," << to << endl;
				}
				lst = table[i];
			}
		}
		if (!(report & REP_CSV)) {
			cout << "[" << proc << "] Ran from " << lst - start
				 << " until " << table[icnt - 1] - start << " ("
				<< table[icnt - 1] - lst << ")" << endl;
		} else {
			cout << proc << "," << signed __int64(lst)-gstart << "," << signed __int64(table[icnt-1])-gstart << endl;

		}

		LeaveCriticalSection(&output_lock);
	}
	delete table;
}
