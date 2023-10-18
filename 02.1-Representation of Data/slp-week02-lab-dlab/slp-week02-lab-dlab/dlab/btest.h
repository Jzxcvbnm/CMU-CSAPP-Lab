/*
 * CS:APP Data Lab
 */

/* 
 * Students enter their personal names and login IDs in a struct of
 * this type in their bits.c file.
 */
typedef struct {
    char *name;   
    char *id;
} info_struct;

/* Declare different function types */
typedef int (*funct_t) (void);
typedef int (*funct1_t)(int);
typedef int (*funct2_t)(int, int); 
typedef int (*funct3_t)(int, int, int); 

/* Combine all the information about a function and its tests as structure */
typedef struct {
    char *name;
    funct_t solution_funct; /* These will typically be casted */
    funct_t test_funct;
    int args;
    char *ops;
    int op_limit;
    int rating;
    int arg_ranges[3][2]; /* Min & Max values for each argument */
} test_rec, *test_ptr;

extern test_rec test_set[];







