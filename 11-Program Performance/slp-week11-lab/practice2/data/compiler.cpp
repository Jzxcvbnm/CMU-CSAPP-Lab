/* compiler -- */

#include "flincl.h"
#include "stdio.h"
#include "shortstack.h"
#include "semantics.h"
#include "compiler.h"
#include "ctype.h"
#include "assert.h"
#include "string.h"

#define ctrace if (machine->trace_flag) trace

Compiler::Compiler()
{
}


bool Compiler::init(Machine_ptr m)
{
	machine = m;
	return semantics.init(machine, this);
}


void Compiler::expected_close_squote()
{
    report_error("expected close single quote");
}


void Compiler::expected_close_quote()
{
    report_error("expected close quote");
}


void Compiler::expected_char_const()
{
    report_error("expected character constant");
}


void Compiler::expected_close_paren_or_formal()
{
    report_error("expected close paren or formal parameter");
}


void Compiler::expected_close_paren()
{
    report_error("expected close paren");
}


void Compiler::expected_close_bracket()
{
    report_error("expected close bracket");
}


void Compiler::expected_equal()
{
    report_error("expected equal");
}


void Compiler::expected_indent()
{
    report_error("Compiler:: expected indent");
}


void Compiler::expected_newline()
{
    report_error("expected newline");
}


void Compiler::expected_open_paren()
{
    report_error("expected open paren");
}


void Compiler::expected_open_brace()
{
    report_error("expected open brace");
}

void Compiler::expected_to()
{
    report_error("expected to");
}


void Compiler::expected_token()
{
    report_error("expected more");
}


void Compiler::expected_colon()
{
    report_error("expected colon");
}


void Compiler::expected_comma()
{
    report_error("expected comma");
}


void Compiler::expected_statement()
{
    report_error("expected statement");
}


void Compiler::expected_method_or_field_selector()
{
    report_error("expected method or field selector");
}


void Compiler::expected_decl()
{
    report_error("expected declaration");
}


void Compiler::expected_hex_constant()
{
	report_error("expected hexadecimal constant");
}


void Compiler::expected_oct_constant()
{
	report_error("expected octal constant");
}


void Compiler::expected_number_constant()
{
	report_error("expected number constant");
}


void Compiler::no_digit_in_hex()
{
	report_error("no digits found in hexidecimal constant");
}


void Compiler::no_digit_in_exponent()
{
	report_error("no digit found in exponent");
}



void Compiler::expected_id(char *msg)
{
    char s[100];
    strcpy(s, "expected class name");
    strcat(s, msg);
    report_error(s);
}


void Compiler::expected_variable()
{
    report_error("expected variable name");
}


void Compiler::expected_expression()
{
	report_error("expected expression");
}


void Compiler::indentation_error()
{
    report_error("indentation error");
}

void Compiler::expected_equal_or_in()
{
	report_error("expected = or in after for variable");
}

void Compiler::report_error(char *s)
{
	char msg[128];
	if (!token) {
	    sprintf(msg, "Error: %s", s);
	} else {
		if (token->typ == TOKEN_INDENT) strcpy(token->str, "[indentation]");
		else if (token->typ == TOKEN_DEDENT) strcpy(token->str, "[dedentation]");
		else if (token->typ == TOKEN_NEWLINE) strcpy(token->str, "[newline]");
		sprintf(msg, "Error in line %d near \"%s\": %s", 
		         line_number, token->str, s);
	}
	machine->error_msg(msg);
	error_flag = true;
}


// get_token -- returns true if token is found
//
bool Compiler::get_token()
{
    other_token();
    if (unget_count) {
        unget_count--;
        // ctrace("Token: (ungot), %s\n", token->str);
        return true;
    }
    token->typ = TOKEN_OTHER;
    int i = 0;
    int c;
    if (dedent_count) {
        token->typ = TOKEN_DEDENT;
        dedent_count--;
        token->str[0] = 0;
        ctrace("Token: DEDENT\n");
        return true;
    }
    while ((c = getc(inf)) != EOF) { // this loops over white space
		i = 0;
        token->str[i++] = c;
		if (c == '#') {
			while ((c = getc(inf)) != EOF && c != '\n') ;
			if (c == '\n') ungetc(c, inf);
			continue;
		}
        if (c == '\n') {
			line_number++;
			if (newline_flag) { // (user types from terminal)
				if (interactive_mode) {
					// blank line generates DEDENTs in interactive mode
					while (0 < indentation[istack_top]) {
						dedent_count++;
						istack_top--;
					}
					if (0 != indentation[istack_top]) {
						indentation_error();
					}
					if (dedent_count > 0) {
						dedent_count--;
						token->typ = TOKEN_DEDENT;
						ctrace("Token: DEDENT");
					} else {
						continue; // ignore 2nd blank line
					}
					i = 0;
				} else continue; // ignore blank lines
			} else {
				token->typ = TOKEN_NEWLINE;
				newline_flag = true;
				ctrace("Token: NEWLINE");
			}
        } else if (newline_flag) {
            newline_flag = false;
            int indent = 0;
            while (isspace(c)) {
                indent++;
                if ((c = getc(inf)) == EOF) break;
            }
            ungetc(c, inf);
            if (c == '#' || c == '\n') {
                newline_flag = true;
                continue; // ignore blank lines
            }
            // generate indent or dedent?
            if (indent > indentation[istack_top]) {
                token->typ = TOKEN_INDENT;
                indentation[++istack_top] = indent;
                i = 0; // take space out of token
                ctrace("Token: INDENT");
            } else if (indent < indentation[istack_top]) {
                while (indent < indentation[istack_top]) {
                    dedent_count++;
                    istack_top--;
                }
                if (indent != indentation[istack_top]) {
                    indentation_error();
                }
                dedent_count--;
                token->typ = TOKEN_DEDENT;
                i = 0;
                ctrace("Token: DEDENT");
            } else { // no indentation, newline_flag is off
                continue;
            }
        } else if (isspace(c)) {
            continue; // ignore spaces after initial ones handled above
        } else if (__iscsymf(c)) {
            newline_flag = false;
            /* get alphanumeric */
            while ((c = getc(inf)) != EOF) {
                if (__iscsym(c)) {
                    token->str[i++] = c;
                } else {
                    ungetc(c, inf);
                    break;
                }
            }
            token->typ = TOKEN_ID;
            ctrace("Token: ID");
        } else if (isdigit(c) || c == '-') {
			char first_digit = 0;
            newline_flag = 0;  // index of first digit in string
			if (c == '-') {
				first_digit = 1;
			}
            /* get number */
            while ((c = getc(inf)) != EOF) {
                if (isdigit(c)) {
                    token->str[i++] = c;
                } else {
                    break;
                }
            }
            if (c == '.' || tolower(c) == 'e') {
                token->typ = TOKEN_DOUBLE;
                if (c == '.') token->str[i++] = c;
				else ungetc(c, inf); // unget 'e'

                while ((c = getc(inf)) != EOF) {
                    if (isdigit(c)) {
                        token->str[i++] = c;
                    } else {
                        ungetc(c, inf);
                        ctrace("Token: DOUBLE");
                        break;
                    }
                }
				// may be followed by exponent "e+006"
				if ((c = getc(inf)) != EOF) {
					if (tolower(c) == 'e') {
						token->str[i++] = 'e';
						if ((c = getc(inf)) != EOF) {
							if ((c == '+') || (c == '-')) {
								token->str[i++] = c;
							} else {
								ungetc(c, inf);
							}
							int expcount = 0;
							while ((c = getc(inf)) != EOF) {
								if (isdigit(c)) {
									expcount++;
									token->str[i++] = c;
								} else {
									ungetc(c, inf);
									if (expcount == 0) {
										token->str[i] = 0;
										no_digit_in_exponent();
									}
									break;
								}
							}
						}
					}
				}
			} else if ((i == first_digit + 1) &&           // enough for "0x"
					   (token->str[first_digit] == '0') && // got the "0"
					   (tolower(c) == 'x')) {              // got the "x"
                token->typ = TOKEN_LONG;
				token->str[i++] = c;
				int digit_count = 0;
				while ((c = getc(inf)) != EOF) {
					if (isxdigit(c)) {
						digit_count = 0;
						token->str[i++] = c;
					} else {
						if (digit_count == 0) {
							token->str[i] = 0;
							no_digit_in_hex();
						}
                        ungetc(c, inf);
                        ctrace("Token: LONG");
						break;
					}
				}
            } else {
                ungetc(c, inf);
                token->typ = TOKEN_LONG;
                ctrace("Token: LONG");
            }
        } else if (c == '"') {
            newline_flag = false;
			// leave quotes in... i = 0; // skip the quote
            int quote_count = 0;
            token->typ = TOKEN_STRING;
            while ((c = getc(inf)) != EOF) {
                token->str[i++] = c;
                if (c == '"') {
                    if (quote_count == 0) {
                        quote_count = 1;
                    } else /* quote_count == 1 */ {
                        i = i - 1; // erase second quote
                        quote_count = 0;
                    }
                } else if (quote_count == 1) {
                    // we closed the quote
                    ungetc(c, inf);
					i--;
                    ctrace("Token: STRING");
                    break;
                } else if (c == '\\') { // escape character
                    c = getc(inf);
                    if (c == EOF) break;
                    token->str[i - 1] = c;
                }
            }
            if (quote_count != 1) {
                expected_close_quote();
            } else {
				i--; // back over the close quote
			}
        } else if (c == '\'') {
            newline_flag = false;
			// leave quotes in ... i = 0; // skip the quote
            int quote_count = 0;
            token->typ = TOKEN_SYMBOL;
            while ((c = getc(inf)) != EOF) {
                token->str[i++] = c;
                if (c == '\'') {
                    if (quote_count == 0) {
                        quote_count = 1;
                    } else /* quote_count == 1 */ {
                        i = i - 1; // erase second quote
                        quote_count = 0;
                    }
                } else if (quote_count == 1) {
                    // we closed the quote
                    ungetc(c, inf);
					i--;
                    ctrace("Token: SYMBOL");
                    break;
                } else if (c == '\\') { // escape character
                    c = getc(inf);
                    if (c == EOF) break;
                    token->str[i - 1] = c;
                }
            }
            if (quote_count != 1) {
                expected_close_quote();
            } else {
				i--; // back over the close quote
			}

        } else if (c == '>' || c == '<' || c == '!' || c == '=') {
            newline_flag = false;
			if ((c = getc(inf)) != EOF) {
                if ((c == '=') ||
                    (token->str[i - 1] == '>' && c == '>') ||
                    (token->str[i - 1] == '<' && c == '<')) {
				    token->str[i++] = c;
                } else ungetc(c, inf);
            }
			ctrace("Token: OTHER");
        } else if (c == '*') {
            newline_flag = false;
            if ((c = getc(inf)) != EOF && (c == '*')) {
                token->str[i++] = c;
            } else ungetc(c, inf);
            ctrace("Token: OTHER");
        } else {
            newline_flag = false;
            ctrace("Token: OTHER");
        }
		token->str[i] = 0;
		if (token->typ != TOKEN_NEWLINE) ctrace(", '%s'\n", token->str);
		else ctrace(", '\\n'\n");
        return !error_flag;
    }
	if (istack_top > 0) {
		istack_top--;
		token->typ = TOKEN_DEDENT;
        token->str[0] = 0;
        ctrace("Token: DEDENT\n");
        return true;
	}
	ctrace("Token: none\n");
    return false; // false and no error_flag means end of file
}


// unget_token -- back up to previous token
// (currently supports up to two levels of unget)
// This is more complex than I expected. token is a pointer
// to either token1 or token2, and unget_count tells how
// many levels we have ungotten.  The key "trick" is that
// unget always changes token to the other buffer, and get
// always changes token back.
//
bool Compiler::unget_token()
{
    assert(unget_count < 2);
    // ctrace("unget_token\n");
    unget_count++;
    other_token();
    return true;
}


// compile_file -- compile from opened file handle.
//
// interactive indicates that blank lines should terminate
// compilation of a top-level statement or declaration
// (normally blank lines are ignored, but this does not 
// work well for user type-in)
//
bool Compiler::compile_file(FILE *file, bool interactive)
{
	interactive_mode = interactive;
	inf = file; // so that get_token() will read from file
	if (!semantics.reset()) return false;
    error_flag = false;
    token = tokens;
    unget_count = 0;
    token->typ = TOKEN_OTHER;
    istack_top = 0;
    indentation[0] = 0;
    dedent_count = 0;
	newline_flag = false;
	line_number = 1;
    while (!machine->exit_flag && parse_decl()) ;
	return !error_flag;
}


bool Compiler::compile(char *filename)
{
    FILE *f = fopen(filename, "r");
	bool result;
    if (f) {
		result = compile_file(f, false);
		fclose(f);
	} else {
        char msg[128];
        strcpy(msg, "Compiler: can't open ");
        strcat(msg, filename);
        report_error(msg);
		result = false;
    }
	return result;
}


bool Compiler::parse_decl()
{
    if (get_token()) {
        return parse_decl_2();
    }
    return false;
}


bool Compiler::parse_decl_2()
{
    if (token->typ == TOKEN_ID) {
        if (streql(token->str, "def")) {
            return parse_method();
        } else if (streql(token->str, "var")) {
            return parse_var();
        } else if (streql(token->str, "class")) {
            return parse_class();
        } else {
            // assume we have an immediate statement
            // create a method for it
            semantics.method_start();
            semantics.method_name_is("<immediate command>");
            semantics.begin_formals();
            semantics.end_of_formals();
            if (parse_stmt_2() && semantics.method_end()) {
                // execute the method now
                Symbol_ptr s_imcd = Symbol::create("<immediate command>", machine);
                machine->global_function(s_imcd);
                if (machine->error_flag) return false;
                machine->call();
                return !machine->error_flag;
            }
            return false; // allow immediate stmt in place of decl
        }
    } else if (token->typ == TOKEN_NEWLINE) {
        return true;
    }
    return false;
}


bool Compiler::parse_id(char *msg)
{
    if (get_token() && token->typ == TOKEN_ID) return true;
    expected_id(msg);
    return false;
}


bool Compiler::parse_method()
{
    if (get_token()) {
        if (token->typ == TOKEN_ID &&
            semantics.method_start() &&
            semantics.method_name_is(token->str)) {
            if (parse_formals() &&
				parse_colon() &&
                parse_stmt_list() && 
                semantics.method_end()) {
                return true;
            }
        }
    }
    return false;
}

bool Compiler::parse_formals()
{
    semantics.begin_formals();
    if (!get_token() || !streql(token->str, "(")) {
        expected_open_paren();
        return false;
    }
    if (!parse_token()) return false;
    bool more = true;
    bool first = true; // allow ')' only on first time through loop
    while (more) {
        if (token->typ == TOKEN_ID) {
            bool ok = true;
            if ((streql(token->str, "optional") && 
                 (ok = semantics.set_formal_type(FORMAL_OPTIONAL))) ||
                (streql(token->str, "keyword") &&
                 (ok = semantics.set_formal_type(FORMAL_KEYWORD))) ||
                (streql(token->str, "rest") &&
                 (ok = semantics.set_formal_type(FORMAL_REST))) ||
                (streql(token->str, "dictionary") &&
                 (ok = semantics.set_formal_type(FORMAL_DICTIONARY))) ||
                (streql(token->str, "required") &&
                 (ok = semantics.set_formal_type(FORMAL_REQUIRED)))) {
                if (!parse_id("formal parameter")) return false;
            }
            // if a set_formal_type failed, return now:
            if (!ok) return false;
            if (!semantics.formal_is(token->str)) return false;
            // look for default value specification:
            if (get_token()) {
                if (streql(token->str, "=")) {
                    if (!parse_default_value()) return false;
                } else unget_token();
            } else unget_token();
            // look for comma
            if (get_token()) {
                more = streql(token->str, ",");
                if (more) {
                    if (!parse_token()) return false;
                }
            } else { // no token!
                expected_close_paren();
                return false;
            }
        } else if (first && streql(token->str, ")")) {
            more = false;
        } else break;
        first = false;
    }
    if (streql(token->str, ")")) {
        return semantics.end_of_formals();
    } else {
        expected_close_paren_or_formal();
        return false();
    }
}


int Compiler::hex_to_int(char h)
{
	if (isdigit(h)) return h - '0';
	else if (isxdigit(h)) return tolower(h) - 'a' + 10;
	expected_hex_constant();
	return 0;
}


int Compiler::oct_to_int(char o)
{
	if ((o >= '0') && (o <= '7')) return o - '0';
	expected_oct_constant();
	return 0;
}


int Compiler::dec_to_int(char d)
{
	if (isdigit(d)) return d - '0';
	expected_number_constant();
	return 0;
}


int64 Compiler::string_to_int(char *s)
{
	int64 result = 0;
	int sign = 1;
	if (!s || *s == 0) return result;
	if (*s == '-') {
		if (*s++ == 0) return result;
		sign = -1;
	} 
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { // hexadecimal
		for (int i = 2; s[i] && !error_flag; i++) {
			result = (result << 4) + hex_to_int(s[i]);
		}
		result *= sign;
	} else if (s[0] == '0') { // octal
		for (int i = 1; s[i] && !error_flag; i++) {
			result = (result << 3) + oct_to_int(s[i]);
		}
		result *= sign;
	} else {
		// since negative numbers can be larger (in magnitude) than positive
		// we'll compute decimal numbers in the negative form and then apply
		// the sign.
		for (int i = 0; s[i] && !error_flag; i++) {
			result = (result * 10) - dec_to_int(s[i]);
		}
		result *= -sign;
	}
	return result;
}


bool Compiler::parse_default_value()
{
    if (!parse_token()) return false;
    // assume value is in one token
    if (token->typ == TOKEN_LONG) {
		int64 val = string_to_int(token->str);
		if (error_flag) return false;
        return semantics.default_value(
            FLong::create(val, machine));
	} else if (token->typ == TOKEN_DOUBLE) {
		// make sure there is a + or - after e:
		char *pos = strpbrk(token->str, "eE");
		if (pos && pos[1] != '+' && pos[1] != '-') {
			// open up some space:
			memmove(pos + 2, pos + 1, strlen(pos + 1));
			pos[1] = '+';
		}
        return semantics.default_value(
            FDouble::create(atof(token->str), machine));
	} else if (token->typ == TOKEN_STRING) {
        return semantics.default_value(
            FString::create(token->str + 1, machine));
    } else if (token->typ == TOKEN_SYMBOL) {
        return semantics.default_value(
            Symbol::create(token->str + 1, machine));
	} else return false;
}


bool Compiler::parse_stmt()
{
    if (get_token()) {
        return parse_stmt_2();
    }
    expected_statement();
    return false;
}


// parse_stmt_2 -- parse stmt, assumes first token already gotten
//
bool Compiler::parse_stmt_2()
{
    if (streql(token->str, "class") || streql(token->str, "var") ||
        streql(token->str, "def")) {
        return parse_decl_2();
    } else if (streql(token->str, "if")) {
        return parse_if();
    } else if (streql(token->str, "while")) {
        return parse_while();
    } else if (streql(token->str, "for")) {
        return parse_for();
    } else if (streql(token->str, "return")) {
        return parse_return();
	} else if (streql(token->str, "print")) {
		return parse_print();
    } else if (streql(token->str, "load")) {
        return parse_load();
    } else if (token->typ == TOKEN_ID) {
        char id[STRING_MAX];
        strcpy(id, token->str);
        if (!parse_id_expr(id)) return false;
        if (get_token()) {
            if (streql(token->str, "=")) {
                return parse_assign();
            } else if (token->typ == TOKEN_NEWLINE) {
                semantics.end_id_expr();
                return true;
            }
        }
        expected_newline();
    } else {
        expected_statement();
    }
    return false;
}


bool Compiler::parse_if()
{
    // at this point, IF has already been parsed
    if (parse_expr()) {
        semantics.if_true();
        if (parse_colon() && parse_stmt_list()) {
			long elif_count = 0;
			while (get_token() && streql(token->str, "elif")) {
				semantics.if_else();
				if (parse_expr()) {
					semantics.if_true();
					if (parse_colon() && parse_stmt_list()) {
						elif_count++;
						trace("elif_count %d\n", elif_count);
					} else return false;
				} else return false;
			}
            if (streql(token->str, "else")) {
                semantics.if_else();
                if (parse_colon() && parse_stmt_list()) {
					// if_end called for each elif and for else
					while (elif_count >= 0) {
						semantics.if_end();
						elif_count--;
					}
					return true;
				} else return false;
            } else {
                unget_token();
				while (elif_count >= 0) {
					semantics.if_end();
					elif_count--;
				}
                return true;
            }
        }
    }
    return false;
}


bool Compiler::parse_while()
{
    return semantics.while_begin() && 
        parse_expr() && parse_colon() &&
        semantics.while_true() && 
        parse_stmt_list() && 
        semantics.while_end();
}


bool Compiler::parse_for()
{
	if (!parse_token()) return false;
	if (token->typ != TOKEN_ID) {
		expected_id(token->str);
		return false;
	}
	if (!semantics.for_var(token->str)) return false;
	if (!parse_token()) return false;
	else if (streql(token->str, "=")) {
		if (!parse_expr() || !semantics.for_init())
			return false;
		if (!get_token() || !streql(token->str, "to")) {
			expected_to();
			return false;
		}
		if (!(parse_expr() && semantics.for_to()))
			return false;
		if (!parse_token()) return false;
		if (streql(token->str, "by")) {
			if (!parse_expr() || !semantics.for_by(true))
				return false;
		} else {
			unget_token();
			if (!semantics.for_by(false)) return false;
		}
		if (!parse_colon() || !parse_stmt_list() || 
			!semantics.for_end_range())
			return false;
	} else if (streql(token->str, "in")) {
		if (!parse_expr() || !semantics.for_array() ||
			!parse_colon() || !parse_stmt_list() ||
			!semantics.for_end_elements())
			return false;
	} else {
		expected_equal_or_in();
		return false;
	}
	return true;
}


bool Compiler::parse_indent()
{
	if (get_token() && token->typ == TOKEN_INDENT) {
		return true;
	}
	expected_indent();
	return false;
}


bool Compiler::parse_newline()
{
    if (get_token() && token->typ == TOKEN_NEWLINE) {
        return true;
    }
    expected_newline();
    return false;
}


bool Compiler::parse_return()
{
    if (get_token() && token->typ == TOKEN_NEWLINE) {
		return semantics.return_null();
	}
    unget_token();
	return parse_expr() && parse_newline() && semantics.return_end();
}


bool Compiler::parse_load()
{
	return parse_expr() && parse_newline() && semantics.load_end();
}


bool Compiler::parse_print()
{
    bool print_endl = true;
    while (true) {
        // look for an expression
        if (!parse_token()) return false;
        if (token->typ == TOKEN_NEWLINE) {
            if (print_endl) return semantics.print_end();
            else return true;
        } else if (unget_token() && parse_expr()) {
            semantics.print_expr();
        } else return false; // no expression found
        print_endl = false;
        // look for a separator
        if (!parse_token()) return false;
        if (token->typ == TOKEN_NEWLINE) {
            return semantics.print_end();
        } else if (streql(token->str, ",")) {
            semantics.print_comma();
        } else if (streql(token->str, ";")) {
            // nothing
        } else expected_comma();
    }
}

bool Compiler::parse_stmt_list()
{
	if (!parse_token()) return false;
	if (token->typ != TOKEN_NEWLINE) {
		return parse_stmt_2(); // just one statement after a colon
	} 
	if (!parse_indent()) return false;
	while (true) {
		if (get_token()) {
			if (token->typ == TOKEN_DEDENT) {
				return true;
			} else if (parse_stmt_2()) {
				// continue parsing stmts
			} else break;
		} else {
			expected_statement();
			break;
		}
	}
	return false;
}


bool Compiler::parse_assign()
{
    // expr and "=" have already been parsed
    return semantics.rval2lval() && 
           parse_expr() && 
           parse_newline() && 
           semantics.assign_end();
}


/*
bool Compiler::parse_assign_id(char *id)
{
	return parse_expr() && parse_newline() && semantics.assign_id(id);
}
*/


bool Compiler::parse_expr()
{
    if (parse_conj()) {
        return parse_disjunctions();
    }
    return false;
}


bool Compiler::parse_conj()
{
    if (parse_relation()) {
        return parse_conjunctions();
    }
    return false;
}


bool Compiler::parse_relation()
{
    if (parse_sum()) {
        if (parse_relop()) {
            char op[8];
            strcpy(op, token->str);
            // we can insist on another token because relop must
            // be followed by an expression:
            if (!parse_token()) return false;
            // special treatment for "not in" which is two tokens:
            if (streql(op, "not") && streql(token->str, "in")) {
                strcat(op, token->str); // to make notin
            // special treatment for "is not" which is two tokens:
            } else if (streql(op, "is") && streql(token->str, "not")) {
                strcat(op, token->str); // to make isnot
            } else unget_token();
            return parse_sum() && semantics.relop(op);
        } else return true;
    }
    return false;
}


bool Compiler::parse_sum()
{
    if (parse_prod_expr()) {
        return parse_addends();
    }
    return false;
}


bool Compiler::parse_prod_expr()
{
    if (parse_factor_expr()) {
        return parse_factors();
    }
    return false;
}


bool Compiler::parse_factor_expr()
{
    if (get_token()) {
        if (streql(token->str, "-")) { // unary minus
            return parse_factor_expr() && semantics.unary_minus();
        } else if (streql(token->str, "+")) { // unary plus
            return parse_factor_expr();
        } else if (streql(token->str, "not")) { // not
            return parse_factor_expr() && semantics.not();
        } else if (streql(token->str, "~")) { // logical not
            return parse_factor_expr() && semantics.lognot();
        } else {
            unget_token();
            return parse_exp_expr();
        }
    }
    return false;
}


bool Compiler::parse_exp_expr()
{
    if (parse_inner_expr()) {
        return parse_exponents();
    }
    return false;
}




// parse_inner_expr -- parses an inner_expr or an lval_expr
//
bool Compiler::parse_inner_expr()
{
    if (get_token()) {
		if (streql(token->str, "(")) {
			if (parse_expr()) {
				if (parse_close_paren()) {
					return parse_post_inner(NULL);
				}
			}
		} else if (token->typ == TOKEN_ID) {
			char id[STRING_MAX];
			strcpy(id, token->str);
			return parse_post_inner(id);
		} else if (token->typ == TOKEN_LONG) {
			semantics.push_long(string_to_int(token->str));
			return true;
		} else if (token->typ == TOKEN_DOUBLE) {
			semantics.push_double(atof(token->str));
			return true;
        } else if (token->typ == TOKEN_SYMBOL) {
            semantics.push_symbol(token->str + 1);
            return true;
		} else if (streql(token->str, "[")) {
            // it is possible to apply built-in methods to literal array
			return parse_literal_array() && parse_post_inner(NULL);
        } else if (streql(token->str, "{")) {
            return parse_literal_dict() && parse_post_inner(NULL);
		} else if (token->typ == TOKEN_STRING) {
			semantics.push_string(token->str + 1);
			return true;
		}
	}
    return false;
}


bool Compiler::parse_literal_array()
{
	bool first = true;
	semantics.array_start();
	while (true) {
        if (!parse_token()) return false;
		if (streql(token->str, "]")) {
			return semantics.array_end();
		}
        unget_token();
		if (!first && !parse_comma()) return false;
        if (semantics.before_array_expr() && 
            parse_expr() && 
            semantics.array_expr()) {
		} else return false;
        first = false;
	}
	return false;
}

bool Compiler::parse_literal_dict()
{
	bool first = true;
	semantics.dict_start();
	while (true) {
        if (!parse_token()) return false;
		if (streql(token->str, "}")) {
			return semantics.dict_end();
		}
        unget_token();
		if (!first && !parse_comma()) return false;
        if (!(semantics.before_dict_key() && 
              parse_expr() && 
              semantics.dict_key())) {
            return false;
        }
        if (!parse_colon()) return false;
        if (!(semantics.before_dict_val() && 
              parse_expr() && 
              semantics.dict_val())) {
            return false;
        }
        first = false;
	}
	return false;
}



// parse_post_inner -- parses things that can come after an inner_expr
//
// code generation:
//    if we find an id, call semantics.id(id)
//    if we find id(...), call semantics.call(id)
//    if we find <expr>.meth(...), call semantics.method_call(meth)
//
bool Compiler::parse_post_inner(char *id)
{
    if (get_token()) {
        if (streql(token->str, ".")) {
			if (id) semantics.id(id);
            if (get_token() && (token->typ == TOKEN_ID)) {
                char member[STRING_MAX];
                strcpy(member, token->str);
                if (get_token() && streql(token->str, "(")) {
                    unget_token();
                    if (semantics.method_call(member) && parse_actuals()) {
                        return parse_post_inner(NULL);
                    }
                } else return semantics.field(member) && parse_post_inner(NULL);
            } else {
                expected_method_or_field_selector();
                return false;
            }
        } else if (streql(token->str, "[")) {
			if (id) semantics.id(id);
            if (parse_expr()) {
                if (get_token() && streql(token->str, "]")) {
                    return semantics.aref() && parse_post_inner(NULL);
                } else {
                    expected_close_bracket();
                    return false;
                }
            }
        } else if (streql(token->str, "(")) {
            unget_token();
			semantics.call(id);
            if (!parse_actuals()) return false;
            return parse_post_inner(NULL);
        } else if (id) {
            unget_token();
			semantics.id(id);
            return true;
        }
    }
	unget_token();
    return true;
}


// parse_id_expr -- when called, id is parsed
//
bool Compiler::parse_id_expr(char *id)
{
    if (parse_id_prod_expr(id)) {
        return parse_addends();
    }
    return false;
}


bool Compiler::parse_id_prod_expr(char *id)
{
    if (parse_id_inner_expr(id)) {
        return parse_factors();
    }
    return false;
}


bool Compiler::parse_id_inner_expr(char *id)
{
    // we've gotten the id, so just look for suffixes
    return parse_post_inner(id);
}


bool Compiler::parse_disjunctions()
{
    if (get_token()) {
        if (streql(token->str, "|") || streql(token->str, "or") ||
            streql(token->str, "^") || streql(token->str, "<<") ||
            streql(token->str, ">>")) {
            char op[4];
            strcpy(op, token->str);
            semantics.begin_or(op);
            if (parse_conj()) {
                return parse_disjunctions() &&
                       semantics.end_or(op);
            }
        } else {
            unget_token();
            return true;
        }
    }
    return true;
}


bool Compiler::parse_conjunctions()
{
    if (get_token()) {
        if (streql(token->str, "&") || streql(token->str, "and")) {
            char op[4];
            strcpy(op, token->str);
            semantics.begin_and(op);
            if (parse_relation()) {
                return parse_conjunctions() &&
                       semantics.end_and(op);
            }
        } else {
            unget_token();
            return true;
        }
    }
    return true;
}


bool Compiler::parse_relop()
{
    if (get_token()) {
        if (streql(token->str, "<") ||
            streql(token->str, "<=") ||
            streql(token->str, "==") ||
            streql(token->str, "!=") ||
            streql(token->str, ">") ||
            streql(token->str, ">=") ||
            streql(token->str, "is") ||
            streql(token->str, "in") ||
            streql(token->str, "not")) {
            return true;
        } else {
            unget_token();
            return false;
        }
    }
    return false;
}


bool Compiler::parse_addends()
{
    while (get_token()) {
        if (streql(token->str, "+") || streql(token->str, "-")) {
            char op[4];
            strcpy(op, token->str);
            if (parse_prod_expr()) {
                semantics.addop(op);
            } else return false;
        } else {
            unget_token();
            return true;
        }
    }
    return true;
}


bool Compiler::parse_factors()
{
    while (get_token()) {
        if (streql(token->str, "*") || streql(token->str, "/") || streql(token->str, "%")) {
            char op[4];
            strcpy(op, token->str);
            if (parse_factor_expr()) {
                semantics.mulop(op);
            } else return false;
        } else {
            unget_token();
            return true;
        }
    } 
    return true;
}


bool Compiler::parse_exponents()
{
    while (get_token()) {
        if (streql(token->str, "**")) {
            if (parse_inner_expr()) {
                semantics.power();
            } else return false;
        } else {
            unget_token();
            return true;
        }
    } 
    return true;
}



bool Compiler::parse_actuals()
{
    if (!parse_open_paren()) return false;
	bool first = true;
    bool keyword = false;
    long key_index;
    while (true) {
        if (!parse_token()) return false;
        if (streql(token->str, ")")) {
			return semantics.actual_end();
		}
		unget_token();
        if (!first && !parse_comma()) return false;
        if (!parse_token()) return false;
        if (token->typ == TOKEN_ID) {
            if (!parse_token()) return false;
            if (streql(token->str, "=")) { // keyword parameter
                keyword = true;
                unget_token(); // back up to keyword
                key_index = semantics.insert_symbol(token->str);
                get_token(); // re-get the '='
            } else {
                unget_token();
                unget_token();
            }
        } else {
            unget_token();
        }
        if (!parse_expr()) return false;
		first = false;
		if (keyword) semantics.keyword(key_index);
        else semantics.actual();
        keyword = false;
    }
}


// parse_paren_expr -- parse an expr in parens
//
bool Compiler::parse_paren_expr()
{
    return parse_open_paren() && parse_expr() && parse_close_paren();
}


// parse_class -- assume CLASS already parsed
//
bool Compiler::parse_class()
{
    char cl[STRING_MAX];    // the class name
    char su[STRING_MAX];    // the superclass name

    cl[0] = 0; su[0] = 0;
    if (!parse_id("class name")) return false;
    strcpy(cl, token->str);
    if (!parse_token()) return false;
    if (streql(token->str, "(")) {
        if (!parse_id("super name")) return false;
        strcpy(su, token->str);
        if (!parse_close_paren()) return false;
    } else unget_token();
    if (!parse_colon() || !parse_newline()) return false;
    if (!get_token() || token->typ != TOKEN_INDENT) {
        expected_decl();
        return false;
    }
	if (!semantics.class_start(cl, su)) return false;
    while (true) {
        get_token();
        if (token->typ == TOKEN_DEDENT) return true;
        unget_token();
        if (parse_decl()) {
            // loop to get next declaration
        } else {
            // expected_decl();
            return false;
        }
    }
}
        

bool Compiler::parse_colon()
{
    if (get_token() && streql(token->str, ":")) {
        return true;
    }
    expected_colon();
    return false;
}


bool Compiler::parse_comma()
{
    if (get_token() && streql(token->str, ",")) {
        return true;
    }
    expected_comma();
    return false;
}


bool Compiler::parse_open_paren()
{
    if (get_token() && streql(token->str, "(")) {
        return true;
    }
    expected_open_paren();
    return false;
}


bool Compiler::parse_token()
{
    if (get_token()) return true;
	// get_token returns false on either eof or bad token, e.g. "0x"
	// check for eof and report as error:
	if (!error_flag) expected_token();
    return false;
}

bool Compiler::parse_close_paren()
{
    if (get_token() && streql(token->str, ")")) {
        return true;
    }
    expected_close_paren();
    return false;
}


bool Compiler::parse_var()
{
    while (true) {
        if (get_token()) {
            if (token->typ != TOKEN_ID) break;
			char id[STRING_MAX];
			strcpy(id, token->str);
			semantics.var_decl(id);
            if (get_token()) {
                if (streql(token->str, "=")) {
                    if (!parse_expr()) {
                        return false;
                    }
					semantics.assign_id(id);
                } else unget_token(); // not an equal sign
            }
            if (get_token()) {
                if (token->typ = TOKEN_NEWLINE) {
                    return true;
                } else if (streql(token->str, ",")) {
                    // loop to get next variable
                }
            } else {
                expected_newline();
                return false;
            }
        } else {
            break;
        }
    }
    expected_variable();
    return false;
}


