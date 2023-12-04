/*

  driver.c

  Driver program.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include "defs.h"
#include "cache.h"


#define FIND_BEST_OF_MANY


/*******************************************
 *******************************************
 *
 *		FILE-GLOBAL DECLARATIONS
 *
 *******************************************
 *******************************************
 */


/* Keep track of a number of different test functions */
#define MAX_BENCHMARKS 100
#define DIM_CNT 5



/* 
 * defining FIND_BEST_OF_MANY might help you sift through many different functions;
 *  if it is defined, the program will print the name and speed of the best function for each size tested.
 */
#ifdef FIND_BEST_OF_MANY
char* best_desc;
double best_mean;
#endif



/* struct to hold all information about a certain function to be tested */
typedef struct {
	lab_test_func tfunct;
	int cache_hits[DIM_CNT];
	int cache_attempts[DIM_CNT];
	float cache_hitrate[DIM_CNT];
	char *description;
	unsigned short valid; /* The function is tested if this is non zero */
} bench_t;


/* keep track of benchmarks */
static bench_t benchmarks_rotate[MAX_BENCHMARKS];
static int rotate_benchmark_count = 0;
static bench_t benchmarks_smooth[MAX_BENCHMARKS];
static int smooth_benchmark_count = 0;


/* Define block size for aligning data */
#define BSIZE 64
#define MAX_DIM 1280 /* 1024+256 */
#define ODD_DIM 96 /* not a power of 2, not a dimension that will be scored for speed */

/* giant array to hold all three image matrices */
static pixel data[3*MAX_DIM*MAX_DIM+BSIZE/sizeof(pixel)];

/* pointers to each image matrix */
static pixel *orig = 0;
static pixel *copy_of_orig = 0;
static pixel *result = 0;

/* sizes we will be testing */
static int test_dim[] = {64, 128, 256, 512, 1024};


/*******************************************
 *******************************************
 *
 *			STATIC FUNCTIONS
 *
 *******************************************
 *******************************************
 */


/* Returns random integer in interval [low, high) */
static int random_in_interval(int low, int high) {
	int size = high - low;

	return (rand()% size) + low;
}


static int clean_up_matrix[128][128];
static void clean_up_cache(){
	int i,j;
	for(i=0; i<1000; i++){
		for(j=0; j<1000; j++){
			clean_up_matrix[i][j] = 0;
		}
	}
}


/* create a dim*dim image of random contents */
static void create(int dim)
{
	int i, j;
	
	orig = data;
	while ((unsigned) orig % BSIZE) {
		orig++;
	}
	result = orig + dim*dim;
	copy_of_orig = result + dim*dim;

	for(i=0; i<dim; i++) {
		for(j=0; j<dim; j++) {
			/* Original image initialized to random colors */
			orig[PIXEL(i,j,dim)].red = random_in_interval(0, 256);
			orig[PIXEL(i,j,dim)].green = random_in_interval(0, 256);
			orig[PIXEL(i,j,dim)].blue = random_in_interval(0, 256);
			//orig[PIXEL(i,j,dim)].alpha = random_in_interval(0, 256);
			/* Copy of original image for checking result */
			copy_of_orig[PIXEL(i,j,dim)].red = orig[PIXEL(i,j,dim)].red;
			copy_of_orig[PIXEL(i,j,dim)].green = orig[PIXEL(i,j,dim)].green;
			copy_of_orig[PIXEL(i,j,dim)].blue = orig[PIXEL(i,j,dim)].blue;
			//copy_of_orig[PIXEL(i,j,dim)].alpha = orig[PIXEL(i,j,dim)].alpha;
			/* Result image initialized to all black */
			result[PIXEL(i,j,dim)].red = 0;
			result[PIXEL(i,j,dim)].green = 0;
			result[PIXEL(i,j,dim)].blue = 0;
			//result[PIXEL(i,j,dim)].alpha = 0;
		}
	}
	return;
}


/* unimplemented */
static void cleanup(int dim) {
	return;
}


/* Returns 1 if the two arguments don't have same RGBA values, 0 o.w. */
static int compare_pixels(pixel p1, pixel p2) {
	return (p1.red != p2.red) || (p1.green != p2.green) || (p1.blue != p2.blue) || 0/*(p1.alpha != p2.alpha)*/;
}


/* Make sure the orig array is unchanged */
static int check_orig(int dim) {
	int i, j;

	for (i = 0; i < dim; i++) 
		for (j = 0; j < dim; j++) 
			if (compare_pixels(orig[PIXEL(i,j,dim)], copy_of_orig[PIXEL(i,j,dim)])) {
		printf("\n");
		printf("Error: Original image has been changed!\n");
		printf("Changed at position %d, %d\n", i, j);
		printf("A: (%d, %d, %d)\nB: (%d,%d,%d)\n",
				orig[PIXEL(i,j,dim)].red,
				orig[PIXEL(i,j,dim)].green,
				orig[PIXEL(i,j,dim)].blue,
				copy_of_orig[PIXEL(i,j,dim)].red,
				copy_of_orig[PIXEL(i,j,dim)].green,
				copy_of_orig[PIXEL(i,j,dim)].blue);
		return 1;
	}

	return 0;
}

/* Make sure the rotate actually works. The orig array should not 
	 have been tampered with! 
*/
static int check_rotate(int dim) {
	int err = 0;
	int i, j;
	int badi = 0;
	int badj = 0;
	pixel orig_bad, res_bad;

	if (check_orig(dim)) return 1; /* return 1 if original image has been
				 changed */

	for (i = 0; i < dim; i++) 
		for (j = 0; j < dim; j++) 
			if (compare_pixels(orig[PIXEL(i,j,dim)], result[PIXEL(dim-1-j,i,dim)])) {
	err++;
	badi = i;
	badj = j;
	orig_bad = orig[PIXEL(i,j,dim)];
	res_bad = result[PIXEL(dim-1-j,i,dim)];
			}
	
	if (err) {
		printf("\n");
		printf("ERROR: Dimension=%d, %d errors\n", dim, err);		
		printf("E.g., The following two pixels should have equal value:\n");
		printf("src[%d][%d].{red,green,blue,alpha} = {%d,%d,%d}\n",
		 badi, badj, orig_bad.red, orig_bad.green, orig_bad.blue); 
					//, orig_bad.alpha);
		printf("dst[%d][%d].{red,green,blue, alpha} = {%d,%d,%d}\n",
		 (dim-1-badj), badi, res_bad.red, res_bad.green, res_bad.blue);
					//, res_bad.alpha);
	}

	return err;
}

/* Make sure the smooth actually works. The orig array should not 
	 have been tampered with! 
*/
static int check_smooth(int dim) {
	int err = 0;
	int i, j;
	int badi = 0;
	int badj = 0;
	pixel p, orig_bad, res_bad;

	if (check_orig(dim)) return 1; /* return 1 if original image has been
				 changed */

	for( i=0; i < dim; i++ ) {
		for( j=0 ; j < dim ; j++ ) {
			if (i != 0 && j != 0 && i != (dim - 1) && j != (dim - 1)) {
				cache_smooth(&p, &orig[PIXEL(i,j,dim)],
						&orig[PIXEL(i-1,j,dim)],
						&orig[PIXEL(i+1,j,dim)],
						&orig[PIXEL(i,j-1,dim)],
						&orig[PIXEL(i,j+1,dim)],
						&orig[PIXEL(i-1,j-1,dim)],
						&orig[PIXEL(i+1,j+1,dim)],
						&orig[PIXEL(i-1,j-1,dim)],
						&orig[PIXEL(i+1,j+1,dim)]);
			}
			else {
				cache_copy(&p, &orig[PIXEL(i, j, dim)]);
			}
			
			if(compare_pixels(p, result[PIXEL(i,j,dim)]))
			{
				err++;
				badi = i;
				badj = j;
				orig_bad = p;
				res_bad = result[PIXEL(i,j,dim)];
			}
		}
	}
	
	if (err) {
		printf("\n");
		printf("ERROR: Dimension=%d, %d errors\n", dim, err);		
		printf("E.g., The following two pixels should have equal value:\n");
		printf("src[%d][%d].{red,green,blue,alpha} = {%d,%d,%d}\n",
		 badi, badj, orig_bad.red, orig_bad.green, orig_bad.blue); 
					//, orig_bad.alpha);
		printf("dst[%d][%d].{red,green,blue, alpha} = {%d,%d,%d}\n",
		 badi, badj, res_bad.red, res_bad.green, res_bad.blue);
					//, res_bad.alpha);
	}

	return err;
}


static void print(int dim, pixel *screen){
	int i, j;

	assert(screen != 0);

	for (j = 0; j < dim; j++){
		for (i = 0; i < dim; i++){
			int rval, bval, gval;
			rval = (int) screen[PIXEL(i,j,dim)].red;
			gval = (int) screen[PIXEL(i,j,dim)].green;
			bval = (int) screen[PIXEL(i,j,dim)].blue;
			//bval = (int) screen[PIXEL(i,j,dim)].alpha;
			printf("%d ", rval+bval+gval);
		}
		printf("\n");
	}	

	return;
}


static void func_wrapper(void *arglist[]) {
	pixel *src, *dst;
	int mydim;
	lab_test_func f;

	f = (lab_test_func) arglist[0];
	mydim = *((int *) arglist[1]);
	src = (pixel *) arglist[2];
	dst = (pixel *) arglist[3];

	(*f)(mydim, src, dst);

	return;
}

static char rotate_naive_reference_descr[] = "Rotate Reference Naive Implementation!";
static void rotate_naive_reference(int dim, pixel* src, pixel* dst) {
	int i, j;
	for(i=0; i<dim; i++) {
		for(j=0; j<dim; j++) {
			COPY(&dst[PIXEL(dim-1-j,i,dim)], &src[PIXEL(i,j,dim)]);
		}
	}
}

static char smooth_naive_reference_descr[] = "Smooth Reference Naive Implementation!";
static void smooth_naive_reference(int dim, pixel* src, pixel* dst) {
	int i, j;
	for(i=0; i<dim;i++) {
		COPY(&dst[PIXEL(i,0,dim)], &src[PIXEL(i,0,dim)]);
		COPY(&dst[PIXEL(i,dim-1,dim)], &src[PIXEL(i,dim-1,dim)]);
	}
	for(j=1; j<dim-1;j++) {
		COPY(&dst[PIXEL(0,j,dim)], &src[PIXEL(0,j,dim)]);
		COPY(&dst[PIXEL(dim-1,j,dim)], &src[PIXEL(dim-1,j,dim)]);
	}
	for(i=1; i<dim-1; i++) {
		for(j=1; j<dim-1; j++) {
			SMOOTH(&dst[PIXEL(j,i,dim)],
					&src[PIXEL(j,i,dim)],
					&src[PIXEL(j-1,i,dim)],
					&src[PIXEL(j+1,i,dim)],
					&src[PIXEL(j,i+1,dim)],
					&src[PIXEL(j,i-1,dim)],
					&src[PIXEL(j-1,i-1,dim)],
					&src[PIXEL(j+1,i+1,dim)],
					&src[PIXEL(j-1,i+1,dim)],
					&src[PIXEL(j+1,i-1,dim)]);
		}
	}
}



static void run_rotate_benchmark(int idx, int dim) {
	benchmarks_rotate[idx].tfunct(dim, orig, result);
}

static void run_smooth_benchmark(int idx, int dim) {
	benchmarks_smooth[idx].tfunct(dim, orig, result);
}



static void test_rotate(int bench_index) {
	int i;
	int test_num;
	char *description = benchmarks_rotate[bench_index].description;
	
	/* Check for odd dimension */
	create(ODD_DIM);
	run_rotate_benchmark(bench_index, ODD_DIM);
	if (check_rotate(ODD_DIM)) {
		printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
			benchmarks_rotate[bench_index].description, ODD_DIM);
		return;
	}
	
	/* run tests on all dimensions */
	for(test_num=0; test_num < DIM_CNT; test_num++) {
		int dim;
		
		/* Create a test image of the required dimension */
		dim = test_dim[test_num];
		create(dim);
		
		
		/* Check that the code works */
		run_rotate_benchmark(bench_index, dim);
		if (check_rotate(dim)) {
			printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
			benchmarks_rotate[bench_index].description, dim);
			return;
		}
		
		/* Measure rotate cache performance */
		{
			int tmpdim = dim;
			void *arglist[4];
			double dimension = (double) dim;
			double work = dimension*dimension;
#ifdef DEBUG
			printf("DEBUG: dimension=%.1f\n",dimension);
			printf("DEBUG: work=%.1f\n",work);
#endif
			arglist[0] = (void *) benchmarks_rotate[bench_index].tfunct;
			arglist[1] = (void *) &tmpdim;
			arglist[2] = (void *) orig;
			arglist[3] = (void *) result;
			
			create(dim);
			
			set_cache_measurement_enabled(1);
			reset_cache();
			func_wrapper(arglist);
			benchmarks_rotate[bench_index].cache_attempts[test_num] = 
						get_read_count() + get_write_count();
			benchmarks_rotate[bench_index].cache_hits[test_num] = 
						benchmarks_rotate[bench_index].cache_attempts[test_num]
							- get_read_miss_count() - get_write_miss_count();
			benchmarks_rotate[bench_index].cache_hitrate[test_num] = 
				((float)benchmarks_rotate[bench_index].cache_hits[test_num]) /
				((float)benchmarks_rotate[bench_index].cache_attempts[test_num]);
		}
		
	}
	/* Print results as a table */
	printf("Rotate: Version = %s:\n", description);
	printf("Dim");
	for (i = 0; i < DIM_CNT; i++)
		printf("\t%d", test_dim[i]);
	printf("\tMean\n");
	
	
	printf("hitrate");
	for (i = 0; i < DIM_CNT; i++) {
		printf("\t%.1f", 100.0*((double)(benchmarks_rotate[bench_index].cache_hits[i])) /
						  ((double)(benchmarks_rotate[bench_index].cache_attempts[i])));
	}
	printf("\n");
	
	/* Compute Improvement */
	
	{
		double prod;
		double mean;
		double ratios[DIM_CNT];
		int exists = 0;
		
		
		prod = 1.0;   //* Geometric mean */ /*
		
		printf("Incr.");
		for (i = 0; i < DIM_CNT; i++) {
			//our naive impl.
			ratios[i] = benchmarks_rotate[bench_index].cache_hitrate[i] / 
					benchmarks_rotate[0].cache_hitrate[i];

			prod *= ratios[i];
			printf("\t%.2f", ratios[i]);
		}
		///* Geometric mean */ /*
		mean = pow(prod, 1.0/(double) DIM_CNT);
#ifdef FIND_BEST_OF_MANY
			if(mean > best_mean) {
				best_mean = mean;
				best_desc = benchmarks_rotate[bench_index].description;
			}
#endif
		printf("\t%.2f", mean);
		printf("\n\n");
		
	}
	
#ifdef DEBUG
	fflush(stdout);
#endif
	return;	
}


static void test_smooth(int bench_index) {
	int i;
	int test_num;
	char *description = benchmarks_smooth[bench_index].description;
	
	/* Check for odd dimension */
	create(ODD_DIM);
	run_smooth_benchmark(bench_index, ODD_DIM);
	
	if (check_smooth(ODD_DIM)) {
		printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
			benchmarks_smooth[bench_index].description, ODD_DIM);
		return;
	}
	
	/* run tests on all dimensions */
	for(test_num=0; test_num < DIM_CNT; test_num++) {
		int dim;
		
		/* Create a test image of the required dimension */
		dim = test_dim[test_num];
		
		create(dim);
		
		
		/* Check that the code works */
		run_smooth_benchmark(bench_index, dim);
		if (check_smooth(dim)) {
			printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
			benchmarks_smooth[bench_index].description, dim);
			return;
		}
		
		/* Measure smooth cache performance */
		{
			int imgdim = dim;
			void *arglist[4];
			double dimension = (double) dim;
			double work = dimension*dimension;
#ifdef DEBUG
			printf("DEBUG: dimension=%.1f\n",dimension);
			printf("DEBUG: work=%.1f\n",work);
#endif
			arglist[0] = (void *) benchmarks_smooth[bench_index].tfunct;
			arglist[1] = (void *) &imgdim;
			arglist[2] = (void *) orig;
			arglist[3] = (void *) result;
			
			create(dim);
			
			set_cache_measurement_enabled(1);
			reset_cache();
			func_wrapper(arglist);
			benchmarks_smooth[bench_index].cache_attempts[test_num] = 
						get_read_count() + get_write_count();
			benchmarks_smooth[bench_index].cache_hits[test_num] = 
						benchmarks_smooth[bench_index].cache_attempts[test_num]
							- get_read_miss_count() - get_write_miss_count();
			benchmarks_smooth[bench_index].cache_hitrate[test_num] = 
				((float)benchmarks_smooth[bench_index].cache_hits[test_num]) /
				((float)benchmarks_smooth[bench_index].cache_attempts[test_num]);
		}
		
		
		
	}
	/* Print results as a table */
	printf("Smooth: Version = %s:\n", description);
	printf("Dim");
	for (i = 0; i < DIM_CNT; i++)
		printf("\t%d", test_dim[i]);
	printf("\tMean\n");
	
	
	printf("hitrate");
	for (i = 0; i < DIM_CNT; i++) {
		printf("\t%.1f", 100.0*((double)(benchmarks_smooth[bench_index].cache_hits[i])) /
						  ((double)(benchmarks_smooth[bench_index].cache_attempts[i])));
	}
	printf("\n");
	
	/* Compute Improvement */
	
	{
		double prod;
		double mean;
		double ratios[DIM_CNT];
		int exists = 0;
		
		
		prod = 1.0;   //* Geometric mean */ /*
		
		printf("Incr.");
		for (i = 0; i < DIM_CNT; i++) {
			//our naive impl.
			ratios[i] = benchmarks_smooth[bench_index].cache_hitrate[i] / 
					benchmarks_smooth[0].cache_hitrate[i];
			

			prod *= ratios[i];
			printf("\t%.2f", ratios[i]);
		}
		///* Geometric mean */ /*
		mean = pow(prod, 1.0/(double) DIM_CNT);
		printf("\t%.2f", mean);
		printf("\n\n");
		
#ifdef FIND_BEST_OF_MANY
		if(mean > best_mean) {
			best_mean = mean;
			best_desc = benchmarks_smooth[bench_index].description;
		}
#endif
	}
	
	
	
#ifdef DEBUG
	fflush(stdout);
#endif
	return;	
}




/*******************************************
 *******************************************
 *
 *			GRADER-ONLY FUNCTIONS
 *
 *******************************************
 *******************************************
 */

//void test_called_defines(


/*******************************************
 *******************************************
 *
 *				FUNCTIONS
 *
 *******************************************
 *******************************************
 */

void add_rotate_function(lab_test_func f, char *description) {
	if(rotate_benchmark_count < MAX_BENCHMARKS) {
		benchmarks_rotate[rotate_benchmark_count].tfunct = f;
		benchmarks_rotate[rotate_benchmark_count].description = description;
		benchmarks_rotate[rotate_benchmark_count].valid = 0;
		rotate_benchmark_count++;
	}
	else {
		printf("ERROR: tried to register too many rotate functions.\n");
	}
}

void add_smooth_function(lab_test_func f, char *description) {
	if(smooth_benchmark_count < MAX_BENCHMARKS) {
		benchmarks_smooth[smooth_benchmark_count].tfunct = f;
		benchmarks_smooth[smooth_benchmark_count].description = description;
		benchmarks_smooth[smooth_benchmark_count].valid = 0;
		smooth_benchmark_count++;
	}
	else {
		printf("ERROR: tried to register too many smooth functions.\n");
	}
}

int main(int argc, char *argv[])
{
	extern char* optarg;
	int i;
	int seed = 1729;
	int quit_after_dump = 0;
	char c = '0';
	char *bench_func_file = NULL;
	char *func_dump_file = NULL;
	char *group_id = NULL;
	/* register all the defined functions */
	add_rotate_function(rotate_naive_reference, rotate_naive_reference_descr);
	register_rotate_functions();
	add_smooth_function(smooth_naive_reference, smooth_naive_reference_descr);
	register_smooth_functions();
	reset_cache();
	set_cache_measurement_enabled(0);
	srand(seed);

	for(i=0; i<rotate_benchmark_count; i++)
		benchmarks_rotate[i].valid = 1;
	
	for(i=0; i<smooth_benchmark_count; i++)
		benchmarks_smooth[i].valid = 1;
	
#ifdef FIND_BEST_OF_MANY
	best_mean = 0;
#endif
	
	for(i=0; i<rotate_benchmark_count; i++) {
		if (benchmarks_rotate[i].valid) {
			test_rotate(i);
		}
	}

#ifdef FIND_BEST_OF_MANY
	printf("Best algo here: %s, \t%f\n\n\n", best_desc, best_mean);
	best_mean = 0;
#endif

	
	for(i=0; i<smooth_benchmark_count; i++) {
		if(benchmarks_smooth[i].valid) {
			test_smooth(i);
		}
	}
	
#ifdef FIND_BEST_OF_MANY
	printf("Best algo here: %s, \t%f\n\n", best_desc, best_mean);
#endif
	
	return 0;
}



