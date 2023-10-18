#ifndef FIXEDPREC_H
#define FIXEDPREC_H

#include <stdint.h>

typedef struct {
  // TODO: add field
      uint64_t whole;
    uint64_t frac;
    int flag;
    //0有效非负(whole&(1<<63))>>63
    //1有效负数
    //2错误
    //4正上溢出
    //5负下溢出
    //6正下溢出
    //7负上溢出
} Fixedpoint;

// Create a Fixedpoint value representing an integer.
//
// Parameters:
//   whole - the value of the whole part of the representation
//
// Returns:
//   the Fixedpoint value
Fixedpoint fixedpoint_create(uint64_t whole);


// Create a Fixedpoint value from specified whole and fractional values.
//
// Parametrs:
//   whole - the value of the whole part of the representation
//   frac - the value of the fractional part of the representation, where
//          the highest bit is the halves (2^-1) place, the second highest
//          bit is the fourths (2^-2) place, etc.
//
// Returns:
//   the Fixedpoint value
Fixedpoint fixedpoint_create2(uint64_t whole, uint64_t frac);

// Create a Fixedpoint value from a string representation.
// The string will have one of the following forms:
//
//    X
//    -X
//    X.Y
//    -X.Y
//
// In all value strings, X and Y are sequences of 0 to 16 hex digits
// (chosen from 0-9, a-f, A-F).
//
// Returns:
//   if the string is valid, the Fixedpoint value;
//   if the string is invalid, a Fixedpoint value for which
//   fixedpoint_is_err returns true
Fixedpoint fixedpoint_create_from_hex(const char *hex);

// Get the whole part of the given Fixedpoint value.
//
// Parameters:
//   val - a valid Fixedpoint value
//
// Returns:
//   a uint64_t value which is the whole part of the Fixedpoint value
uint64_t fixedpoint_whole_part(Fixedpoint val);

// Get the fractional part of the given Fixedpoint value.
//
// Parameters:
//   val - a valid Fixedpoint value
//
// Returns:
//   a uint64_t value which is the fractional part of the Fixedpoint value
uint64_t fixedpoint_frac_part(Fixedpoint val);

// Compute the sum of two valid Fixedpoint values.
//
// Parameters:
//   left - the left Fixedpoint value
//   right - the right Fixedpoint value
//
// Returns:
//   if the sum left + right is in the range of values that can be represented
//   exactly, the sum is returned;
//   if the sum left + right is not in the range of values that can be
//   represented, then a value for which either fixedpoint_is_overflow_pos or
//   fixedpoint_is_overflow_neg returns true is returned (depending on whether
//   the overflow was positive or negative)
Fixedpoint fixedpoint_add(Fixedpoint left, Fixedpoint right);

// Compute the difference of two valid Fixedpoint values.
//
// Parameters:
//   left - the left Fixedpoint value
//   right - the right Fixedpoint value
//
// Returns:
//   if the difference left - right is in the range of values that can be represented
//   exactly, the difference is returned;
//   if the difference left - right is not in the range of values that can be
//   represented, then a value for which either fixedpoint_is_overflow_pos or
//   fixedpoint_is_overflow_neg returns true is returned (depending on whether
//   the overflow was positive or negative)
Fixedpoint fixedpoint_sub(Fixedpoint left, Fixedpoint right);

// Negate a valid Fixedpoint value.  (I.e. a value with the same magnitude but
// the opposite sign is returned.)  As a special case, the zero value is considered
// to be its own negation.
//
// Parameters:
//   val - a valid Fixedpoint value
//
// Returns:
//   the negation of val
Fixedpoint fixedpoint_negate(Fixedpoint val);

// Return a Fixedpoint value that is exactly 1/2 the value of the given one.
//
// Parameters:
//   val - a valid Fixedpoint value
//
// Return:
//   a Fixedpoint value exactly 1/2 of the given one, if it can be represented exactly;
//   otherwise, a Fixedpoint value for which either fixedpoint_is_underflow_pos
//   or fixedpoint_is_underflow_neg returns true (depending on whether the
//   computed value would have been positive or negative)
Fixedpoint fixedpoint_halve(Fixedpoint val);

// Return a Fixedpoint value that is exactly twice the value of the given one.
//
// Parameters:
//   val - a valid Fixedpoint value
//
// Return:
//   a Fixedpoint value exactly twice the given one, if it can be represented exactly;
//   otherwise, a Fixedpoint value for which either fixedpoint_is_overflow_pos
//   or fixedpoint_is_overflow_neg returns true (depending on whether the
//   computed value would have been positive or negative)
Fixedpoint fixedpoint_double(Fixedpoint val);

// Compare two valid Fixedpoint values.
//
// Parameters:
//   left - the left Fixedpoint value
//   right - the right Fixedpoint value
//
// Returns:
//    -1 if left < right;
//     0 if left == right;
//     1 if left > right
int fixedpoint_compare(Fixedpoint left, Fixedpoint right);

// Determine whether a Fixedpoint value is equal to 0.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val is a valid Fixedpoint value equal to 0;
//   0 is val is not a valid Fixedpoint value equal to 0
int fixedpoint_is_zero(Fixedpoint val);

// Determine whether a Fixedpoint value is an "error" value resulting
// from a call to fixedpoint_create_from_hex for which the argument
// string was invalid.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val is the result of a call to fixedpoint_create_from_hex with
//   an invalid argument string;
//   0 otherwise
int fixedpoint_is_err(Fixedpoint val);

// Determine whether a Fixedpoint value is negative (less than 0).
//
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val is a valid value less than 0;
//   0 otherwise
int fixedpoint_is_neg(Fixedpoint val);

// Determine whether a Fixedpoint value is the result of negative overflow.
// Negative overflow results when a sum, difference, or product is negative
// and has a magnitude that is too large to represent.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val is the result of an operation where negative overflow occurred;
//   0 otherwise
int fixedpoint_is_overflow_neg(Fixedpoint val);

// Determine whether a Fixedpoint value is the result of positive overflow.
// Positive overflow results when a sum, difference, or product is positive
// and has a magnitude that is too large to represent.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val is the result of an operation where positive overflow occurred;
//   0 otherwise
int fixedpoint_is_overflow_pos(Fixedpoint val);

// Determine whether a Fixedpoint value is the result of negative underflow.
// Negative underflow occurs when a division (i.e., fixedpoint_halve)
// produces a value that is negative, and can't be exactly represented because
// the fractional part of the representation doesn't have enough bits.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val is the result of an operation where negative underflow occurred;
//   0 otherwise
int fixedpoint_is_underflow_neg(Fixedpoint val);

// Determine whether a Fixedpoint value is the result of positive underflow.
// Positive underflow occurs when a division (i.e., fixedpoint_halve)
// produces a value that is positive, and can't be exactly represented because
// the fractional part of the representation doesn't have enough bits.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val is the result of an operation where positive underflow occurred;
//   0 otherwise
int fixedpoint_is_underflow_pos(Fixedpoint val);

// Determine whether a Fixedpoint value represents a valid negative or non-negative number.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   1 if val represents a valid negative or non-negative number;
//   0 otherwise
int fixedpoint_is_valid(Fixedpoint val);

// Return a dynamically allocated C character string with the representation of
// the given valid Fixedpoint value.  The string should start with "-" if the
// value is negative, and should use the characters 0-9 and a-f to represent
// each hex digit of the whole and fractional parts. As a special case, if the
// Fixedpoint value represents an integer (i.e., the fractional part is 0),
// then no "decimal point" ('.') should be included.
//
// Parameters:
//   val - the Fixedpoint value
//
// Returns:
//   dynamically allocated character string containing the representation
//   of the Fixedpoint value
char *fixedpoint_format_as_hex(Fixedpoint val);

#endif // FIXEDPREC_H
