// bufoverflow.c

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Like gets, except that characters are typed as pairs of hex digits.
   Nondigit characters are ignored.  Stops when encounters newline */
char *getxs(char *dest)
{
  int c;
  int even = 1; /* Have read even number of digits */
  int otherd = 0; /* Other hex digit of pair */
  char *sp = dest;
  while ((c = getchar()) != EOF && c != '\n') {
    if (isxdigit(c)) {
      int val;
      if ('0' <= c && c <= '9')
	val = c - '0';
      else if ('A' <= c && c <= 'F')
	val = c - 'A' + 10;
      else
	val = c - 'a' + 10;
      if (even) {
	otherd = val;
	even = 0;
      } else {
	*sp++ = otherd * 16 + val;
	even = 1;
      }
    }
  }
  *sp++ = '\0';
  return dest;
}

int getbuf()
{
  char buf[32];
  getxs(buf);
  return 1;
}

void test()
{
  int val;
  printf("Type Hex string:");
  val = getbuf();
  printf("getbuf returned 0x%x\n", val);
}

int main()
{
  int buf[32];
  /* This little hack is an attempt to get the stack to be in a
     stable position
  */
  int offset = (((int) buf) & 0xFFF);   
  int *space = (int *) malloc(offset);  
  *space = 0; /* So that don't get complaint of unused variable */
  test();
  return 0;
}

