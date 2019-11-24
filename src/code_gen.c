#include "my_string.h"
#include "code_gen.h"
#include "stdio.h"
#include "stdbool.h"

#define ADD_INST(_inst)														\
	if (!append_string(&code, (_inst "\n"))) return false

#define ADD_CODE(_code)														\
	if (!append_string(&code, (_code))) return false

#define ADD_CODE_INT(_code)													\
	do {																	\
		char str[MAX_DIGITS];												\
		sprintf(str, "%d", _code);											\
		ADD_CODE(str);														\
	} while (0)

#define HEADER\
"\n .IFJcode19"\
"\n DEFVAR GF@%tmp_op1"\
"\n DEFVAR GF@%tmp_op2"\
"\n DEFVAR GF@%tmp_op3"\
"\n DEFVAR GF@%exp_result"\
"\n JUMP $$main"\

#define MAIN_START\
"\n # Main scope"\
"\n LABEL $$main"\
"\n CREATEFRAME"\
"\n PUSHFRAME"\

#define MAIN_END\
"\n # End of main scope"\
"\n POPFRAME"\
"\n CLEARS"\

#define BUILD_IN_FUNCTIONS\
 "\n# Built-in function Ord"\
 "\n LABEL $ord"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n MOVE LF@%retval nil@nil"\
 "\n DEFVAR LF@cond_range"\
 "\n LT LF@cond_range LF@%1 int@0"\
 "\n JUMPIFEQ $chr$err LF@cond_range bool@true"\
 "\n DEFVAR LF@len"\
 "\n CREATEFRAME"\
 "\n DEFVAR TF@%0"\
 "\n MOVE TF@%0 LF@%0"\
 "\n CALL $length"\
 "\n SUB LF@len TF@%retval int@1"\
 "\n GT LF@cond_range LF@%1 LF@len"\
 "\n JUMPIFEQ $ord$err LF@cond_range bool@true"\
 "\n STRI2INT LF@%retval LF@%0 LF@%1"\
 "\n JUMP $ord$ok"\
 "\n LABEL $ord$err"\
 "\n MOVE LF@%retval nil@nil"\
 "\n LABEL $ord$ok"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n# Built-in function Chr"\
 "\n LABEL $chr"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n MOVE LF@%retval nil@nil"\
 "\n DEFVAR LF@cond_range"\
 "\n LT LF@cond_range LF@%0 int@0"\
 "\n JUMPIFEQ $chr$err LF@cond_range bool@true"\
 "\n GT LF@cond_range LF@%0 int@255"\
 "\n JUMPIFEQ $chr$err LF@cond_range bool@true"\
 "\n INT2CHAR LF@%retval LF@%0"\
 "\n JUMP $chr$ok"\
 "\n LABEL $chr$err"\
 "\n EXIT int@4"\
 "\n LABEL $chr$ok"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n# Built-in function Inputs"\
 "\n LABEL $inputs"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n READ LF@%retval string"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n# Built-in function Inputi"\
 "\n LABEL $inputi"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n READ LF@%retval int"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n# Built-in function Inputf"\
 "\n LABEL $inputf"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n READ LF@%retval float"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n# Built-in function Print"\
 "\n LABEL $print"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n MOVE LF@%retval nil@nil"\
 "\n # DEFVAR LF@print_loop_cond"\
 "\n # DEFVAR LF@pr_arg"\
 "\n # LABEL $print$loop"\
 "\n # GT LF@print_loop_cond LF@%0 int@0"\
 "\n # JUMPIFEQ $end$loop LF@print_loop_cond bool@false"\
 "\n WRITE LF@%0"\
 "\n WRITE string@\032"\
 "\n # WRITE string@\010"\
 "\n # JUMP $print$loop"\
 "\n # LABEL $end$loop"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n# Built-in function Length"\
 "\n LABEL $length"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n STRLEN LF@%retval LF@%0"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # Built-in function SubStr"\
 "\n LABEL $substr"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n MOVE LF@%retval string@"\
 "\n DEFVAR LF@length_str"\
 "\n CREATEFRAME"\
 "\n DEFVAR TF@%0"\
 "\n MOVE TF@%0 LF@%0"\
 "\n CALL $length"\
 "\n MOVE LF@length_str TF@%retval"\
 "\n DEFVAR LF@ret_cond"\
 "\n LT LF@ret_cond LF@length_str int@0"\
 "\n JUMPIFEQ $substr$return LF@ret_cond bool@true"\
 "\n EQ LF@ret_cond LF@length_str int@0"\
 "\n JUMPIFEQ $substr$return LF@ret_cond bool@true"\
 "\n LT LF@ret_cond LF@%1 int@0"\
 "\n JUMPIFEQ $substr$return LF@ret_cond bool@true"\
 "\n DEFVAR LF@range"\
 "\n ADD LF@range LF@%1 int@1"\
 "\n GT LF@ret_cond LF@range LF@length_str"\
 "\n JUMPIFEQ $substr$return LF@ret_cond bool@true"\
 "\n EQ LF@ret_cond LF@%2 int@0"\
 "\n JUMPIFEQ $substr$return LF@ret_cond bool@true"\
 "\n DEFVAR LF@max_n"\
 "\n MOVE LF@max_n LF@length_str"\
 "\n SUB LF@max_n LF@max_n LF@%1"\
 "\n DEFVAR LF@edit_n_cond"\
 "\n LT LF@edit_n_cond LF@%2 int@0"\
 "\n JUMPIFEQ $substr$edit_n LF@edit_n_cond bool@true"\
 "\n GT LF@edit_n_cond LF@%2 LF@max_n"\
 "\n JUMPIFEQ $substr$edit_n LF@edit_n_cond bool@true"\
 "\n JUMP $substr$process"\
 "\n LABEL $substr$edit_n"\
 "\n MOVE LF@%2 LF@max_n"\
 "\n LABEL $substr$process"\
 "\n DEFVAR LF@index"\
 "\n MOVE LF@index LF@%1"\
 "\n DEFVAR LF@char"\
 "\n DEFVAR LF@process_loop_cond"\
 "\n LABEL $substr$process_loop"\
 "\n GETCHAR LF@char LF@%0 LF@index"\
 "\n CONCAT LF@%retval LF@%retval LF@char"\
 "\n ADD LF@index LF@index int@1"\
 "\n SUB LF@%2 LF@%2 int@1"\
 "\n GT LF@process_loop_cond LF@%2 int@0"\
 "\n JUMPIFEQ $substr$process_loop LF@process_loop_cond bool@true"\
 "\n LABEL $substr$return"\
 "\n POPFRAME"\
 "\n RETURN"\
							
string_t code;

static bool insert_built_in_functions()
{
	ADD_INST(BUILD_IN_FUNCTIONS);

	return true;
}

static bool generate_file_header()
{
	ADD_INST(HEADER);

	return true;
}

bool generate_main_scope_start()
{
	ADD_INST(MAIN_START);

	return true;
}

bool generate_main_scope_end()
{
	ADD_INST(MAIN_END);

	return true;
}

bool generate_function_start(char *function_id)
{
	ADD_CODE("\n# Start of function "); ADD_CODE(function_id); ADD_CODE("\n");

	ADD_CODE("LABEL $"); ADD_CODE(function_id); ADD_CODE("\n");
	ADD_INST("PUSHFRAME");

	return true;
}

bool generate_function_end(char *function_id)
{
	ADD_CODE("# End of function "); ADD_CODE(function_id); ADD_CODE("\n");

	ADD_CODE("LABEL $"); ADD_CODE(function_id); ADD_CODE("%return\n");
	ADD_INST("POPFRAME");
	ADD_INST("RETURN");

	return true;
}

bool generate_var_declare(char *var_id)
{
	ADD_CODE("DEFVAR LF@"); ADD_CODE(var_id); ADD_CODE("\n");

	return true;
}