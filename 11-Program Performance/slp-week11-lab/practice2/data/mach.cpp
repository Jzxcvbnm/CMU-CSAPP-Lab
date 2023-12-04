/* mach.cpp -- execution machine
 */

#include "flincl.h"
//#include "classes.h"
#include "shortstack.h"
#include "semantics.h"
#include "compiler.h"

FVal s_true, s_false;
FVal s_symbol, s_long, s_double, s_string;
FVal s_array, s_dict, s_object, s_class;
FVal s_file;
FVal s_init;
Symbol_ptr s_global;
Symbol_ptr s_readline;
Symbol_ptr s_token;
Symbol_ptr s_close;
Symbol_ptr s_write;
Symbol_ptr s_readvalue;
Symbol_ptr s_read;
Symbol_ptr s_keys;
Symbol_ptr s_values;
Symbol_ptr s_clear;
Symbol_ptr s_remove;
Symbol_ptr s_copy;
Symbol_ptr s_has_key;
Symbol_ptr s_get;
Symbol_ptr s_get2;
Symbol_ptr s_chr;
Symbol_ptr s_ord;
Symbol_ptr s_intern;
Symbol_ptr s_hex;
Symbol_ptr s_oct;
Symbol_ptr s_id;
Symbol_ptr s_inst;
Symbol_ptr s_subcl;



typedef void (Machine::*Primitive)(void);

Primitive instr_decode[4096] = {
/* 0 */        Machine::op_noop,
/* 1 */        Machine::op_newobj,
/* 2 */        Machine::op_newarray,
/* 3 */        Machine::op_aref,
/* 4 */        Machine::op_dup,
/* 5 */        Machine::op_unappend,
/* 6 */        Machine::op_print,
/* 7 */        Machine::op_append,
/* 8 */        Machine::op_reverse,
/* 9 */        Machine::op_last,
/* 10 */       Machine::op_loopinc,
/* 11 */       Machine::op_looptst,
/* 12 */       Machine::op_member,
/* 13 */       Machine::op_loopnxt,
/* 14 */       Machine::op_notequal,
/* 15 */       Machine::op_length,
/* 16 */       Machine::op_cndand,
/* 17 */       Machine::op_cndor,
/* 18 */       Machine::op_isatom,
/* 19 */       Machine::op_issymbol,
/* 20 */       Machine::op_isnumber,
/* 21 */       Machine::op_isinteger,
/* 22 */       Machine::op_isfloat,
/* 23 */       Machine::op_isstring,
/* 24 */       Machine::op_isarray,
/* 25 */       Machine::op_isobject,
/* 26 */       Machine::op_isnull,
/* 27 */       Machine::op_newdict,
/* 28 */       Machine::op_isnegative,
/* 29 */       Machine::op_iszero,
/* 30 */       Machine::op_ispositive,
/* 31 */       Machine::op_iseven,
/* 32 */       Machine::op_isodd,
/* 33 */       Machine::op_iseql,
/* 34 */       Machine::op_isequal,
/* 35 */       Machine::op_jump,
/* 36 */       Machine::op_jumpifnot,
/* 37 */       Machine::op_truncate,
/* 38 */       Machine::op_float,
/* 39 */       Machine::op_round,
/* 40 */       Machine::op_add,
/* 41 */       Machine::op_sub,
/* 42 */       Machine::op_mul,
/* 43 */       Machine::op_div,
/* 44 */       Machine::op_inc,
/* 45 */       Machine::op_dec,
/* 46 */       Machine::op_rem,
/* 47 */       Machine::op_min,
/* 48 */       Machine::op_max,
/* 49 */       Machine::op_abs,
/* 50 */       Machine::op_sin,
/* 51 */       Machine::op_cos,
/* 52 */       Machine::op_tan,
/* 53 */       Machine::op_exp,
/* 54 */       Machine::op_sqrt,
/* 55 */       Machine::op_random,
/* 56 */       Machine::op_logand,
/* 57 */       Machine::op_logior,
/* 58 */       Machine::op_logxor,
/* 59 */       Machine::op_lognot,
/* 60 */       Machine::op_less,
/* 61 */       Machine::op_lesseql,
/* 62 */       Machine::op_noteql,
/* 63 */       Machine::op_gtreql,
/* 64 */       Machine::op_gtr,
/* 65 */       Machine::op_strcat,
/* 66 */       Machine::op_subseq,
/* 67 */       Machine::op_keys,
/* 68 */       Machine::op_power,
/* 69 */       Machine::op_type,
/* 70 */       Machine::op_is,
/* 71 */       Machine::op_isnot,
/* 72 */       Machine::op_send,
/* 73 */       Machine::op_super,
/* 74 */       Machine::op_noop,
/* 75 */       Machine::op_noop,
/* 76 */       Machine::op_isupper,
/* 77 */       Machine::op_islower,
/* 78 */       Machine::op_isalpha,
/* 79 */       Machine::op_isdigit,
/* 80 */       Machine::op_isalnum,
/* 81 */       Machine::op_toupper,
/* 82 */       Machine::op_tolower,
/* 83 */       Machine::op_pop2,
/* 84 */       Machine::op_pop,
/* 85 */       Machine::op_param,
/* 86 */       Machine::op_call,
/* 87 */       Machine::op_return,
/* 88 */       Machine::op_retnull,
/* 89 */       Machine::op_setaref,
/* 90 */	   Machine::op_xcall,
/* 91 */	   Machine::op_endl,
/* 92 */       Machine::op_setfield,
/* 93 */       Machine::op_jumpif,
/* 94 */       Machine::op_bool2int,
/* 95 */       Machine::op_int2bool,
/* 96 */       Machine::op_minus,
/* 97 */       Machine::op_field,
/* 98 */       Machine::op_readline,
/* 99 */       Machine::op_repr,
/* 100 */      Machine::op_flatten,
/* 101 */      Machine::op_str,
/* 102 */      Machine::op_token,
/* 103 */      Machine::op_close,
/* 104 */      Machine::op_write,
/* 105 */      Machine::op_readval,
/* 106 */      Machine::op_read,
/* 107 */      Machine::op_notmember,
/* 108 */      Machine::op_not,
/* 109 */      Machine::op_rshift,
/* 110 */      Machine::op_lshift,
/* 111 */      Machine::op_index,
/* 112 */      Machine::op_load,
/* 113 */      Machine::op_sort,
/* 114 */      Machine::op_values,
/* 115 */      Machine::op_clear,
/* 116 */      Machine::op_remove,
/* 117 */      Machine::op_copy,
/* 118 */      Machine::op_haskey,
/* 119 */      Machine::op_get,
/* 120 */      Machine::op_get2,
/* 121 */      Machine::op_chr,
/* 122 */      Machine::op_ord,
/* 123 */      Machine::op_intern,
/* 124 */      Machine::op_hex,
/* 125 */      Machine::op_oct,
/* 126 */      Machine::op_id,
/* 127 */      Machine::op_inst,
/* 128 */      Machine::op_subcl,
/* 129 */      Machine::op_find,
/* 130 */      Machine::op_keyparam,
/* 131 */      Machine::op_space,
/* 132 */	   Machine::op_exit,
/* 133 */	   Machine::op_frame_previous,
/* 134 */	   Machine::op_frame_variables,
/* 135 */	   Machine::op_frame_pc,
/* 136 */	   Machine::op_frame_method,
/* 137 */	   Machine::op_frame_class,
/* 138 */	   Machine::op_except_nest,
/* 139 */	   Machine::op_get_frame,
/* 140 */	   Machine::op_unread,
/* 141 */	   Machine::op_value
};

Machine::Machine()
{
    tos = expr_stack;
    frame = NULL;
    call_stack = NULL;
    symbols = Dict::create(this);
    pc = NULL;
    running = true;
    error_flag = false;
	trace_flag = false;
	callin_method = NULL;
///	classes = Classes::create(this);
}


char *type_string[] = { "free  ", "symb  ", "long  ", "doub  ", "string", "array ", "data  ",
					    "dict  ", "frame ", "meth  ", "obj   ", "class ", "symbs ", "entry ",
						"code  ", "locs  ", "builtn", "file  " };

void Machine::execute()
{
	running = true;
	error_flag = false;
    while (running) {
        uint16 instr = *pc++;
        int arg = instr_arg(instr);
		if (trace_flag) {
			char text[64];
			// show the stack
			FUnion *s = tos - 1;
			while (s >= expr_stack) {
				if (s->node) {
				    trace(" %s", type_string[s->node->get_tag()]);
					if (s->node->get_tag() == TAG_LONG) {
						trace("%I64d", s->flong->int64_data);
					} else if (s->node->get_tag() == TAG_DOUBLE) {
                        trace("%g", s->fdouble->double_data);
                    }
				} else trace(" null  ");
				s--;
			}
			trace("\n");
			// disassemble the instruction
			Code_ptr c = frame->get_method()->get_code();
            int len;
            trace(c->disassemble(pc - c->initial_pc() - 1, 
                                 instr, *pc, text, &len, 
                                 frame->get_method()));
		}
        switch (instr_type(instr)) {
          case instr_op:
	        (this->*(instr_decode[arg]))();
            break;
          case instr_load_global:
            push(frame->get_method()->symbol(arg)->get_value().node);
            break;
          case instr_store_global:
            frame->get_method()->symbol(arg)->set_value(pop().node, this);
            break;
          case instr_load_instvar:
            push(frame->get_self()->instvar(arg).node);
            break;
          case instr_store_instvar:
            frame->get_self()->set_instvar(arg, pop().node, this);
            break;
          case instr_load_local:
            push(frame->local(arg).node);
            break;
          case instr_store_local:
            frame->set_local(arg, pop().node, this);
            break;
          case instr_load_int:
            push(FLong::create(arg_sign_extend(arg), this));
            break;
          case instr_load_lit:
            push(frame->get_method()->get_literals()->fastref(arg).node);
            break;
          case instr_load_atom:
            push(frame->get_method()->symbol(arg));
            break;
          case instr_local_frame:
			push(frame->get_self());
            // no break, fall through
		  case instr_frame: {
            op_frame(frame->get_method()->symbol(arg));
            break;
			}
          case instr_load_field: {
            Object_ptr obj = pop().object->type_check(this);
			if (!obj) break;
            FClass_ptr fclass = obj->get_fclass();
 			Method_ptr method = frame->get_method();
            FVal selector = method->symbol(arg);
			long var_index;
            //  now look up the slot:
            if (!fclass->get_symboltable()->lookup(selector, &method, 
												   &var_index) ||
				method) {
                undefined_field();
                break;
            }
            push(obj->instvar(var_index).node);
            break;
          }
          case instr_store_field: {
            Object_ptr obj = pop().object->type_check(this);
			if (!obj) break;
            FClass_ptr fclass = obj->get_fclass();
 			Method_ptr method = frame->get_method();
            FVal selector = method->symbol(arg);
			long index;
            //  now look up the slot:
            if (!fclass->get_symboltable()->
				    lookup(selector, &method, &index) || method) {
                undefined_field();
                break;
            }
            obj->set_instvar(index, pop().node, this);
            break;
          }
          default:
            illegal_instruction();
            break;
        }
    }
}


FUnion Machine::node_alloc(Tag tag, size_t size)
{
	FUnion p;
	p.node = (Node_ptr) malloc(size);
    p.node->node_initialize(tag, Node::slots_from_size(size), this);
	p.node->clear();
	return p;
}


FUnion Machine::node_alloc_color(Tag tag, size_t size, Color color)
{
	FUnion p = node_alloc(tag, size);
	p.node->set_color(color);
    return p;
}


size_t Machine::bigger(size_t size)
{
	long big = size + (size >> 2); // try for 25% bigger, round up to mult of 8
	big = round_up_size(big);
	return big;
}


void Machine::initialize()
{
	exit_flag = false;
	exception_nesting = 0;
	for (int i = SMALL_INT_MIN; i < SMALL_INT_MAX; i++) {
		small_ints[i - SMALL_INT_MIN] = FLong::must_create(i, this);
	}
	for (i = 0; i < SHORT_STRINGS_LEN; i++) {
		char ss[2];
		ss[0] = i; 
		ss[1] = 0;
		// note that strlen(ss) is zero for i=0, otherwise 1
		short_strings[i] = FString::create_len(strlen(ss), this);
		short_strings[i]->set_string(ss);
	}
	s_false = NULL;
	s_true = Symbol::create("t", this);
	s_true->set_value(s_true, this);	// initialize value to itself
	s_symbol = Symbol::create("symbol", this);
	s_long = Symbol::create("long", this);
	s_double = Symbol::create("double", this);
	s_string = Symbol::create("string", this);
	s_array = Symbol::create("array", this);
	s_dict = Symbol::create("dict", this);
	s_object = Symbol::create("object", this);
	s_class = Symbol::create("class", this);
    s_file = Symbol::create("file", this);
	for (i = 0; i < TAG_COUNT; i++) tag_to_type[i] = NULL;
	tag_to_type[TAG_SYMBOL] = s_symbol;
	tag_to_type[TAG_LONG] = s_long;
	tag_to_type[TAG_DOUBLE] = s_double;
	tag_to_type[TAG_STRING] = s_string;
	tag_to_type[TAG_ARRAY] = s_array;
	tag_to_type[TAG_DICT] = s_dict;
	tag_to_type[TAG_OBJECT] = s_object;
	tag_to_type[TAG_CLASS] = s_class;
	tag_to_type[TAG_FILE] = s_file;

	s_init = Symbol::create("init", this);
	s_global = Symbol::create("global", this);
/*
    s_readline = Symbol::create("readline", this);
    s_close = Symbol::create("close", this);
    s_token = Symbol::create("token", this);
    s_write = Symbol::create("write", this);
    s_readvalue = Symbol::create("readvalue", this);
    s_read = Symbol::create("read", this);
    s_keys = Symbol::create("keys", this);
    s_values = Symbol::create("values", this);
    s_values = Symbol::create("clear", this);
    s_values = Symbol::create("remove", this);
    s_copy = Symbol::create("copy", this);
    s_has_key = Symbol::create("has_key", this);
    s_get = Symbol::create("get", this);
    s_get2 = Symbol::create("get2", this);
    s_chr = Symbol::create("chr", this);
    s_ord = Symbol::create("ord", this);
    s_intern = Symbol::create("intern", this);
    s_hex = Symbol::create("hex", this);
    s_oct = Symbol::create("oct", this);
    s_id = Symbol::create("id", this);
    s_inst = Symbol::create("inst", this);
    s_subcl = Symbol::create("subcl", this);
*/
	s_runtime_exception = Symbol::create("runtime_exception", this);

    builtinfn_init(); // install builtin functions

	// create an initial frame and associated method so calls will work:
    frame = Frame::create(FRAME_LOCAL_BASE + EXPR_STACK_MAX, this);
	Method_ptr method = Method::create(s_global, this);
	s_global->set_function(method, this); // save for others to use
	push(method); // protect from GC
	frame->init_method(method, this);
	pop();
	method->set_stack_offset(0);
	method->set_frame_slots(EXPR_STACK_MAX);
	// create callin_method to aid in debugging
	callin_method = Method::create(Symbol::create("<callin_method>", this), this);
	callin_method->set_stack_offset(0);
	callin_method->set_frame_slots(EXPR_STACK_MAX);
}



void Machine::mark_white()
{
	for (int i = SMALL_INT_MIN; i < SMALL_INT_MAX; i++) {
		small_ints[i - SMALL_INT_MIN]->set_color(GC_WHITE);
	}
}


/* 1 */

FClass_ptr Machine::class_lookup(Symbol_ptr s)
{
	FClass_ptr fclass = s->get_value().fclass;
	if (fclass && fclass->has_tag(TAG_CLASS)) 
		return fclass;
	return NULL;
}


void Machine::class_add(Symbol_ptr s, FClass_ptr fclass)
{
	s->set_value(fclass, this);
}


void Machine::file_load(char *filename)
{
    Compiler compiler;
    compiler.init(this);
    compiler.compile(filename);
}

