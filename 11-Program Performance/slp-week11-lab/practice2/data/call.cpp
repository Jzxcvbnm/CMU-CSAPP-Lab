cccd
aaaab
/* call.cpp -- implements method calls and returns, jumps, etc. */


#include "memory.h"
#include "flincl.h"
#include "assert.h"


void Machine::op_frame(Symbol_ptr sym)
{
	if (trace_flag) {
        trace("instr_frame:\n");
	    frame->trace();
    }
	Method_ptr method = frame->get_method();
    FClass_ptr fclass = method->get_fclass();
    Symbol_ptr selector = sym;
	long var_index;
    //  now look up the method:
    Object_ptr obj = read_tos().object;
    if (!obj) { // self == NULL, making a call from a global function
        method = selector->get_function();
    } else {
        obj->type_check(this);
        if (error_flag) return;
        if (!obj->get_fclass()->get_symboltable()->lookup(
				        selector, &method, &var_index) ||
			!method) {
			method = selector->get_function();
            if (method) obj = NULL;
        }
    }
	if (!method) {
		undefined_method(selector);
		return;
	}
    do_frame(obj, method);
}


// do_frame -- creates a new frame
//
// On entry:
//           top of stack is to be replaced by frame
//           zero (arg index) is pushed
void Machine::do_frame(Object_ptr obj, Method_ptr method)
{
    long frame_slots;
    long n_parms;
    // method may be builtin or ordinary
    if (method->has_tag(TAG_METHOD)) {
        frame_slots = method->get_frame_slots();
	    n_parms = method->get_parameters() +
                  method->get_keywords() + method->get_rest_flag() +
                  method->get_dict_flag();
    } else {
        Builtin_ptr builtin = (Builtin_ptr) method;
        assert(method->has_tag(TAG_BUILTIN));
        n_parms = builtin->get_parameters();
        frame_slots = (n_parms != 0xFFFF ? n_parms : 1);
    }
    Frame_ptr frame = Frame::create(FRAME_LOCAL_BASE + frame_slots, this);

    // store object, method, and current frame in new frame:
    frame->init_self(obj, this); 
    frame->init_method(method, this); 
    frame->init_previous(frame, this);
    // replace tos with frame, overwriting object:
    (*(tos - 1)).frame = frame;
	if (n_parms == 0xFFFF) { // this mechanism is for builtins only
		frame->set_local(0, Array::create(0, 3, this), this);
	}
	// this is the argument index:
    push(FLong::create(0, this));
    if (method->has_tag(TAG_METHOD)) {
        Array_ptr defaults = method->get_default();
        long parameters = method->get_parameters();
        if (defaults) {
            // copy default values into frame:
            long base = parameters - method->get_optionals();
            push(defaults);
            op_print();
            for (int i = 0; i < defaults->get_array_len(); i++) {
                frame->set_local(base + i, defaults->fastref(i).node, this);
            }
        }
        parameters += method->get_keywords();
        if (method->get_rest_flag()) {
            // create rest array?
		    frame->set_local(parameters, Array::create(0, 3, this), this);
        }
        if (method->get_dict_flag()) {
            // create the dictionary
            frame->set_local(parameters + method->get_rest_flag(),
                             Dict::create(this), this);
        }
    }
}


/*
void Machine::op_frame()
{
    int index = *pc++;  // fetch the method index
    Object_ptr obj = read_tos().object->type_check(this);
    Method_ptr method = obj->get_fclass()->get_methods()->method(index);
    do_frame(obj, method);
}
*/

/*
void Machine::op_gframe()
{
    push(globals);
    op_frame();
}
*/

void Machine::op_param()
{
    /* on entry, stack (top down) contains:
     *  parameter, index, frame
     */
    FVal param = read_tos().node;
    int index = (int) (read_next_tos().flong->int64_data);
    Frame_ptr frame = read_next_next_tos().frame;
    Method_ptr method = frame->get_method();
	long parameters = method->get_parameters();
    if (index < parameters) {
        /* the first local is the first arg, etc. */
        frame->set_local(index, param, this);
		pop(); // pop the parameter
        replace(FLong::create(index + 1, this));
    } else if (parameters == 0xFFFF) { // special case for built-in
		// append param to array in frame local 0
		frame->local(0).array->append(param, this);
	    pop(); // pop the parameter
        replace(FLong::create(index + 1, this));
	} else if (method->has_tag(TAG_BUILTIN)) {
        too_many_parameters();
    } else if (method->get_rest_flag()) {
        frame->local(parameters + method->get_keywords()).array->
            append(param, this);
        pop(); // pop the parameter
	} else too_many_parameters();
}


void Machine::op_keyparam()
{
    /* on entry, stack (top down) contains:
     * parameter, index, frame; keyword is next opcode
     */
    FVal param = read_tos().node;
    Frame_ptr frame = read_next_next_tos().frame;
    Method_ptr method = frame->get_method();
    Symbol_ptr key = method->symbol(*pc++);
    if (trace_flag) trace("keyparam is %s\n", key->get_name()->get_string());
    if (method->has_tag(TAG_BUILTIN)) {
        invalid_keyword_parameter();
        return;
    }   
    // keyword should be in locals dictionary and should be between
    // parameters and parameters + keywords
	long parameters = method->get_parameters();
    long keywords = method->get_keywords();
    long index;
    if (!method->get_localtable()->lookup(key, &index)) {
        if (method->get_dict_flag()) {
            Dict_ptr dict = frame->local(parameters + keywords +
                                         method->get_rest_flag()).dict;
            dict->insert(key, param, this);
            pop();
            replace(FLong::create(index + 1, this));
            return;
        }
    } else if (index >= parameters && index < parameters + keywords) {
        frame->set_local(index, param, this);
        pop();
        return;
    }
    invalid_keyword_parameter();
}


void Machine::op_call()
{
    Frame_ptr nextframe = read_next_tos().frame;
	// check parameter count
	Method_ptr method = nextframe->get_method();
    // get_signature works for both methods and builtins
	long parameters = method->get_parameters();
    long optionals = method->get_optionals();
	if ((parameters != 0xFFFF) && 
        (parameters - optionals > (int) (read_tos().flong->int64_data))) {
		parameter_count_mismatch();
		return;
	}
    pop_n(2); // get rid of the arg counter and nextframe
	// now copy the stack into the frame
	// the destination is stored in the method:
	long offset = FRAME_LOCAL_BASE + method->get_stack_offset();
	// the size is simply tos - expr_stack
	// trace("call frame->set_tos(%d)\n", tos - expr_stack);
	frame->set_tos(tos - expr_stack);
	FUnion *p = expr_stack;
	while (p < tos) {
		if (trace_flag) trace("call saving %x at %d\n", *p, offset);
		set_ptr_in(frame, offset++, (*p).node, this);
		p++;
	}
	tos = expr_stack;   // empty the expression stack
	frame->set_pc(pc);	// return address
    nextframe->init_previous(frame, this);
    frame = nextframe;
	::trace("in op_call\n");
	frame->trace_stack();
    if (method->has_tag(TAG_METHOD)) {
	    if (trace_flag) {
            trace("op_call method:\n");
	        method->trace();
        }
        pc = method->get_code()->initial_pc();
    } else if (method->has_tag(TAG_BUILTIN)) {
        Builtin_ptr builtin = (Builtin_ptr) method;
	    if (trace_flag) trace("op_call builtin\n");
        Builtin_fn_ptr fnptr = builtin->get_fn();
        (*fnptr)(this);
        // after the call, restore the frame
        frame = frame->get_previous();
        // result is already on top of stack and
        // pc is already pointing to next instruction
        // I'm not sure if we need to test pc here
        if (!pc) running = false;
    } else {
        report_error("method or builtin expected");
        assert(false);
    }
}


void Machine::op_return()
{
    // tos is the return value -- we will transfer it to the stack
    FVal result = pop().node;
    // move the reference to previous frame to frame:
    Frame_ptr prev = frame->get_previous();
    frame = prev;

    // now restore the stack from frame:
    // first, get the from location
    long offset = FRAME_LOCAL_BASE + frame->get_method()->get_stack_offset();
    // the to location is simply the expr_stack,
    // the count is stored in the frame
    tos = expr_stack + frame->get_tos();
	if (trace_flag) trace("return frame->tos() is %d\n", frame->get_tos());
    FUnion *p = expr_stack;
    while(p < tos) {
		if (trace_flag) trace("return restore %x from %d\n", 
            frame->get_ptr(offset), offset);
		*p++ = frame->get_ptr(offset++);
	}
    // now clear the frame's copy:
    frame->set_tos(0);
    
    // now push the return value
    push(result);
	pc = frame->get_pc();
	if (trace_flag) trace("pc restored to %x\n", pc);
	if (!pc) running = false;
}


void Machine::op_retnull()
{
    // move the reference to previous frame to frame:
    Frame_ptr prev = frame->get_previous();
    frame = prev;
	// now restore the stack from frame:
	// first, get the from location
	long offset = FRAME_LOCAL_BASE + frame->get_method()->get_stack_offset();
	// the to location is simply the expr_stack,
	// the count is stored in the frame
	tos = expr_stack + frame->get_tos();
	if (trace_flag) trace("return frame->tos() is %d\n", frame->get_tos());
	FUnion *p = expr_stack;
	while(p < tos) {
		if (trace_flag) trace("return restore %x from %d\n", 
                              frame->get_ptr(offset), offset);
		*p++ = frame->get_ptr(offset++);
	}
	// now clear the frame's copy:
	frame->set_tos(0);
    // now push the return value
    push(NULL);
	pc = frame->get_pc();
	if (trace_flag) trace("pc restored to %x\n", pc);
	if (!pc) running = false;
}


void Machine::op_pop2()
{
	pop(); pop();
}


void Machine::op_pop()
{
	pop();
}


void Machine::op_jump()
{
    // the short following the pc is a relative offset:
    pc += *(int16 *)pc;
}


void Machine::op_jumpifnot()
{
	FVal v = pop().node;
	if (v == NULL) pc += *(int16 *)pc; // add the offset
	else pc++; // skip the offset
}


void Machine::op_jumpif()
{
	FVal v = pop().node;
	if (v != NULL) pc += *(int16 *)pc; // add the offset
	else pc++; // skip the offset
}


void Machine::op_cndand()
{
	FVal v = read_tos().node;
	if (v == NULL) pc += *(int16 *)pc; // add the offset
	else {
        pop();
        pc++; // skip the offset
    }
}


void Machine::op_cndor()
{
	FVal v = read_tos().node;
	if (v != NULL) pc += *(int16 *)pc; // add the offset
	else {
        pc++; // skip the offset
        pop();
    }
}



// op_send called from: send(object, selector, param1, param2, ...)
// and compiled as push object, push selector, send (creates frame), 
// param1, param, param2, param, call
//
void Machine::op_send()
{
	Symbol_ptr selector = pop().symbol->type_check(this);
	Object_ptr obj = read_tos().object->type_check(this);
	if (error_flag) return;
	Method_ptr method;
	long var_index;
	if (!obj->get_fclass()->get_symboltable()->lookup(
		    selector, &method, &var_index) || !method) {
	    method = selector->get_function();
		if (!method) {
			undefined_method(selector);
			return;
		}
		obj = NULL;
	}
	do_frame(obj, method);
}


void Machine::op_super()
{
	Symbol_ptr selector = pop().symbol->type_check(this);
	Object_ptr obj = read_tos().object->type_check(this);
	if (error_flag) return;
	Method_ptr method;
	long var_index;
	FClass_ptr cl = obj->get_fclass()->get_super();
	if (!cl->get_symboltable()->lookup(
		    selector, &method, &var_index) || !method) {
	    method = selector->get_function();
		if (!method) {
			undefined_method(selector);
			return;
		}
		obj = NULL;
	}
	do_frame(obj, method);
}


void Machine::op_xcall()
{
	union { uint16 op[2];
	        void (*fn)();
	} u;
	u.op[0] = *pc++;
	u.op[1] = *pc++; 
	(*(u.fn))();
}


// op_loopinc -- loop variable increment and load
// 
// the loop var offset is in the next word (*pc)
// the top of stack is the increment
// add increment to the loop variable and leave
// the result on the stack for the next instruction
// as an optimization, we jump directly to the next
// instruction, which must be op_looptst()
//
void Machine::op_loopinc()
{
	short offset = *pc++;
	push(read_tos().node); // duplicate the increment
	push(frame->local(offset).node); // load the local
	op_add();
	frame->set_local(offset, read_tos().node, this);
}


// op_looptst -- test and branch
// 
// must follow op_loopinc, assumes loop var is
// on top of stack, next is increment, and below
// that is the final value
//
void Machine::op_looptst()
{
	push(read_next_next_tos().node); // copy final val to tos
	op_less();
	op_jumpif();
}


// op_loopnxt -- load next array element and branch
//
// stack top has index, next is array
// pc points to address to branch to
//
void Machine::op_loopnxt()
{
	short arg = *pc++;
	long i = (long) (read_tos().flong->must_get_long(this));
	if (error_flag) return; // error -- didn't find index
	Array_ptr ap = read_next_tos().array->type_check(this);
	if (!ap) return; // error -- not an array
	long len = ap->get_array_len();
	if (i >= len) {
        pop_n(2);
        return; // normal return after last element
    }
	replace(FLong::create(i + 1, this));
	push(ap->fastref(i).node);
	pc += arg;
	return;
}


void Machine::op_exit()
{
	running = false;
	exit_flag = true;
}






    


    
