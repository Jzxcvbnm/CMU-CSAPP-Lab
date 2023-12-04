/* math -- instructions for Machine */

#include "flincl.h"
#include "math.h"

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

/* get_long -- get an integer value from FVal,
 *  assumes that p is not NULL
 */
        

void Machine::op_truncate()
{
    FUnion p = read_tos();
    long pval;
    if (p.node) {
        if (p.node->has_tag(TAG_LONG)) {
            // do nothing
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            pval = (long) p.fdouble->get_double();
            pop();
            push(FLong::create(pval, this));
        } else not_a_number();
    } else not_a_number();
}


void Machine::op_float()
{
    FUnion p = read_tos();
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            replace(FDouble::create((double) pval, this));
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            // do nothing
        } else not_a_number();
    } else not_a_number();
}


void Machine::op_round()
{
    FUnion p = read_tos();
    int64 pval;
    if (p.node) {
        if (p.node->has_tag(TAG_LONG)) {
            // do nothing
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            pval = (long) (p.fdouble->get_double() + 0.5);
            replace(FLong::create(pval, this));
        } else not_a_number();
    } else not_a_number();
}


void Machine::op_inc()
{
    FUnion p = read_tos();
	FVal newp;
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            newp = FLong::create(pval + 1, this);
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            newp = FDouble::create(p.fdouble->get_double() + 1.0, this);
        } else {
            newp = NULL;
            not_a_number();
        }
    }
    replace(newp);
}


void Machine::op_dec()
{
    FUnion p = read_tos();
	FVal newp = NULL;
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            newp = FLong::create(pval - 1, this);
        } else if (p.node->has_tag(TAG_DOUBLE)) {
            newp = FDouble::create(p.fdouble->get_double() - 1.0, this);
        } else {
            not_a_number();
        }
    }
    replace(newp);
}

#define abs64(x) ((x) < 0 ? -(x) : x)

void Machine::op_abs()
{
    FUnion p = read_tos();
	FVal newp = NULL;
    int64 pval;
    if (p.node) {
        if (p.flong->test_long(&pval)) {
            newp = FLong::create(abs64(pval), this);
        } else if (p.node->has_tag(TAG_DOUBLE)) {
			double d = p.fdouble->get_double();
			if (d < 0) d = -d;
            newp = FDouble::create(d, this);
        } else {
            not_a_number();
        }
    }
    replace(newp);
}


void Machine::op_sin()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::create(sin(d), this));
    } else not_a_float();
}


void Machine::op_cos()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::create(cos(d), this));
    } else not_a_float();
}


void Machine::op_tan()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::create(tan(d), this));
    } else not_a_float();
}


void Machine::op_exp()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::create(exp(d), this));
    } else not_a_float();
}


void Machine::op_sqrt()
{
    FDouble_ptr p = read_tos().fdouble;
	double d;
    if (p->test_double(&d)) {
        replace(FDouble::create(sqrt(d), this));
    } else not_a_float();
}


void Machine::op_lognot()
{
    FLong_ptr p = read_tos().flong;
	int64 i;
    if (p->test_long(&i)) {
        replace(FLong::create(~i, this));
    } else not_an_integer();
}


void Machine::op_minus()
{
    FUnion p = read_tos();
	FVal newp;
    int64 i;
	double d;
    if (p.node) {
        if (p.flong->test_long(&i)) {
            newp = FLong::create(-i, this);
        } else if (p.fdouble->test_double(&d)) {
            newp = FDouble::create(-d, this);
        } else {
            newp = NULL;
            not_a_number();
        }
    }
    replace(newp);
}


void Machine::op_bool2int()
{
	Symbol_ptr p = read_tos().symbol;
	if (p == NULL) replace(FLong::create(0, this));
	else if (p == s_true) replace(FLong::create(1, this));
	else not_a_boolean();
}


void Machine::op_int2bool()
{
    FLong_ptr p = read_tos().flong;
	int64 i;
    if (p->test_long(&i)) {
        replace(i ? s_true : NULL);
    } else not_an_integer();
}


/* BINARY OPERATORS */


void Machine::op_add()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::create(p1val + p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1val + p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::create(p1.fdouble->get_double() + p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1.fdouble->get_double() + p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_ARRAY) && p2.node->has_tag(TAG_ARRAY)) {
            res = p1.array->concatenate(p2.array, this);
        } else if (p1.node->has_tag(TAG_STRING) && p2.node->has_tag(TAG_STRING)) {
            res = p1.fstring->concatenate(p2.fstring, this);
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_sub()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::create(p1val - p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1val - p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::create(p1.fdouble->get_double() - p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1.fdouble->get_double() - 
                                          p2.fdouble->get_double(), this);
            } else not_a_number();
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_mul()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::create(p1val * p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1val * p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::create(p1.fdouble->get_double() * p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1.fdouble->get_double() * 
                                          p2.fdouble->get_double(), this);
            } else not_a_number();
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_power()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 l;
    double d1, d2;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&l)) {
            d1 = (double) l;
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            d1 = p1.fdouble->get_double();
        } else {
            not_a_number();
            return;
        }
        if (p2.flong->test_long(&l)) {
            d2 = (double) l;
        } else if (p2.node->has_tag(TAG_DOUBLE)) {
            d2 = p2.fdouble->get_double();
        } else {
            not_a_number();
            return;
        }
        pop();
        replace(FDouble::create(pow(d1, d2), this));
    } else not_a_number();
}


void Machine::op_div()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.node && p2.node) {
        if (p1.flong->test_long(&p1val)) {
            if (p2.flong->test_long(&p2val)) {
                res = FLong::create(p1val / p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1val / p2.fdouble->get_double(), this);
            } else not_a_number();
        } else if (p1.node->has_tag(TAG_DOUBLE)) {
            if (p2.flong->test_long(&p2val)) {
                res = FDouble::create(p1.fdouble->get_double() / p2val, this);
            } else if (p2.node->has_tag(TAG_DOUBLE)) {
                res = FDouble::create(p1.fdouble->get_double() / 
                                          p2.fdouble->get_double(), this);
            } else not_a_number();
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_rem()
{
    FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 p1val, p2val;
    FVal res = NULL;
    if (p1.flong->test_long(&p1val) && p2.flong->test_long(&p2val)) {
        res = FLong::create(p1val % p2val, this);
		pop();
		replace(res);
		return;
	}
	not_a_number();
    pop();
}


void Machine::op_min()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            res = FLong::create(min(i1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::create(min(i1, d2), this);
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            res = FDouble::create(min(d1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::create(min(d1, d2), this);
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_max()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            res = FLong::create(max(i1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::create(max(i1, d2), this);
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            res = FDouble::create(max(d1, i2), this);
        } else if (p2.fdouble->test_double(&d2)) {
            res = FDouble::create(max(d1, d2), this);
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}

void Machine::op_logand()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::create((long) (i1 & i2), this);
	}
	pop();
	replace(res);
}


void Machine::op_rshift()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::create(i1 >> i2, this);
	} else p1->get_long_error(this);
	pop();
	replace(res);
}


void Machine::op_lshift()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::create(i1 << i2, this);
	} else p1->get_long_error(this);
	pop();
	replace(res);
}




void Machine::op_logior()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::create((long) (i1 | i2), this);
	}
	pop();
	replace(res);
}


void Machine::op_logxor()
{
	FLong_ptr p1 = read_next_tos().flong;
	FLong_ptr p2 = read_tos().flong;
	int64 i1, i2;
	FVal res = NULL;
	if (p1->test_long(&i1) && p2->test_long(&i2)) {
		res = FLong::create(i1 ^ i2, this);
	}
	pop();
	replace(res);
}


void Machine::op_less()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 < i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 < d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 < i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 < d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_lesseql()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 <= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 <= d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 <= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 <= d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_noteql()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 != i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 != d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 != i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 != d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_gtreql()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 >= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 >= d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 >= i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 >= d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}


void Machine::op_gtr()
{
	FUnion p1 = read_next_tos();
    FUnion p2 = read_tos();
    int64 i1, i2;
	double d1, d2;
    FVal res = NULL;
    if (p1.flong->test_long(&i1)) {
        if (p2.flong->test_long(&i2)) {
            if (i1 > i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (i1 > d2) res = s_true; 
        } else not_a_number();
    } else if (p1.fdouble->test_double(&d1)) {
        if (p2.flong->test_long(&i2)) {
            if (d1 > i2) res = s_true; 
        } else if (p2.fdouble->test_double(&d2)) {
            if (d1 > d2) res = s_true; 
        } else not_a_number();
    } else not_a_number();
    pop();
    replace(res);
}



/* NO OPERANDS */
void Machine::op_random() { illegal_instruction(); }


