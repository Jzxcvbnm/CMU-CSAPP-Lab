/* Testing Code */

#include <limits.h>




int test_bitAnd(int x, int y)
{
  return x&y;
}






int test_bitOr(int x, int y)
{
  return x|y;
}






int test_isZero(int x) {
  return x == 0;
}






int test_minusOne(void) {
  return -1;
}






int test_tmax(void) {
  return LONG_MAX;
}







int test_bitXor(int x, int y)
{
  return x^y;
}






int test_getByte(int x, int n)
{
  union  {
    int word;
    unsigned char bytes[4];
  } u;
  int test = 1;
  int littleEndian = (int) *(char *) &test;
  u.word = x;
  return littleEndian ? (unsigned) u.bytes[n] : (unsigned) u.bytes[3-n];
}






int test_isEqual(int x, int y)
{
  return x == y; 
}






int test_negate(int x) {
  return -x;
}






int test_isPositive(int x) {
  return x > 0;
}


