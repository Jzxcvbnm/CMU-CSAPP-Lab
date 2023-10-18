#include <iostream>
#include<gtest/gtest.h>
#include "fixedpoint.h"

using namespace std;

class JTestFixture:public testing::Test {
protected:
   // static void SetUpTestCase();
   // static void TearDownTestCase();

    virtual void SetUp() override;
   // virtual void TearDown() override;

    Fixedpoint zero, one, one_half,one_fourth,large1,large2,max;

};

void JTestFixture::SetUp() {
    zero = fixedpoint_create(0UL);
    one = fixedpoint_create(1UL);
    one_half = fixedpoint_create2(0UL, 0x8000000000000000UL);
    one_fourth = fixedpoint_create2(0UL, 0x4000000000000000UL);
    large1 = fixedpoint_create2(0x4b19efceaUL, 0xec9a1e2418UL);
    large2 = fixedpoint_create2(0xfcbf3d5UL, 0x4d1a23c24fafUL);
    max = fixedpoint_create2(0xFFFFFFFFFFFFFFFFUL, 0xFFFFFFFFFFFFFFFFUL);

}

TEST_F(JTestFixture, test_fixedpoint_whole_part)
{
    EXPECT_EQ(0UL, fixedpoint_whole_part(zero));
    EXPECT_EQ(1UL, fixedpoint_whole_part(one));
    EXPECT_EQ(0UL, fixedpoint_whole_part(one_half));
    EXPECT_EQ(0UL, fixedpoint_whole_part(one_fourth));
    EXPECT_EQ(0x4b19efceaUL, fixedpoint_whole_part(large1));
    EXPECT_EQ(0xfcbf3d5UL, fixedpoint_whole_part(large2));
}

TEST_F(JTestFixture, test_fixedpoint_frac_part)
{
  ASSERT_TRUE(0UL == fixedpoint_frac_part(zero));
  ASSERT_TRUE(0UL == fixedpoint_frac_part(one));
  ASSERT_TRUE(0x8000000000000000UL == fixedpoint_frac_part(one_half));
  ASSERT_TRUE(0x4000000000000000UL == fixedpoint_frac_part(one_fourth));
  ASSERT_TRUE(0xec9a1e2418UL == fixedpoint_frac_part(large1));
  ASSERT_TRUE(0x4d1a23c24fafUL == fixedpoint_frac_part(large2));
}

TEST_F(JTestFixture, test_create_from_hex)
{
  Fixedpoint val1 = fixedpoint_create_from_hex("f6a5865.00f2");
  ASSERT_TRUE(fixedpoint_is_valid(val1));
  ASSERT_TRUE(0xf6a5865UL == fixedpoint_whole_part(val1));
  ASSERT_TRUE(0x00f2000000000000UL == fixedpoint_frac_part(val1));
}

TEST_F(JTestFixture, test_format_as_hex)
{
  char *s;

  s = fixedpoint_format_as_hex(zero);
  ASSERT_TRUE(0 == strcmp(s, "0"));
  free(s);

  s = fixedpoint_format_as_hex(one);
  ASSERT_TRUE(0 == strcmp(s, "1"));
  free(s);

  s = fixedpoint_format_as_hex(one_half);
  ASSERT_TRUE(0 == strcmp(s, "0.8"));
  free(s);

  s = fixedpoint_format_as_hex(one_fourth);
  ASSERT_TRUE(0 == strcmp(s, "0.4"));
  free(s);

  s = fixedpoint_format_as_hex(large1);
  ASSERT_TRUE(0 == strcmp(s, "4b19efcea.000000ec9a1e2418"));
  free(s);

  s = fixedpoint_format_as_hex(large2);
  ASSERT_TRUE(0 == strcmp(s, "fcbf3d5.00004d1a23c24faf"));
  free(s);
}

TEST_F(JTestFixture,  test_negate) {
  // none of the test fixture objects are negative
  ASSERT_TRUE(!fixedpoint_is_neg(zero));
  ASSERT_TRUE(!fixedpoint_is_neg(one));
  ASSERT_TRUE(!fixedpoint_is_neg(one_half));
  ASSERT_TRUE(!fixedpoint_is_neg(one_fourth));
  ASSERT_TRUE(!fixedpoint_is_neg(large1));
  ASSERT_TRUE(!fixedpoint_is_neg(large2));

  // negate the test fixture values
  Fixedpoint zero_neg = fixedpoint_negate(zero);
  Fixedpoint one_neg = fixedpoint_negate(one);
  Fixedpoint one_half_neg = fixedpoint_negate(one_half);
  Fixedpoint one_fourth_neg = fixedpoint_negate(one_fourth);
  Fixedpoint large1_neg = fixedpoint_negate(large1);
  Fixedpoint large2_neg = fixedpoint_negate(large2);

  // zero does not become negative when negated
  ASSERT_TRUE(!fixedpoint_is_neg(zero_neg));

  // all of the other values should have become negative when negated
  ASSERT_TRUE(fixedpoint_is_neg(one_neg));
  ASSERT_TRUE(fixedpoint_is_neg(one_half_neg));
  ASSERT_TRUE(fixedpoint_is_neg(one_fourth_neg));
  ASSERT_TRUE(fixedpoint_is_neg(large1_neg));
  ASSERT_TRUE(fixedpoint_is_neg(large2_neg));

  // magnitudes should stay the same
  ASSERT_TRUE(0UL == fixedpoint_whole_part(zero));
  ASSERT_TRUE(1UL == fixedpoint_whole_part(one));
  ASSERT_TRUE(0UL == fixedpoint_whole_part(one_half));
  ASSERT_TRUE(0UL == fixedpoint_whole_part(one_fourth));
  ASSERT_TRUE(0x4b19efceaUL == fixedpoint_whole_part(large1));
  ASSERT_TRUE(0xfcbf3d5UL == fixedpoint_whole_part(large2));
  ASSERT_TRUE(0UL == fixedpoint_frac_part(zero));
  ASSERT_TRUE(0UL == fixedpoint_frac_part(one));
  ASSERT_TRUE(0x8000000000000000UL == fixedpoint_frac_part(one_half));
  ASSERT_TRUE(0x4000000000000000UL == fixedpoint_frac_part(one_fourth));
  ASSERT_TRUE(0xec9a1e2418UL == fixedpoint_frac_part(large1));
  ASSERT_TRUE(0x4d1a23c24fafUL == fixedpoint_frac_part(large2));
}

TEST_F(JTestFixture,  test_add) {

  Fixedpoint lhs, rhs, sum;

  lhs = fixedpoint_create_from_hex("-c7252a193ae07.7a51de9ea0538c5");
  rhs = fixedpoint_create_from_hex("d09079.1e6d601");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_neg(sum));
  ASSERT_TRUE(0xc7252a0c31d8eUL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x5be47e8ea0538c50UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("0");
  rhs = fixedpoint_create_from_hex("0");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(!fixedpoint_is_neg(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("4");
  rhs = fixedpoint_create_from_hex("-1");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(!fixedpoint_is_neg(sum));
  ASSERT_TRUE(0x3UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("934.ade8d38a");
  rhs = fixedpoint_create_from_hex("-edef814.21f023189");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_neg(sum));
  ASSERT_TRUE(0xedeeedfUL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x74074f8e90000000UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("8bd.0e34492025065");
  rhs = fixedpoint_create_from_hex("5d7b061d6.034f5d");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(!fixedpoint_is_neg(sum));
  ASSERT_TRUE(0x5d7b06a93UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x1183a62025065000UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("-8a6a9f92d72.82a9b99ad4e76");
  rhs = fixedpoint_create_from_hex("-8a93a62c25996.e09875");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_neg(sum));
  ASSERT_TRUE(0x8b1e10cbb8709UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x63422e9ad4e76000UL == fixedpoint_frac_part(sum));

  // to do: add more test case
}

TEST_F(JTestFixture,   test_sub) {


  Fixedpoint lhs, rhs, diff;

  lhs = fixedpoint_create_from_hex("-ccf35aa3a04a3b.b105");
  rhs = fixedpoint_create_from_hex("f676e8.58");
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_neg(diff));
  ASSERT_TRUE(0xccf35aa496c124UL == fixedpoint_whole_part(diff));
  ASSERT_TRUE(0x0905000000000000UL == fixedpoint_frac_part(diff));

  // more test
  lhs = fixedpoint_create_from_hex("0");
  rhs = fixedpoint_create_from_hex("0");
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT_TRUE(!fixedpoint_is_neg(diff));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(diff));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(diff));

  lhs = fixedpoint_create_from_hex("0");
  rhs = fixedpoint_create_from_hex("1");
  diff = fixedpoint_sub(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_neg(diff));
  ASSERT_TRUE(0x1UL == fixedpoint_whole_part(diff));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(diff));
}

TEST_F(JTestFixture,  test_is_overflow_pos) {
  Fixedpoint sum;

  sum = fixedpoint_add(max, one);
  ASSERT_TRUE(fixedpoint_is_overflow_pos(sum));

  sum = fixedpoint_add(one, max);
  ASSERT_TRUE(fixedpoint_is_overflow_pos(sum));

  Fixedpoint negative_one = fixedpoint_negate(one);

  sum = fixedpoint_sub(max, negative_one);
  ASSERT_TRUE(fixedpoint_is_overflow_pos(sum));

  // more test
  Fixedpoint lhs, rhs;
  lhs = fixedpoint_create_from_hex("ffffffffffffffff.f");
  rhs = fixedpoint_create_from_hex("0.1");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_overflow_pos(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("1.1");
  rhs = fixedpoint_create_from_hex("ffffffffffffffff.f");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_overflow_pos(sum));
  ASSERT_TRUE(0x1UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("0.1");
  rhs = fixedpoint_create_from_hex("ffffffffffffffff.f");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_overflow_pos(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

}

TEST_F(JTestFixture, test_is_err) {

  // too many characters
  Fixedpoint err1 = fixedpoint_create_from_hex("88888888888888889.6666666666666666");
  ASSERT_TRUE(fixedpoint_is_err(err1));

  // too many characters
  Fixedpoint err2 = fixedpoint_create_from_hex("6666666666666666.88888888888888889");
  ASSERT_TRUE(fixedpoint_is_err(err2));

  // this one is actually fine
  Fixedpoint err3 = fixedpoint_create_from_hex("-6666666666666666.8888888888888888");
  ASSERT_TRUE(fixedpoint_is_valid(err3));
  ASSERT_TRUE(!fixedpoint_is_err(err3));

  // whole part is too large
  Fixedpoint err4 = fixedpoint_create_from_hex("88888888888888889");
  ASSERT_TRUE(fixedpoint_is_err(err4));

  // fractional part is too large
  Fixedpoint err5 = fixedpoint_create_from_hex("7.88888888888888889");
  ASSERT_TRUE(fixedpoint_is_err(err5));

  // invalid hex digits in whole part
  Fixedpoint err6 = fixedpoint_create_from_hex("123xabc.4");
  ASSERT_TRUE(fixedpoint_is_err(err6));

  // invalid hex digits in fractional part
  Fixedpoint err7 = fixedpoint_create_from_hex("7.0?4");
  ASSERT_TRUE(fixedpoint_is_err(err7));
}


// TODO: implement more test functions
TEST_F(JTestFixture, test_is_overflow_neg) {
  Fixedpoint sum;

  Fixedpoint negative_one = fixedpoint_negate(one);
  sum = fixedpoint_sub(negative_one, max );
  ASSERT_TRUE(fixedpoint_is_overflow_neg(sum));

  Fixedpoint lhs, rhs;
  lhs = fixedpoint_create_from_hex("-ffffffffffffffff.f");
  rhs = fixedpoint_create_from_hex("-0.1");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_overflow_neg(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("-1.1");
  rhs = fixedpoint_create_from_hex("-ffffffffffffffff.f");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_overflow_neg(sum));
  ASSERT_TRUE(0x1UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("-0.1");
  rhs = fixedpoint_create_from_hex("-ffffffffffffffff.f");
  sum = fixedpoint_add(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_overflow_neg(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));

  lhs = fixedpoint_create_from_hex("-0.1");
  rhs = fixedpoint_create_from_hex("ffffffffffffffff.f");
  sum = fixedpoint_sub(lhs, rhs);
  ASSERT_TRUE(fixedpoint_is_overflow_neg(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(sum));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(sum));
}

// TODO: implement more test functions
TEST_F(JTestFixture,  test_halve) {

  Fixedpoint halve;

  halve = fixedpoint_halve(zero);
  ASSERT_TRUE(!fixedpoint_is_neg(halve));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part(halve));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part(halve));

  halve = fixedpoint_halve(one);
  ASSERT_TRUE(!fixedpoint_is_neg(halve));
  ASSERT_TRUE(fixedpoint_whole_part(one_half) == fixedpoint_whole_part(halve));
  ASSERT_TRUE(fixedpoint_frac_part(one_half) == fixedpoint_frac_part(halve));

  halve = fixedpoint_halve(one_half);
  ASSERT_TRUE(!fixedpoint_is_neg(halve));
  ASSERT_TRUE(fixedpoint_whole_part(one_fourth) == fixedpoint_whole_part(halve));
  ASSERT_TRUE(fixedpoint_frac_part(one_fourth) == fixedpoint_frac_part(halve));
}

TEST_F(JTestFixture, test_double) {

  Fixedpoint fp_double;

  fp_double = fixedpoint_double(zero);
  ASSERT_TRUE(!fixedpoint_is_neg( fp_double ));
  ASSERT_TRUE(0x0UL == fixedpoint_whole_part( fp_double ));
  ASSERT_TRUE(0x0UL == fixedpoint_frac_part( fp_double ));

  fp_double = fixedpoint_double(one_half);
  ASSERT_TRUE(!fixedpoint_is_neg( fp_double ));
  ASSERT_TRUE(fixedpoint_whole_part(one) == fixedpoint_whole_part( fp_double ));
  ASSERT_TRUE(fixedpoint_frac_part(one) == fixedpoint_frac_part( fp_double ));

}

TEST_F(JTestFixture, test_is_underflow_pos) {

  Fixedpoint fp;
  fp = fixedpoint_halve(zero);
  ASSERT_TRUE(!fixedpoint_is_underflow_pos(fp));

  fp = fixedpoint_halve(one);
  ASSERT_TRUE(!fixedpoint_is_underflow_pos(fp));

  fp = fixedpoint_halve(one_half);
  ASSERT_TRUE(!fixedpoint_is_underflow_pos(fp));

  fp = fixedpoint_halve(max);
  ASSERT_TRUE(fixedpoint_is_underflow_pos(fp));

}

TEST_F(JTestFixture, test_is_underflow_neg) {

  Fixedpoint fp;
  fp = fixedpoint_halve(zero);
  ASSERT_TRUE(!fixedpoint_is_underflow_neg(fp));

  fp = fixedpoint_halve(one);
  ASSERT_TRUE(!fixedpoint_is_underflow_neg(fp));

  fp = fixedpoint_halve(one_half);
  ASSERT_TRUE(!fixedpoint_is_underflow_neg(fp));

  fp = fixedpoint_halve(fixedpoint_negate(max));
  ASSERT_TRUE(fixedpoint_is_underflow_neg(fp));

}



int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
