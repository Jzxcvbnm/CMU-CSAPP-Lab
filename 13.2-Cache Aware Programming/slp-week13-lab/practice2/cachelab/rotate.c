#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "cache.h"


/* Here is an our naive implementation */
char rotate_descr[] = "Naive Row-wise Traversal of src";
void rotate(int dim, pixel *src, pixel *dst) {
	int i, j, k, l;

//	for(i=0; i < dim; i++) {
//		for(j=0; j < dim; j++) {
//			COPY(&dst[PIXEL(dim-1-j,i,dim)], &src[PIXEL(i,j,dim)]);
//		}
//	}

    for(i=0;i<dim;i+=4)//展开步长为4
        for(j=0;j<dim;j+=4)
            for(k=i;k<i+4;++k)
                for(l=j;l<j+4;++l)
                    COPY(&dst[PIXEL(dim-1-l, k, dim)], &src[PIXEL(k,l,dim)]);


	return;
}


/* Add additional functions to test here */
void register_rotate_functions() {
	add_rotate_function(&rotate, rotate_descr);
	
}

