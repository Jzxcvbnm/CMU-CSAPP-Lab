#include "dmhelper.h"
#include <stdio.h>
#include <stdlib.h>

/* returns 1 if c is in str, 0 if not */
int checkString(char *str, char c) {
	while (*str++) {
		if (*str == c) {
			return 1;
		}
	}
	return 0;
}

char *getMsg(int error) {
	switch(error) {
		case 1:
			return "Starting edge of the payload has been overwritten.";
		case 2:
			return "Ending edge of the payload has been overwritten.";
		case 3:
			return "Header has been corrupted.";
		case 4:
			return "Attempting to free an unallocated block.";
	}
	return "";
}

/* Prints out an error message in a readable format, given
		the message, filename and line number.
	First checks to make sure '\n' does not exist in
		msg and filename.
	Returns 0 on success, -1 on failure. */
int error(int errorCode, char *filename, int linenumber) {
	char *msg = getMsg(errorCode);
	if (checkString(filename, '\n')) {
		printf("Invalid filename\n");
		return -1;
	}
	printf("Error: %s\n\tin block freed at %s, line %d\n", msg, filename, linenumber);
	exit(-1);
}



int errorfl(int errorCode, char *filename_malloc, int linenumber_malloc, char *filename_free, int linenumber_free) {
	char *msg = getMsg(errorCode);
	if (checkString(filename_malloc, '\n')) {
		printf("Invalid \"malloc\" filename\n");
		return -1;
	}
	if (checkString(filename_free, '\n')) {
		printf("Invalid \"free\" filename\n");
		return -1;
	}
	printf("Error: %s\n\tin block allocated at %s, line %d\n", msg, filename_malloc, linenumber_malloc);
	printf("\tand freed at %s, line %d\n", filename_free, linenumber_free);
	exit(-1);
}
