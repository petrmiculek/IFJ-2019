/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */
#include "my_string.h"
#include "code_gen.h"
#include "stdio.h"
#include "stdbool.h"
#include "err.h"
#include "parser.h"
#include "psa.h"
#include "exp_stack.h"
#include <stdlib.h>
#include <string.h>

static int append_res = 0;

#define buffer_length 10
#define CODE_APPEND(string) if ((append_res = (int)append_c_string_to_string(&code, (string))) != RET_OK) \
                                return append_res;

#define CODE_APPEND_AND_EOL(string) if ((append_res = (int) append_c_string_to_string(&code, (string "\n"))) != RET_OK)\
                                return append_res;

#define CODE_APPEND_VALUE(value)                         \
do {                                                     \
        char buffer[(buffer_length)];                    \
        snprintf(buffer, (buffer_length), "%d", value);  \
        CODE_APPEND(buffer);                             \
    } while (0);                                         \


#define HEADER \
"\n .IFJcode19"\
"\n DEFVAR GF@%tmp_op1"\
"\n DEFVAR GF@%tmp_op2"\
"\n DEFVAR GF@%tmp_op3"\
"\n DEFVAR GF@%exp_result"\
"\n JUMP $$main"\

#define MAIN_START \
"\n # Main scope"\
"\n LABEL $$main"\
"\n CREATEFRAME"\
"\n PUSHFRAME"\

#define MAIN_END \
"\n # End of main scope"\
"\n POPFRAME"\
"\n CLEARS"\

#define BUILT_IN_FUNCTIONS \
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
 "\n MOVE LF@%retval string@" /* TODO string@what? */  \
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

int
init_code_string()
{
    return (int) init_string(&code);
}

void
print_code_string()
{
    fprintf(stdout, "%s", code.str);
}

int
insert_built_in_functions()
{
    CODE_APPEND_AND_EOL(BUILT_IN_FUNCTIONS);

    return RET_OK;
}

int
generate_var_declare(char *var_id)
{
    CODE_APPEND("DEFVAR LF@");
    CODE_APPEND(var_id);
    CODE_APPEND("\n");

    return RET_OK;
}

int
generate_file_header()
{
    CODE_APPEND_AND_EOL(HEADER);

    return RET_OK;
}
int
generate_main_scope_start()
{
    CODE_APPEND_AND_EOL(MAIN_START);

    return RET_OK;
}

int
generate_main_scope_end()
{
    CODE_APPEND_AND_EOL(MAIN_END);

    return RET_OK;
}

int
generate_function_start(char *function_id)
{
    CODE_APPEND("\n# Start of function ")
    CODE_APPEND(function_id)
    CODE_APPEND("\n")

    CODE_APPEND("LABEL $")
    CODE_APPEND(function_id)
    CODE_APPEND("\n")
    CODE_APPEND_AND_EOL("PUSHFRAME")
    CODE_APPEND_AND_EOL("DEFVAR LF@%retval")
    CODE_APPEND_AND_EOL("MOVE LF@%retval nil@nil")
    // next up, arguments

    return RET_OK;
}

int
generate_function_end(char *function_id)
{
    CODE_APPEND("# End of function ")
    CODE_APPEND(function_id)
    CODE_APPEND("\n")

    CODE_APPEND("LABEL $")
    CODE_APPEND(function_id)
    CODE_APPEND("%return\n")
    CODE_APPEND_AND_EOL("POPFRAME")
    CODE_APPEND_AND_EOL("RETURN")

    return RET_OK;
}

int
generate_unique_number()
{
    static int current_result = 0;

    return ++current_result;
}

string_t *
generate_unique_identifier(const char *prefix_scope, const char *prefix_type)
{
    string_t *dest;
    if (NULL == (dest = calloc(sizeof(string_t), 1)))
        return NULL;

    init_string(dest);

    append_c_string_to_string(dest, prefix_scope);
    append_char_to_string(dest, '$');
    append_c_string_to_string(dest, prefix_type);
    append_char_to_string(dest, '$');

    int tmp_buffer_size = 10;
    char tmp[tmp_buffer_size];
    memset(tmp, 0, tmp_buffer_size);

    snprintf(tmp, tmp_buffer_size, "%d", generate_unique_number());
    append_c_string_to_string(dest, tmp);

    return dest;
}

int
generate_write(sym_table_item *identifier, bool scope)
{
    CODE_APPEND("WRITE ")

    if (scope == local)
    {
        CODE_APPEND("LF@")
    }
    else
    {
        CODE_APPEND("GF@")
    }

    CODE_APPEND(identifier->identifier.str)
    CODE_APPEND("\n")

    return RET_OK;
}

int
generate_function_call(string_t *identifier)
{
    CODE_APPEND("CALL ")
    CODE_APPEND(identifier->str)
    CODE_APPEND("\n")

    return RET_OK;
}

int
generate_function_param(int param_number, string_t *identifier, bool scope)
{
    CODE_APPEND("DEFVAR ")
    CODE_APPEND("TF@%")
    CODE_APPEND_VALUE(param_number)
    CODE_APPEND("\n")

    CODE_APPEND("MOVE TF@%")
    CODE_APPEND_VALUE(param_number)
    CODE_APPEND(" ")

    if (scope == local)
    {
        CODE_APPEND("LF@")
    }
    else
    {
        CODE_APPEND("GF@")
    }

    CODE_APPEND(identifier->str)
    CODE_APPEND("\n")

    return RET_OK;
}

int
generate_operand(string_t operand, int tmp, unsigned int symbol, int frame)
{
    CODE_APPEND(" MOVE ")
    CODE_APPEND("GL@%tmp_op")
    CODE_APPEND_VALUE(tmp)
    switch (symbol)
    {
        case OP_INT:
        {
            CODE_APPEND(" int@")
            CODE_APPEND(operand.str)
            CODE_APPEND("\n")
            return RET_OK;
        }
        case OP_DOC:
        {
            CODE_APPEND(" string@")
            CODE_APPEND(operand.str)
            CODE_APPEND("\n")
            return RET_OK;
        }
        case OP_FLOAT:
        {
            CODE_APPEND(" float@")
            CODE_APPEND(operand.str)
            CODE_APPEND("\n")
            return RET_OK;
        }
        case OP_NONE:
        {
            CODE_APPEND(" nil@")
            CODE_APPEND_AND_EOL("nil")
            return RET_OK;
        }
        case OP_STR:
        {
            CODE_APPEND(" string@")
            CODE_APPEND(operand.str)
            CODE_APPEND("\n")
            return RET_OK;
        }
        case OP_ID:
        {
            //TODO 
            if(frame == 1)
                CODE_APPEND(" LF@")
                CODE_APPEND(operand.str)
                CODE_APPEND("\n")
            else
            {
                CODE_APPEND(" GF@")
                CODE_APPEND(operand.str)
                CODE_APPEND("\n")
            }
            
                return RET_OK;
        }
        default:
            break;
    }
    return RET_INTERNAL_ERROR;
}

int
generate_operation(sem_t op1, sem_t op2, int result, unsigned int rule)
{
    switch(rule)
    {
        case R_PLUS:
        {
            CODE_APPEND(" ADD ")
            break;
        }
        case R_MIN:
        {
            CODE_APPEND(" SUB ")
            break;
        }
        case R_MUL:
        {
            CODE_APPEND(" MUL ")
            break;
        }
        case R_DIV:
        {
            CODE_APPEND(" DIV ")
            break;
        }
        case R_IDIV:
        {
            CODE_APPEND(" IDIV ")
            break;
        }
        default:
        {
           return RET_INTERNAL_ERROR;
        }

    }
    CODE_APPEND("GF@%tmp_op")
    CODE_APPEND_VALUE(result)
    CODE_APPEND(" GF@%")
    CODE_APPEND(op1.sem_data.str)
    CODE_APPEND(" GF@%")
    CODE_APPEND(op2.sem_data.str)
    CODE_APPEND("\n")
    return RET_OK;
}

int
generate_result(sem_t result)
{
    CODE_APPEND(" MOVE ")
    CODE_APPEND("GL@%exp_result ")
    CODE_APPEND("GF@%")
    CODE_APPEND(result.sem_data.str)
    CODE_APPEND("\n")
    return RET_OK;
}

int
generate_retype(sem_t op, int to)
{
    if(to == 1) //to float
    {
        CODE_APPEND(" INT2FLOAT ")
        CODE_APPEND("GF@")
        CODE_APPEND(op.sem_data.str)
        CODE_APPEND(" GF@")
        CODE_APPEND(op.sem_data.str)
        CODE_APPEND("\n")
        return RET_OK;
    }
    else
    {
        CODE_APPEND(" FLOAT2INT ")
        CODE_APPEND("GF@")
        CODE_APPEND(op.sem_data.str)
        CODE_APPEND(" GF@")
        CODE_APPEND(op.sem_data.str) 
        CODE_APPEND("\n")
        return RET_OK;
    }
    

}

int
generate_relop(sem_t op1, sem_t op2, int result, unsigned int rule)
{
    switch(rule)
    {
        case R_EA:
        {
            CODE_APPEND(" GT ")
            break;
        }
        case R_A:
        {
            CODE_APPEND(" GT ")
            break;
        }
        case R_EL:
        {
            CODE_APPEND(" LT ")
            break;
        }
        case R_L:
        {
            CODE_APPEND(" LT ")
            break;
        }
        case R_EQ:
        {
            CODE_APPEND(" EQ ")
            break;
        }
        case R_NE:
        {
            CODE_APPEND(" EQ ")
            break;
        }
        default:
        {      
           return RET_INTERNAL_ERROR;

        }
    
    }
    CODE_APPEND("GF@%tmp_op")
    CODE_APPEND_VALUE(result)
    CODE_APPEND(" GF@%")
    CODE_APPEND(op1.sem_data.str)
    if(rule != NE)
    {
        CODE_APPEND(" GF@%")
        CODE_APPEND(op2.sem_data.str)
        CODE_APPEND("\n")
    }
    else
    {
        CODE_APPEND("\n")
    }
    
    if(rule == R_EA || rule == R_EL || rule == R_NE)
    {
        CODE_APPEND(" NOT ")
        CODE_APPEND("GF@%tmp_op")
        CODE_APPEND_VALUE(result)
        CODE_APPEND(" GF@%tmp_op")
        CODE_APPEND_VALUE(result)
        CODE_APPEND("\n")

    }
    return RET_OK; 
}