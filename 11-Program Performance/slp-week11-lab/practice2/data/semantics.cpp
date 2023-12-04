/* semantics -- */

#include "flincl.h"
#include "shortstack.h"
#include "semantics.h"
#include "stdio.h" // for compiler.h
#include "compiler.h"
#include "assert.h"

#define must(emit) if (!(emit)) return false;

typedef struct {
	char *name;
	short params;
	short op;
} Builtin_desc;

// built_ins are "global functions" that cannot be
// overridden or overloaded
Builtin_desc built_ins[] = {
	{ "array", 1, OP_NEWARRAY },
    { "dict", 1, OP_NEWDICT },
    { "flatten", 1, OP_FLATTEN },
    { "repr", 1, OP_REPR },
    { "str", 1, OP_STR },
    { "type", 1, OP_TYPE },
    { "len", 1, OP_LENGTH },
    { "cos", 1, OP_COS },
    { "sin", 1, OP_SIN },
    { "tan", 1, OP_TAN },
    { "exp", 1, OP_EXP },
    { "sqrt", 1, OP_SQRT },
    { "rem", 1, OP_REM },
    { "min", 2, OP_MIN },
    { "max", 2, OP_MAX },
    { "random", 0, OP_RANDOM },
    { "abs", 1, OP_ABS },
    { "int", 1, OP_TRUNCATE },
    { "round", 1, OP_ROUND },
    { "float", 1, OP_FLOAT },
    { "chr", 1, OP_CHR },
    { "ord", 1, OP_ORD },
    { "hex", 1, OP_HEX },
    { "id", 1, OP_ID },
    { "intern", 1, OP_INTERN },
    { "isinstance", 2, OP_INST },
    { "issubclass", 2, OP_SUBCL },
    { "oct", 1, OP_OCT },
    { "strcat", 2, OP_STRCAT },
    { "subseq", 3, OP_SUBSEQ },
    { "isupper", 1, OP_ISUPPER },
    { "islower", 1, OP_ISLOWER },
    { "isalpha", 1, OP_ISALPHA },
    { "isdigit", 1, OP_ISDIGIT },
    { "isalnum", 1, OP_ISALNUM },
    { "toupper", 1, OP_TOUPPER },
    { "tolower", 1, OP_TOLOWER },
    { "find", 4, OP_FIND },
	{ "exit", 0, OP_EXIT },
	{ "frame_previous", 1, OP_FRMPREV },
	{ "frame_variables", 1, OP_FRMVARS },
	{ "frame_pc", 1, OP_FRMPC },
	{ "frame_method", 1, OP_FRMMETH },
	{ "frame_class", 1, OP_FRMCLASS },
	{ "runtime_exception_nesting", 0, OP_NEST },
	{ "frame_get", 0, OP_FRMGET },
	{ NULL, 0, 0 }
};

// builtin_methods are implemented by op_codes,
// but if the first operand's type is object,
// a method lookup is performed. Builtin_methods
// allow us to implement builtin methods on builtin
// types, and run more efficiently than a full
// method call which allocates a frame.
Builtin_desc builtin_methods[] = {
    { "readline", 0, OP_READLINE },
    { "write", 1, OP_WRITE },
    { "read", 1, OP_READ },
	{ "unread", 1, OP_UNREAD },
    { "readvalue", 0, OP_READVAL },
    { "close", 0, OP_CLOSE },
    { "token", 0, OP_TOKEN },
	{ "append", 1, OP_APPEND },
	{ "unappend", 0, OP_UNAPPEND },
    { "last", 0, OP_LAST },
    { "keys", 0, OP_KEYS },
    { "values", 0, OP_VALUES },
    { "clear", 0, OP_CLEAR },
    { "index", 1, OP_INDEX },
    { "reverse", 0, OP_REVERSE },
    { "sort", 0, OP_SORT },
    { "remove", 1, OP_REMOVE },
    { "copy", 0, OP_COPY },
    { "has_key", 1, OP_HASKEY },
    { "get", 2, OP_GET },
	{ "value", 0, OP_VALUE },
    // { "get2", 2, OP_GET2 },
    { NULL, 0, 0 }
};



// builtin -- searches for id in built-in list
//
// if found, return op-code required to execute the built-in
// and set actuals to the number of parameters required by
// the built-in function
// otherwise, return 0 and set actuals to -1
//
short Semantics::builtin(char *id, short *actuals)
{
    int i = 0;
	while (built_ins[i].name) {
		if (streql(built_ins[i].name, id)) {
			*actuals = built_ins[i].params;
			return instr_compose(instr_op, built_ins[i].op);
		}
		i++;
	}
	*actuals = -1;
	return 0;
}


short Semantics::builtin_method(char *id, short *actuals)
{
    int i = 0;
    while (builtin_methods[i].name) {
        if (streql(builtin_methods[i].name, id)) {
            *actuals = builtin_methods[i].params;
            return instr_compose(instr_op, builtin_methods[i].op);
        }
        i++;
    }
    *actuals = -1;
    return 0;
}


bool Semantics::init(Machine_ptr m, Compiler_ptr c)
{
	mach = m;
	comp = c;
	instructions = NULL;
	if_stack = Short_stack::create(3, mach);
	loop_stack = Short_stack::create(2, mach);
	assign_stack = Short_stack::create(2, mach);
	indent_stack = Short_stack::create(8, mach);
	array_stack = Short_stack::create(2, mach);
	call_stack = Short_stack::create(2, mach);
    return if_stack && loop_stack && assign_stack && 
        indent_stack && array_stack && call_stack;
    // BUG: need error report here if returning false
}


Semantics::Semantics()
{
}


bool Semantics::reset()
{
    if (instructions) {
        //FLSys::free(instructions, max_instr);
    }
	class_started = false;
	method_started = false;
	local_offset = 0;
	member_offset = 0;
    next_instr = 0;
    max_instr = 0;
	method = NULL;
	fclass = NULL;
	method_name = NULL;
    class_name = NULL;
    var_name = NULL;
	array_index = 0;
	opcode = 0;
    actuals = 0;
	// initially has room for 16 symbols
	symbols = Array::create(0, 16, mach);
    return symbols != NULL;
}


bool Semantics::emit(short instruction)
{
    if (next_instr >= max_instr) {
        if (!grow()) return false;
    }
    if (next_instr < max_instr) {
        instructions->set_instr(next_instr++, instruction, 
                                mach->trace_flag);
    }
    return true;
}


bool Semantics::emit_at(long location, long instruction)
{
    assert(location < next_instr);
	if (instruction > 0x7FFF || instruction < -0x10000) {
		comp->report_error("virtual machine limitation: branch out of bounds");
		return false;
	}
    instructions->set_instr(location, (short) instruction, 
                            mach->trace_flag);
	return true;
}


bool Semantics::for_var(char *id)
{
	long index;  // variable index in frame
	// is this a local? look in method symbol table
	Symbol_ptr s = Symbol::create(id, mach);
    if (!method->get_localtable()->lookup(s, &index)) {
		comp->report_error("undeclared loop variable");
		return false;
	}
	// now index is the loop variable offset
	loop_stack->push((short) index, mach);
	return true;
}


bool Semantics::for_init()
{
	short index = loop_stack->pop(mach);
	// store the initial value
	must(emit(instr_compose(instr_store_local, (short) index)));
	loop_stack->push(index, mach);  // save for later
	return true;
}


bool Semantics::for_to()
{
	return true;
}


bool Semantics::for_by(bool by_flag)
{
	if (!by_flag) { // no BY expression
		emit(instr_compose(instr_load_int, 1));
	}
	// generate instruction to push the loopcounter
	short index = loop_stack->pop(mach);
	loop_stack->push(index, mach);
	emit(instr_compose(instr_load_local, (short) index));
	emit(OP_JUMP);
	loop_stack->push(next_instr, mach);
	emit(0);
	// machine stack is to_value, by_value, loopcount (top)
	return true;
}


// for_end_range -- called after for-=-to-by-:
//
bool Semantics::for_end_range()
{
	long looptop = loop_stack->pop(mach);
	long loopvar = loop_stack->pop(mach);
	return emit(OP_LOOPINC) &&
	       emit((short) loopvar) &&
		   // jump instruction is pc += *pc,
		   //    next_instr = looptop + *looptop,
		   //    *looptop = next_instr - looptop
		   emit_at(looptop, next_instr - looptop) &&
		   emit(OP_LOOPTST) &&
		   // jump instruction is pc += *pc
		   //    looptop + 1 = next_instr + *next_instr
		   //    *next_instr = looptop + 1 - next_instr
		   emit(looptop + 1 - next_instr) &&
	       // break instructions should branch to here
		   emit(OP_POP2);
}


bool Semantics::for_array()
{
	short index = loop_stack->pop(mach);
	must(emit(instr_compose(instr_load_int, 0)));
	must(emit(OP_JUMP));
	loop_stack->push(next_instr, mach);
	return emit(0) &&
		   // store the initial value
		   emit(instr_compose(instr_store_local, index));
}


bool Semantics::for_end_elements()
{
	// stack has looptop-1
	// machine stack has array index, then array
	long looptop = loop_stack->pop(mach);
	return emit_at(looptop, next_instr - looptop) &&
		   emit(OP_LOOPNXT) &&
		   emit(looptop - next_instr);
}


bool Semantics::return_null()
{
    emit(OP_RETNULL);
    return true;
}


// unemit -- undo the previous emit, and return opcode
//
short Semantics::unemit()
{
    if (next_instr <= 0) return OP_NOOP;
    next_instr--;
    return instructions->get_instr(next_instr);
}

    
bool Semantics::grow()
{
    short newlen = 128;
    if (newlen <= max_instr) newlen = max_instr * 2;
    assert(max_instr < newlen);
    Code_ptr newinstr = Code::create(newlen, mach);
    if (!newinstr) return false;
	if (next_instr > 0) {
        for (long i = 0; i < next_instr; i++) {
            newinstr->set_instr(i, instructions->get_instr(i), false);
		}
	}
    instructions = newinstr;
    max_instr = newlen;
    return true;
}


// call prepares to invoke a function.
// The current opcode,actuals pair is pushed
// The new opcode is looked up and saved until
// after parameters are ready
//
bool Semantics::call(char *id)
{
	call_stack->push(opcode, mach);
	call_stack->push(actuals, mach);
	opcode = builtin(id, &actuals);
	if (opcode == 0) {
	    long index = insert_symbol(id);
	    assert(index >= 0 && index < 2048);
	    emit(instr_compose(instr_local_frame, (short) index));
	}
	return true;
}


bool Semantics::method_call(char *id)
{
	call_stack->push(opcode, mach);
	call_stack->push(actuals, mach);
    opcode = builtin_method(id, &actuals);
    if (opcode == 0) {
	    long index = insert_symbol(id);
	    assert(index >= 0 && index < 2048);
	    opcode = instr_compose(instr_frame, (short) index);
    }
    return true;
}


bool Semantics::class_start(char *class_name, char *super_name)
{
	fclass = FClass::create(mach);
	if (!fclass) return false;
	Symbol_ptr s = Symbol::create(class_name, mach);
	mach->push(s);
	FClass_ptr probe = mach->class_lookup(s);
	if (probe) {
		comp->report_error("class is already defined");
		mach->pop();
		fclass = NULL;
		return false;
	}
	mach->class_add(s, fclass);
	mach->pop();
	if (super_name[0]) {
		s = Symbol::create(super_name, mach);
		FClass_ptr super = mach->class_lookup(s);
		if (!super) {
			comp->report_error("super class is undefined");
			fclass = NULL;
			return false;
		}
		fclass->init_super(super);
		member_offset = super->get_inst_slots();
	} else {
		member_offset = 1; // all objects have class pointer
	}
	class_started = true;
	return true;
}


bool Semantics::method_start()
{
    method_started = true;
    local_offset = 0;
    return true;
}


bool Semantics::method_end()
{
	emit(instr_compose(instr_op, OP_RETNULL));
	// copy the code vector to the method
    Code_ptr newinstr = Code::create(next_instr, mach);
    if (!newinstr) return false;
    for (long i = 0; i < next_instr; i++) {
        newinstr->set_instr(i, instructions->get_instr(i), false);
	}
    int len = 1;
    if (mach->trace_flag) {
        for (i = 0; i < next_instr; i += len) {
            char text[64];
            trace(newinstr->disassemble(i, newinstr->get_instr(i), 
                                        newinstr->get_instr(i + 1), text, 
                                        &len, method));
        }
    }
	method->set_code(newinstr, mach);
	method = NULL;
	method_started = false;
	next_instr = 0;
	return true;
}


bool Semantics::method_name_is(char *name)
{
	method_name = Symbol::create(name, mach);
    method = Method::create(method_name, mach);
    if (class_started) { // insert method in the class
		fclass->enter_method(method_name, method, mach);
		method->set_fclass(fclass, mach);
	} else { // attach method to symbol
        method_name->set_function(method, mach);
        method->set_fclass(NULL, mach);
    }
    // add symbols to method:
   	method->init_symbols(symbols, mach);
	if (mach->trace_flag)
        trace("method_name_is: symbol name is %s\n", 
		      method_name->get_name()->get_string());
    return true;
}


bool Semantics::begin_formals()
{
    formal_type = FORMAL_REQUIRED;
    return true;
}


bool Semantics::set_formal_type(short ft)
{
    if (ft < formal_type) {
    	comp->report_error("formal type out of order");
        return false;
    }
    formal_type = ft;
    return true;
}


bool Semantics::formal_is(char *name)
{
	// convert the name to a symbol in this class
	var_name = Symbol::create(name, mach);
    // position the stack above the parameter:
	method->set_stack_offset(1 + method->get_stack_offset());
    switch (formal_type) {
      case FORMAL_OPTIONAL:
        method->set_optionals(1 + method->get_optionals());
        // no break, fall through to increment parameters too!
      case FORMAL_REQUIRED:
        method->set_parameters(1 + method->get_parameters());
        break;
      case FORMAL_KEYWORD:
        method->set_keywords(1 + method->get_keywords());
        break;
      case FORMAL_REST:
        method->set_rest_flag(true);
        break;
      case FORMAL_DICTIONARY:
        method->set_dict_flag(true);
        break;
    }
        
    return method->get_localtable()->insert_var(var_name, local_offset++, mach);
}


bool Semantics::default_value(FVal v)
{
    // first see if there is an optionals array:
    Array_ptr defaults = method->get_default();
    if (!defaults) {
        defaults = Array::create(1, 1, mach);
        method->set_default(defaults);
    }
    // this new value is for local_offset-1, but the defaults start
    // at parameters - optionals
    long index = local_offset - 1 - 
        (method->get_parameters() - method->get_optionals());
    // make sure array is long enough:
    while (defaults->get_array_len() <= index) {
        defaults->append(NULL, mach);
    }
    defaults->set_fastref(index, v, mach);
    return true;
}


bool Semantics::end_of_formals()
{
    // signature is number of formals
    // stack_offset is number of formals + locals
	// frame_slots is stack_offset + maximum stack size
	method->set_frame_slots(method->get_stack_offset() + 
							EXPR_STACK_MAX);
    return true;
}


bool Semantics::id(char *id)
{
	Symbol_ptr s = Symbol::create(id, mach);
	long index; // variable index in frame
	Method_ptr mp;
	mach->push(s);
	// is this a local? look in method symbol table
    if (method->get_localtable()->lookup(s, &index)) {
		must(emit(instr_compose(instr_load_local, (short) index)));
	} else if (fclass &&
               fclass->get_symboltable()->lookup(s, &mp, &index) &&
			   mp == NULL) {
		must(emit(instr_compose(instr_load_instvar, (short) index)));
	} else { // load value from a global
		index = insert_symbol(id);
		must(emit(instr_compose(instr_load_global, (short) index)));
	}
	mach->pop();
	return true;
}


bool Semantics::assign_id(char *id)
{
	Symbol_ptr s = Symbol::create(id, mach);
	long index; // variable index in frame
	Method_ptr mp;
	mach->push(s);
	// is this a local? look in method symbol table
    if (method->get_localtable()->lookup(s, &index)) {
		must(emit(instr_compose(instr_store_local, (short) index)));
	} else if (fclass && 
               fclass->get_symboltable()->lookup(s, &mp, &index) &&
			   mp == NULL) {
		trace("assign_id got instance index of %d\n", index);
		must(emit(instr_compose(instr_store_instvar, (short) index)));
	} else { // load value from a global
		index = insert_symbol(id);
		must(emit(instr_compose(instr_store_global, (short) index)));
	}
	mach->pop();
	return true;
}


bool Semantics::field(char *id)
{
	trace("field not implemented\n");	// not implemented
	return true;
}


bool Semantics::aref()
{
	return emit(OP_AREF);
}

bool Semantics::if_true()
{
    // emit a test to branch over true part
    must(emit(OP_JUMPIFNOT));
    // save PC for the branch location
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Semantics::if_else()
{
    // this is the target for the if:
	long target = if_stack->pop(mach);
    // emit a jump around else part
    must(emit(OP_JUMP));
    // save PC for the branch location
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    emit(0);
    return emit_at(target, next_instr - target);
}


bool Semantics::if_end()
{
    // this is the target for whatever is on the stack
	long target = if_stack->pop(mach);
    return emit_at(target, next_instr - target);
}


bool Semantics::while_begin()
{
    // remember where to branch back to
    loop_stack->push(next_instr, mach);
    return true;
}

bool Semantics::while_true()
{
    // emit a test to branch over true part
    must(emit(OP_JUMPIFNOT));
    // save PC for the branch location
    loop_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Semantics::while_end()
{
    // top of stack is branch out of loop
    long end_of_loop_branch = loop_stack->pop(mach);
    // now emit loop branch
    must(emit(OP_JUMP));
    must(emit(loop_stack->pop(mach) - next_instr));
    // fix up previous branch to next instr (after loop)
    return emit_at(end_of_loop_branch, next_instr - end_of_loop_branch);
}


bool Semantics::return_end()
{
    return emit(OP_RETURN);
}


bool Semantics::load_end()
{
    return emit(OP_LOAD);
}


bool Semantics::print_expr()
{
	return emit(OP_PRINT);
}


bool Semantics::print_comma()
{
    return emit(OP_SPACE);
}


bool Semantics::print_end()
{
    return emit(OP_ENDL);
}


// assign_end -- called after rval is on stack
// 
// emit pushed instruction to finish the job:
// possibilities are:
//   assign to array: stack has array, index, value
//   assign to head: stack has node, value
//   assign to tail: stack has node, value
//   assign to variable: stack has value
//   assign to field of object: stack has object, index, value
//
bool Semantics::assign_end()
{
    short op = assign_stack->pop(mach);
    return emit(op);
}


// rval2lval -- prepare for an assignment statement
//
// when you encounter an '=', the expression on the left has
// just emitted an instruction to load the value to the stack
// The method "undoes" the emit, and converts the instruction
// from one that loads a value to one that stores a value.
// Error is returned if the instruction cannot be converted.
// E.g. if the instruction is an array reference, convert to 
// a store.  If the instruction is addition, return false.
//
bool Semantics::rval2lval()
{
    // first get previous instruction
    short op = unemit();
    short newop = OP_NOOP;
    
    // now compute the assignment version of op
    short typ = instr_type(op);
    short arg = instr_arg(op);
    if (typ == instr_op) {
        if (op == OP_AREF) newop = OP_SETAREF;
        else if (op == OP_FIELD) newop = OP_SETFIELD;
    } else if (typ == instr_load_global) {
        newop = instr_compose(instr_store_global, arg);
    } else if (typ == instr_load_instvar) {
        newop = instr_compose(instr_store_instvar, arg);
    } else if (typ == instr_load_local) {
        newop = instr_compose(instr_store_local, arg);
    }
    if (newop != OP_NOOP) {
        assign_stack->push(newop, mach);
        return true;
    }
    return false;
}
        

bool Semantics::unary_minus()
{
    return emit(OP_MINUS);
}


bool Semantics::not()
{
    return emit(OP_NULL);
}


bool Semantics::lognot()
{
    return emit(OP_LOGNOT);
}


bool Semantics::push_long(int64 value)
{
	if (mach->arg_sign_extend(instr_arg((long) value)) == value) {
		return emit(instr_compose(instr_load_int, instr_arg((short) value)));
	} else {
		FVal lit = FLong::create(value, mach);
		mach->push(lit);
		assert(method);
		short arg = method->literal_add(lit, mach);
		mach->pop();
		return emit(instr_compose(instr_load_lit, arg));
	}
}

bool Semantics::push_double(double value)
{
	FVal lit = FDouble::create(value, mach);
	return push_fval(lit);
}


bool Semantics::push_string(char *s)
{
    FVal lit = FString::create(s, mach);
	return push_fval(lit);
}


bool Semantics::push_symbol(char *s)
{
    FVal lit = Symbol::create(s, mach);
    return push_fval(lit);
}


bool Semantics::push_fval(FVal v)
{
	mach->push(v);
	assert(method);
	short arg = method->literal_add(v, mach);
	mach->pop();
	return emit(instr_compose(instr_load_lit, arg));
}


bool Semantics::var_decl(char * id)
{
	if (method_started) { // local variable in method
		assert(method);
		Symbol_ptr s = Symbol::create(id, mach);
		assert(s);
		mach->push(s);
		trace("var_decl: before new method variable, locals are:\n");
		method->get_localtable()->trace_locals();
		if (!method->get_localtable()->
			insert_var(s, local_offset++, mach)) {
			comp->report_error("local variable already declared");
			mach->pop();
			return false;
		}
		method->set_stack_offset(1 + method->get_stack_offset());
		mach->pop();
	} else if (class_started) {
		assert(fclass);
		Symbol_ptr s = Symbol::create(id, mach);
		mach->push(s);
		trace("instance var at index %d\n", member_offset);
		if (!fclass->get_symboltable()->
			insert_var(s, member_offset++, mach)) {
			comp->report_error("instance variable already declared");
			mach->pop();
			return false;
		}
		mach->pop();
		fclass->set_inst_slots(member_offset);
	}
	// undeclared variables are globals, the value is stored on the
	// symbol itself, so we do not need to enter it into a table

	// show current state of things
	if (method_started) {
		trace("current locals are:\n");
		method->get_localtable()->trace_locals();
	} else if (class_started) {
		trace("current members are:\n");
		fclass->get_symboltable()->trace_members();
	} else trace("global variable declared\n");
	return true;		
}


// Here's how array code generation works:
//    the current array_index is pushed on array_stack
//     and array_index is set to 0
//     (in case we are dealing with nested arrays)
//    emit placeholder for the size of the array
//     (so there is an upper bound on the size of a 
//     literal array: the maximum literal integer,
//     which is 2047)
//    the location of the literal is pushed on array_stack
//    the create array instruction is emitted
//    for each expression, we duplicate the top of stack,
//     emit a literal index, and emit a store into array op
//    at array_end, we know the size of the array, so we
//     pop the stack and emit the size at that location
//    finally, pop the previous array_index value
bool Semantics::array_start()
{
	array_stack->push(array_index, mach);
	array_index = 0;
	array_stack->push(next_instr, mach);
	return emit(0) && emit(OP_NEWARRAY);
}

bool Semantics::dict_start()
{
	array_stack->push(array_index, mach);
	array_index = 0;
	array_stack->push(next_instr, mach);
	return emit(0) && emit(OP_NEWDICT);
}



bool Semantics::before_array_expr()
{
	short instr = instr_compose(instr_load_int, array_index);
	if (instr_arg(instr) != array_index) {
		comp->report_error("array expression too long");
		return false;
	}
	array_index++;
    return emit(OP_DUP) && emit(instr);
}


bool Semantics::before_dict_key()
{
	array_index++;
    return emit(OP_DUP);
}


bool Semantics::before_dict_val()
{
    return true;
}



bool Semantics::array_expr()
{
    return emit(OP_SETAREF);
}

bool Semantics::dict_key()
{
    return true;
}


bool Semantics::dict_val()
{
    return emit(OP_SETAREF);
}



bool Semantics::array_end()
{
	must(emit_at(array_stack->pop(mach), 
			     instr_compose(instr_load_int, array_index)));
	array_index = array_stack->pop(mach);
	return true;
}


bool Semantics::dict_end()
{
	must(emit_at(array_stack->pop(mach), 
			     instr_compose(instr_load_int, array_index)));
	array_index = array_stack->pop(mach);
	return true;
}


// here's how calls work: 
// there are many built-in functions that compile to instructions
// which must be handled here rather than at run-time. When call()
// or method_call() is called, we look up the function to see if it
// is built-in. If so, we push the corresponding opcode and then
// the required number of parameters onto the call_stack (a 
// compile-time structure). If the function is not built-in, we
// push zero and -1. Whenever we get an actual, we look at the top
// of stack. If opcode = 0 (not built-in), we generate a param opcode, 
// otherwise we decrement
// the top of stack. (At runtime the parameter will be left on the
// expression stack, not copied to a frame.) If the top of stack 
// would go to -1, we return an error (too many parameters).
// When we get an actual_end() we look at the opcode:
// If non-zero, emit it. If zero, emit the
// call opcode. If actuals is greater than zero, return an error
// (too few actuals).
// FOR CONVENIENCE, we will keep the top of the stack in member vars,
// opcode and actuals.

bool Semantics::actual()
{
    if (opcode == 0) return emit(OP_PARAM);
	if (actuals == 0) {
		comp->report_error("too many parameters");
		return false;
	}
	actuals--;
	return true;
}


bool Semantics::keyword(long index)
{
    assert(index < 0x10000);
    if (opcode == 0) {
        return emit(OP_KEYPARAM) && emit((short) index);
    } else {
        comp->report_error("unexpected keyword parameter");
        return false;
    }
}


bool Semantics::actual_end()
{
    // if optional start, stop parameters are missing, fill
    // them in now
    if (opcode == instr_compose(instr_op, OP_FIND)) {
        if (actuals == 2) {
            push_long(0);
            actuals--;
        }
        if (actuals == 1) {
            push_long(-1);
            actuals--;
        }
    // this is another style for variable argument lists
    // if get(key) use OP_GET, if get(key, default) use OP_GET2:
    } else if (opcode == instr_compose(instr_op, OP_GET)) {
        if (actuals == 1) {
            actuals--; // OP_GET really takes one parameter
        } else if (actuals == 0) {
            // 2 parameters, so use OP_GET2:
            opcode = instr_compose(instr_op, OP_GET2);
        }
    }
	if (actuals > 0) {
		comp->report_error("too few parameters");
		return false;
	}
	if (opcode != 0) emit(opcode);
	else emit(instr_compose(instr_op, OP_CALL));
    actuals = call_stack->pop(mach);  // restore actuals
	opcode = call_stack->pop(mach);   // restore opcode
	return true;
}


bool Semantics::end_id_expr()
{
	emit(instr_compose(instr_op, OP_POP));
	return true;
}

int Semantics::insert_symbol(char *str)
{
	Symbol_ptr s = Symbol::create(str, mach);
	mach->push(s);
	long len = symbols->get_array_len();
	for (int i = 0; i < len; i++) {
		if (symbols->fastref(i).symbol == s) {
			mach->pop();
			return i;
		}
	}
	symbols->append(s, mach);
	mach->pop();
	return i;
}


bool Semantics::relop(char *op)
{
    if (streql(op, "<")) return emit(OP_LESS);
    if (streql(op, ">")) return emit(OP_GTR);
    if (streql(op, "<=")) return emit(OP_LESSEQL);
    if (streql(op, ">=")) return emit(OP_GTREQL);
    if (streql(op, "==")) return emit(OP_ISEQUAL);
    if (streql(op, "!=")) return emit(OP_NOTEQUAL);
    if (streql(op, "is")) return emit(OP_IS);
    if (streql(op, "isnot")) return emit(OP_ISNOT);
    if (streql(op, "in")) return emit(OP_MEMBER);
    if (streql(op, "notin")) return emit(OP_NOTMEMBER);
    return false;
}


bool Semantics::addop(char *op)
{
    if (streql(op, "+")) return emit(OP_ADD);
    if (streql(op, "-")) return emit(OP_SUB);
    return false;
}


bool Semantics::mulop(char *op)
{
    if (streql(op, "*")) return emit(OP_MUL);
    if (streql(op, "/")) return emit(OP_DIV);
    if (streql(op, "%")) return emit(OP_REM);
    return false;
}


bool Semantics::power()
{
    return emit(OP_POWER);
}


bool Semantics::begin_and(char *op)
{
    if (op[0] == '&') return true; // logical and
    must(emit(OP_CNDAND));
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Semantics::end_and(char *op)
{
    if (op[0] == '&') return emit(OP_LOGAND);
	long target = if_stack->pop(mach);
    return emit_at(target, next_instr - target);
    // emit a jump around 2nd operand to here
}

bool Semantics::begin_or(char *op)
{
    if (op[0] == '|' ||
        op[0] == '^') return true; // logical or
    must(emit(OP_CNDOR));
    if_stack->push(next_instr, mach);
    // emit zero as a placeholder for branch location
    return emit(0);
}


bool Semantics::end_or(char *op)
{
    if (op[0] == '|') return emit(OP_LOGIOR);
    if (op[0] == '^') return emit(OP_LOGXOR);
    if (op[0] == '<') return emit(OP_LSHIFT);
    if (op[0] == '>') return emit(OP_RSHIFT);
	long target = if_stack->pop(mach);
    return emit_at(target, next_instr - target);
    // emit a jump around 2nd operand to here
}



