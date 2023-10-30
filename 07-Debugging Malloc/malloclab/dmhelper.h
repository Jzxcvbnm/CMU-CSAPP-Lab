#ifndef DMHELPER_H
#define DMHELPER_H

/* Prints out an error message in a readable format, given
		the message, filename and line number, then exits.
	First checks to make sure '\n' does not exist in
		msg and filename.
	Returns 0 on success, -1 on failure. */
int error(int errorCode, char *filename, int linenumber);

/* Prints out an error message in a readable format, given
		the message, filename and line number, then exits.
	First checks to make sure '\n' does not exist in
		msg and filename.
	Returns 0 on success, -1 on failure. */
int errorfl(int errorCode, char *filename_malloc, int linenumber_malloc, char *filename_free, int linenumber_free);

char *getMsg(int error);
#define PRINTBLOCK(s, f, l)	printf("\t%d bytes, created at %s, line %d\n", s, f, l)
#define PRINTERROR(e, f, l) printf("Error: %s\n\tInvalid block created at %s, line %d\n", getMsg(e), f, l);


#endif