#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"
#include "linker.h"

/*output usage*/
void display_usage(char *file)
{
	fprintf(stderr, "Usage: %s -t test_num -f filename\n", file);
	fprintf(stderr, "test_num must be from 1 to 9\n");
	fprintf(stderr, "filename which you want to test, it must be exit!\n");
}

int
main(int argc, char *argv[])
{
	int opt;
	int test_num;   //which file test, it must between 1 and 9
	char *file;  //the name of file that we test, you can find all test files in the "test" floder
	while((opt=getopt(argc, argv, "t:f:"))!=-1)
	{
		switch(opt)
		{
		case 't':
			test_num = atoi(optarg);
			if(test_num<=0||test_num>9)
			{
				display_usage(argv[0]);
				exit(-1);
			}
			break;
		case 'f':
			file = optarg;
			if(file==NULL)
			{
				display_usage(argv[0]);
				exit(-1);
			}
			break;
		default: /*'?'*/
			display_usage(argv[0]);
			exit(-1);	
		}
	}
	//link module
	link(test_num, file);
	return 0;
}
