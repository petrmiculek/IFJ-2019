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

#define buffer_length 25
#define CODE_APPEND(string) if ((append_res = (int)append_c_string_to_string(&code, (string))) != RET_OK) \
                                return append_res;

#define CODE_APPEND_AND_EOL(string) if ((append_res = (int) append_c_string_to_string(&code, (string "\n"))) != RET_OK)\
                                return append_res;

#define CODE_APPEND_VALUE_INT(value)                     \
do {                                                     \
        char buffer[(buffer_length)];                    \
        snprintf(buffer, (buffer_length), "%d", value);  \
        CODE_APPEND(buffer);                             \
    } while (0);                                         \


#define CODE_APPEND_AS_FLOAT(string)                     \
do {                                                     \
        double float_value = strtod((string), NULL);\
        char buffer[(buffer_length)];                    \
        snprintf(buffer, (buffer_length), "%a", float_value);  \
        CODE_APPEND(buffer);                             \
    } while (0);                                         \

#define HEADER \
".IFJcode19"\
"\n DEFVAR GF@%tmp_op1"\
"\n DEFVAR GF@%tmp_op2"\
"\n DEFVAR GF@%tmp_op3"\
"\n DEFVAR GF@%tmp_op1$type"\
"\n DEFVAR GF@%tmp_op2$type"\
"\n DEFVAR GF@%tmp_op3$type"\
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

#define CONVERT_TO_BOOL \
"\n LABEL convert%to%bool"\
"\n PUSHFRAME"\
"\n CREATEFRAME"\
"\n DEFVAR LF@compare"\
"\n TYPE LF@compare GF@%exp_result"\
"\n JUMPIFEQ exp_result%is%string LF@compare string@string"\
"\n JUMPIFEQ exp_result%is%int LF@compare string@int"\
"\n JUMPIFEQ exp_result%is%float LF@compare string@float"\
"\n JUMPIFEQ convert%to%bool%end LF@compare string@bool"\
"\n #exp_result is nil"\
"\n MOVE GF@%exp_result bool@false"\
"\n JUMP convert%to%bool%end"\
\
"\n LABEL exp_result%is%string"\
"\n JUMPIFNEQ result%true  GF@%exp_result string@"\
"\n MOVE GF@%exp_result bool@false"\
"\n JUMP convert%to%bool%end"\
\
"\n LABEL exp_result%is%int"\
"\n JUMPIFNEQ result%true  GF@%exp_result int@0"\
"\n MOVE GF@%exp_result bool@false"\
"\n JUMP convert%to%bool%end"\
\
"\n LABEL exp_result%is%float"\
"\n JUMPIFNEQ result%true  GF@%exp_result float@0x0p+0"\
"\n MOVE GF@%exp_result bool@false"\
"\n JUMP convert%to%bool%end"\
\
"\n LABEL result%true"\
"\n MOVE GF@%exp_result bool@true"\
\
"\n LABEL convert%to%bool%end"\
"\n POPFRAME"\
"\n RETURN"\


#define BUILT_IN_FUNCTIONS \
 "\n# Built-in function Ord"\
 "\n LABEL $ord"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n MOVE LF@%retval nil@nil"\
 "\n DEFVAR LF@type0"\
 "\n DEFVAR LF@type1"\
 "\n TYPE LF@type0 LF@%0"\
 "\n TYPE LF@type1 LF@%1"\
 "\n JUMPIFNEQ $EXIT$ord LF@type0 string@string"\
 "\n JUMPIFNEQ $EXIT$ord LF@type1 string@int"\
 "\n JUMP $OK$ord"\
 "\n LABEL $EXIT$ord"\
 "\n EXIT int@4"\
 "\n LABEL $OK$ord"\
 "\n DEFVAR LF@cond_range"\
 "\n LT LF@cond_range LF@%1 int@0"\
 "\n JUMPIFEQ $chr$err LF@cond_range bool@true"\
 "\n DEFVAR LF@len"\
 "\n CREATEFRAME"\
 "\n DEFVAR TF@%0"\
 "\n MOVE TF@%0 LF@%0"\
 "\n CALL $len"\
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
 "\n DEFVAR LF@type0"\
 "\n TYPE LF@type0 LF@%0"\
 "\n JUMPIFEQ $OK$chr LF@type0 string@int"\
 "\n EXIT int@4"\
 "\n LABEL $OK$chr"\
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
 "\n# Built-in function Length"\
 "\n LABEL $len"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@type"\
 "\n TYPE LF@type LF@%0"\
 "\n JUMPIFEQ $OK$length LF@type string@string"\
 "\n EXIT int@4"\
 "\n LABEL $OK$length"\
 "\n STRLEN LF@%retval LF@%0"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # Built-in function SubStr"\
 "\n LABEL $substr"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n MOVE LF@%retval string@"\
 "\n DEFVAR LF@type0"\
 "\n DEFVAR LF@type1"\
 "\n DEFVAR LF@type2"\
 "\n TYPE LF@type0 LF@%0"\
 "\n TYPE LF@type1 LF@%1"\
 "\n TYPE LF@type2 LF@%2"\
 "\n JUMPIFNEQ $EXIT$substr LF@type0 string@string"\
 "\n JUMPIFNEQ $EXIT$substr LF@type1 string@int"\
 "\n JUMPIFNEQ $EXIT$substr LF@type2 string@int"\
 "\n JUMP $OK$substr"\
 "\n LABEL $EXIT$substr"\
 "\n EXIT int@4"\
 "\n LABEL $OK$substr"\
 "\n DEFVAR LF@length_str"\
 "\n CREATEFRAME"\
 "\n DEFVAR TF@%0"\
 "\n MOVE TF@%0 LF@%0"\
 "\n CALL $len"\
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

#define ARITHMETIC1_SEMANTICS_FUNCTIONS \
 "\n # semantic_plus"\
 "\n LABEL $semantics_runtime_check_plus"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_plus LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $OP1_INT$semantics_runtime_check_plus LF@op1_type string@int"\
 "\n JUMPIFEQ $OP2_INT$semantics_runtime_check_plus LF@op2_type string@int"\
 "\n LABEL $EXIT$semantics_runtime_check_plus"\
 "\n EXIT int@4"\
 "\n LABEL $OP1_INT$semantics_runtime_check_plus"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_plus LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_plus"\
 "\n LABEL $OP2_INT$semantics_runtime_check_plus"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_plus LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_plus"\
 "\n LABEL $OK$semantics_runtime_check_plus"\
 "\n JUMPIFEQ $CONCAT$semantics_runtime_check_plus LF@op1_type string@string"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_plus LF@op1_type string@nil"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_plus LF@op1_type string@bool"\
 "\n ADD LF@%retval LF@op1 LF@op2"\
 "\n JUMP $RET$semantics_runtime_check_plus"\
 "\n LABEL $CONCAT$semantics_runtime_check_plus"\
 "\n CONCAT LF@%retval LF@op1 LF@op2"\
 "\n LABEL $RET$semantics_runtime_check_plus"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # semantic_min"\
 "\n LABEL $semantics_runtime_check_min"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_min LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $OP1_INT$semantics_runtime_check_min LF@op1_type string@int"\
 "\n JUMPIFEQ $OP2_INT$semantics_runtime_check_min LF@op2_type string@int"\
 "\n LABEL $EXIT$semantics_runtime_check_min"\
 "\n EXIT int@4"\
 "\n LABEL $OP1_INT$semantics_runtime_check_min"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_min LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_min"\
 "\n LABEL $OP2_INT$semantics_runtime_check_min"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_min LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_min"\
 "\n LABEL $OK$semantics_runtime_check_min"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_min LF@op1_type string@string"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_min LF@op1_type string@nil"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_min LF@op1_type string@bool"\
 "\n SUB LF@%retval LF@op1 LF@op2"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # semantic_mul"\
 "\n LABEL $semantics_runtime_check_mul"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_mul LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $OP1_INT$semantics_runtime_check_mul LF@op1_type string@int"\
 "\n JUMPIFEQ $OP2_INT$semantics_runtime_check_mul LF@op2_type string@int"\
 "\n LABEL $EXIT$semantics_runtime_check_mul"\
 "\n EXIT int@4"\
 "\n LABEL $OP1_INT$semantics_runtime_check_mul"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_mul LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_mul"\
 "\n LABEL $OP2_INT$semantics_runtime_check_mul"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_mul LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_mul"\
 "\n LABEL $OK$semantics_runtime_check_mul"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_mul LF@op1_type string@string"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_mul LF@op1_type string@nil"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_mul LF@op1_type string@bool"\
 "\n MUL LF@%retval LF@op1 LF@op2"\
 "\n POPFRAME"\
 "\n RETURN"\

#define ARITHMETIC2_SEMANTICS_FUNCTIONS \
 "\n # semantic_div"\
 "\n LABEL $semantics_runtime_check_div"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_div LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $OP1_INT$semantics_runtime_check_div LF@op1_type string@int"\
 "\n JUMPIFEQ $OP2_INT$semantics_runtime_check_div LF@op2_type string@int"\
 "\n LABEL $EXIT$semantics_runtime_check_div"\
 "\n EXIT int@4"\
 "\n LABEL $OP1_INT$semantics_runtime_check_div"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_div LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_div"\
 "\n LABEL $OP2_INT$semantics_runtime_check_div"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_div LF@op1_type LF@op2_type"\
 "\n JUMP $EXIT$semantics_runtime_check_div"\
 "\n LABEL $OK$semantics_runtime_check_div"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_div LF@op1_type string@string"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_div LF@op1_type string@nil"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_div LF@op1_type string@bool"\
 "\n JUMPIFEQ $FLOAT$semantics_runtime_check_div LF@op1_type string@float"\
 "\n JUMPIFNEQ $NOT_ZERO$semantics_runtime_check_div LF@op2 int@0"\
 "\n EXIT int@9"\
 "\n LABEL $FLOAT$semantics_runtime_check_div"\
 "\n JUMPIFNEQ $NOT_ZERO$semantics_runtime_check_div LF@op2 float@0x0p+0"\
 "\n EXIT int@9"\
 "\n LABEL $NOT_ZERO$semantics_runtime_check_div"\
 "\n DIV LF@%retval LF@op1 LF@op2"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # semantic_idiv"\
 "\n LABEL $semantics_runtime_check_idiv"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $OK$semantics_runtime_check_idiv LF@op1_type LF@op2_type"\
 "\n LABEL $EXIT$semantics_runtime_check_idiv"\
 "\n EXIT int@4"\
 "\n LABEL $OK$semantics_runtime_check_idiv"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_idiv LF@op1_type string@string"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_idiv LF@op1_type string@float"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_idiv LF@op1_type string@nil"\
 "\n JUMPIFEQ $EXIT$semantics_runtime_check_idiv LF@op1_type string@bool"\
 "\n JUMPIFNEQ $NOT_ZERO$semantics_runtime_check_idiv LF@op2 int@0"\
 "\n EXIT int@9"\
 "\n LABEL $NOT_ZERO$semantics_runtime_check_idiv"\
 "\n IDIV LF@%retval LF@op1 LF@op2"\
 "\n POPFRAME"\
 "\n RETURN"\

#define RELATIONAL1_SEMANTICS_FUNCTIONS \
 "\n # semantic_a"\
 "\n LABEL $semantics_runtime_check_a"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $a$semantics_runtime_check_a LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $op1_int$semantics_runtime_check_a LF@op1_type string@int"\
 "\n JUMPIFEQ $op2_int$semantics_runtime_check_a LF@op2_type string@int"\
 "\n JUMP exit$semantics_runtime_check_a"\
 "\n LABEL $op1_int$semantics_runtime_check_a"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $a$semantics_runtime_check_a LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_a"\
 "\n LABEL $op2_int$semantics_runtime_check_a"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $a$semantics_runtime_check_a LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_a"\
 "\n LABEL exit$semantics_runtime_check_a"\
 "\n EXIT int@4"\
 "\n LABEL $a$semantics_runtime_check_a"\
 "\n GT LF@%retval LF@op1 LF@op2"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # semantic_ea"\
 "\n LABEL $semantics_runtime_check_ea"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $ea$semantics_runtime_check_ea LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $op1_int$semantics_runtime_check_ea LF@op1_type string@int"\
 "\n JUMPIFEQ $op2_int$semantics_runtime_check_ea LF@op2_type string@int"\
 "\n JUMP exit$semantics_runtime_check_ea"\
 "\n LABEL $op1_int$semantics_runtime_check_ea"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $ea$semantics_runtime_check_ea LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_ea"\
 "\n LABEL $op2_int$semantics_runtime_check_ea"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $ea$semantics_runtime_check_ea LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_ea"\
 "\n LABEL exit$semantics_runtime_check_ea"\
 "\n EXIT int@4"\
 "\n LABEL $ea$semantics_runtime_check_ea"\
 "\n LT LF@%retval LF@op1 LF@op2"\
 "\n NOT LF@%retval LF@%retval"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # semantic_l"\
 "\n LABEL $semantics_runtime_check_l"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $l$semantics_runtime_check_l LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $op1_int$semantics_runtime_check_l LF@op1_type string@int"\
 "\n JUMPIFEQ $op2_int$semantics_runtime_check_l LF@op2_type string@int"\
 "\n JUMP exit$semantics_runtime_check_l"\
 "\n LABEL $op1_int$semantics_runtime_check_l"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $l$semantics_runtime_check_l LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_l"\
 "\n LABEL $op2_int$semantics_runtime_check_l"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $l$semantics_runtime_check_l LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_l"\
 "\n LABEL exit$semantics_runtime_check_l"\
 "\n EXIT int@4"\
 "\n LABEL $l$semantics_runtime_check_l"\
 "\n LT LF@%retval LF@op1 LF@op2"\
 "\n POPFRAME"\
 "\n RETURN"\

#define RELATIONAL2_SEMANTICS_FUNCTIONS \
 "\n # semantic_el"\
 "\n LABEL $semantics_runtime_check_el"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $el$semantics_runtime_check_el LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $op1_int$semantics_runtime_check_el LF@op1_type string@int"\
 "\n JUMPIFEQ $op2_int$semantics_runtime_check_el LF@op2_type string@int"\
 "\n JUMP exit$semantics_runtime_check_el"\
 "\n LABEL $op1_int$semantics_runtime_check_el"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $el$semantics_runtime_check_el LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_el"\
 "\n LABEL $op2_int$semantics_runtime_check_el"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $el$semantics_runtime_check_el LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_el"\
 "\n LABEL exit$semantics_runtime_check_el"\
 "\n EXIT int@4"\
 "\n LABEL $el$semantics_runtime_check_el"\
 "\n GT LF@%retval LF@op1 LF@op2"\
 "\n NOT LF@%retval LF@%retval"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # semantic_eq"\
 "\n LABEL $semantics_runtime_check_eq"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $eq$semantics_runtime_check_eq LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $op1_int$semantics_runtime_check_eq LF@op1_type string@int"\
 "\n JUMPIFEQ $op2_int$semantics_runtime_check_eq LF@op2_type string@int"\
 "\n JUMPIFNEQ $neq$semantics_runtime_check_eq LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_eq"\
 "\n LABEL $op1_int$semantics_runtime_check_eq"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $eq$semantics_runtime_check_eq LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_eq"\
 "\n LABEL $op2_int$semantics_runtime_check_eq"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $eq$semantics_runtime_check_eq LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_eq"\
 "\n LABEL exit$semantics_runtime_check_eq"\
 "\n EXIT int@4"\
 "\n LABEL $neq$semantics_runtime_check_eq"\
 "\n EQ LF@%retval int@0 int@1"\
 "\n JUMP end"\
 "\n LABEL $eq$semantics_runtime_check_eq"\
 "\n EQ LF@%retval LF@op1 LF@op2"\
 "\n LABEL end"\
 "\n POPFRAME"\
 "\n RETURN"\
 \
 "\n # semantic_ne"\
 "\n LABEL $semantics_runtime_check_ne"\
 "\n PUSHFRAME"\
 "\n DEFVAR LF@%retval"\
 "\n DEFVAR LF@op1_type"\
 "\n MOVE LF@op1_type LF@%1"\
 "\n DEFVAR LF@op2_type"\
 "\n MOVE LF@op2_type LF@%2"\
 "\n DEFVAR LF@op1"\
 "\n MOVE LF@op1 LF@%3"\
 "\n DEFVAR LF@op2"\
 "\n MOVE LF@op2 LF@%4"\
 "\n JUMPIFEQ $ne$semantics_runtime_check_ne LF@op1_type LF@op2_type"\
 "\n JUMPIFEQ $op1_int$semantics_runtime_check_ne LF@op1_type string@int"\
 "\n JUMPIFEQ $op2_int$semantics_runtime_check_ne LF@op2_type string@int"\
 "\n JUMPIFNEQ $n_eq$semantics_runtime_check_ne LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_ne"\
 "\n LABEL $op1_int$semantics_runtime_check_ne"\
 "\n INT2FLOAT LF@op1 LF@op1"\
 "\n TYPE LF@op1_type LF@op1"\
 "\n JUMPIFEQ $ne$semantics_runtime_check_ne LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_ne"\
 "\n LABEL $op2_int$semantics_runtime_check_ne"\
 "\n INT2FLOAT LF@op2 LF@op2"\
 "\n TYPE LF@op2_type LF@op2"\
 "\n JUMPIFEQ $ne$semantics_runtime_check_ne LF@op1_type LF@op2_type"\
 "\n JUMP exit$semantics_runtime_check_ne"\
 "\n LABEL exit$semantics_runtime_check_ne"\
 "\n EXIT int@4"\
 "\n LABEL $n_eq$semantics_runtime_check_ne"\
 "\n EQ LF@%retval int@1 int@1"\
 "\n JUMP end$semantics_runtime_check_ne"\
 "\n LABEL $ne$semantics_runtime_check_ne"\
 "\n EQ LF@%retval LF@op1 LF@op2"\
 "\n NOT LF@%retval LF@%retval"\
 "\n LABEL end$semantics_runtime_check_ne"\
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
    CODE_APPEND_AND_EOL(BUILT_IN_FUNCTIONS)
    CODE_APPEND_AND_EOL(ARITHMETIC1_SEMANTICS_FUNCTIONS)
    CODE_APPEND_AND_EOL(ARITHMETIC2_SEMANTICS_FUNCTIONS)
    CODE_APPEND_AND_EOL(RELATIONAL1_SEMANTICS_FUNCTIONS)
    CODE_APPEND_AND_EOL(RELATIONAL2_SEMANTICS_FUNCTIONS)

    return RET_OK;
}
int
insert_convert_to_bool_function()
{
    CODE_APPEND_AND_EOL(CONVERT_TO_BOOL)

    return RET_OK;
}
int
generate_var_declare(char *var_id, bool is_scope_local)
{
    CODE_APPEND("DEFVAR ");

    if (is_scope_local == local)
    {
        CODE_APPEND("LF@")
    }
    else
    {
        CODE_APPEND("GF@")
    }

    CODE_APPEND(var_id);
    CODE_APPEND("\n");

    return RET_OK;

}
int
generate_file_header()
{
    CODE_APPEND_AND_EOL(HEADER)

    return RET_OK;
}
int
generate_main_scope_start()
{
    CODE_APPEND_AND_EOL(MAIN_START)

    return RET_OK;
}
int
generate_main_scope_end()
{
    CODE_APPEND_AND_EOL(MAIN_END)

    return RET_OK;
}
int
generate_function_start(char *function_id)
{
    CODE_APPEND("JUMP end$")
    CODE_APPEND(function_id)

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
    CODE_APPEND_AND_EOL("POPFRAME")
    CODE_APPEND_AND_EOL("RETURN")
    CODE_APPEND("# End of function ")
    CODE_APPEND(function_id)
    CODE_APPEND("\n")
    CODE_APPEND("LABEL end$")
    CODE_APPEND(function_id)
    CODE_APPEND("\n")

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

// token_t chenge to strint_t please

int
append_identifier(const token_t *token, const data_t *data)
{
    table_t *table;
    if (data->parser_in_local_scope == local)
    {
        CODE_APPEND("LF@")
        table = data->local_sym_table;
    }
    else
    {
        CODE_APPEND("GF@")
        table = data->global_sym_table;
    }
    ht_item_t *identifier = ht_search(table, token->string.str);

    if (identifier == NULL)
    {
        fprintf(stderr, "# %s, %d: identifier (%d, %s) not found in (%s)\n",
                __func__, __LINE__,
                token->type, token->string.str,
                (data->parser_in_local_scope == local ? "local" : "global"));

        return RET_SEMANTICAL_ERROR;
    }

    char *token_uniq_identifier = identifier->data->identifier.str;

    CODE_APPEND(token_uniq_identifier)

    return RET_OK;
}

int
append_identifier_string(string_t string, const data_t *data)
{
    ht_item_t *identifier;
    if (data->parser_in_local_scope == local)
    {
        CODE_APPEND(" LF@")
        identifier = ht_search(data->local_sym_table, string.str);
        if (identifier == NULL)
        {
            identifier = ht_search(data->global_sym_table, string.str);
        }

    }
    else
    {
        CODE_APPEND(" GF@")
        identifier = ht_search(data->global_sym_table, string.str);
    }

    if (identifier == NULL)
    {
        fprintf(stderr, "# %s, %s, %d: identifier (%s) not found in (%s)\n",
                __FILE__, __func__, __LINE__,
                string.str,
                (data->parser_in_local_scope == local ? "local" : "global"));

        return RET_SEMANTICAL_ERROR;
    }

    char *token_uniq_identifier = identifier->data->identifier.str;

    CODE_APPEND(token_uniq_identifier)

    return RET_OK;
}

int
generate_write(token_t *token, data_t *data)
{
    int res;

    CODE_APPEND("WRITE ")

    if (token->type == TOKEN_IDENTIFIER)
    {
        if ((res = append_identifier(token, data)) != RET_OK)
        {
            return res;
        }
    }
    else if (token->type == TOKEN_INT)
    {
        CODE_APPEND("int@")
        CODE_APPEND(token->string.str)
    }
    else if (token->type == TOKEN_LIT
        || token->type == TOKEN_DOC)
    {
        CODE_APPEND("string@")
        CODE_APPEND(token->string.str)
    }
    else if (token->type == TOKEN_FLOAT)
    {
        CODE_APPEND("float@")
        CODE_APPEND_AS_FLOAT(token->string.str)
    }
    else if (token->type == TOKEN_NONE)
    {
        CODE_APPEND("string@None")
    }
    else
    {
        fprintf(stderr, "# %s, %s, %u: invalid parameter passed (%d, %s)",
                __FILE__, __func__, __LINE__,
                token->type, token->string.str);

        return RET_SEMANTICAL_ERROR;
    }

    CODE_APPEND("\n") // EOL in IFJCode source-code

    return RET_OK;
}

int
generate_print_space_or_newline(char str)
{
    CODE_APPEND("WRITE ")
    CODE_APPEND("string@")

    if (str == ' ')
    {
        CODE_APPEND("\\032")
    }
    else if (str == '\n')
    {
        CODE_APPEND("\\010")
    }

    CODE_APPEND("\n")

    return RET_OK;
}

int
generate_function_call(string_t *identifier)
{
    CODE_APPEND("CALL $")
    CODE_APPEND(identifier->str)
    CODE_APPEND("\n")
    CODE_APPEND_AND_EOL("MOVE GF@%exp_result TF@%retval")

    return RET_OK;
}

int
generate_function_param(data_t *data)
{
    int param_number = 0;
    token_t *param;
    CODE_APPEND_AND_EOL("CREATEFRAME")
    while (data->call_params->first != NULL)
    {
        param = q_pop(data->call_params);

        CODE_APPEND("DEFVAR ")
        CODE_APPEND("TF@%")
        CODE_APPEND_VALUE_INT(param_number)
        CODE_APPEND("\n")

        CODE_APPEND("MOVE TF@%")
        CODE_APPEND_VALUE_INT(param_number)
        CODE_APPEND(" ")

        int res = RET_OK;

        switch (param->type)
        {
            case TOKEN_FLOAT:CODE_APPEND("float@")
                CODE_APPEND_AS_FLOAT(param->string.str)
                CODE_APPEND("\n")
                break;
            case TOKEN_IDENTIFIER:res = append_identifier_string(param->string, data);
                RETURN_IF_ERR(res);

                CODE_APPEND("\n")
                break;
            case TOKEN_DOC:
            case TOKEN_LIT:CODE_APPEND("string@")
                CODE_APPEND(param->string.str)
                CODE_APPEND("\n")
                break;
            case TOKEN_INT:CODE_APPEND("int@")
                CODE_APPEND(param->string.str)
                CODE_APPEND("\n")
                break;
            case TOKEN_NONE:CODE_APPEND_AND_EOL("nil@nil")
                break;

            default:break;
        }

        param_number++;
    }
    return RET_OK;
}

int
generate_operand(string_t operand, int tmp, unsigned int symbol, data_t *data)
{
    CODE_APPEND(" MOVE ")
    CODE_APPEND("GF@%tmp_op")
    CODE_APPEND_VALUE_INT(tmp)
    int res = RET_OK;
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
            CODE_APPEND_AS_FLOAT(operand.str)
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
            res = append_identifier_string(operand, data);
            RETURN_IF_ERR(res);

            CODE_APPEND("\n")

            return RET_OK;
        }
        default:break;
    }
    return RET_INTERNAL_ERROR;

}

int
generate_result(sem_t result)
{
    CODE_APPEND(" MOVE ")
    CODE_APPEND("GF@%exp_result ")
    CODE_APPEND("GF@%")
    CODE_APPEND(result.sem_data.str)
    CODE_APPEND("\n")
    return RET_OK;
}

int
generate_retype(sem_t op, int to)
{
    if (to == 1) //to float
    {
        CODE_APPEND(" INT2FLOAT ")
        CODE_APPEND("GF@%")
        CODE_APPEND(op.sem_data.str)
        CODE_APPEND(" GF@%")
        CODE_APPEND(op.sem_data.str)
        CODE_APPEND("\n")
        return RET_OK;
    }
    else
    {
        CODE_APPEND(" FLOAT2INT ")
        CODE_APPEND("GF@%")
        CODE_APPEND(op.sem_data.str)
        CODE_APPEND(" GF@%")
        CODE_APPEND(op.sem_data.str)
        CODE_APPEND("\n")
        return RET_OK;
    }

}

int
typecheck(sem_t *op1, sem_t *op2, unsigned int rule, int result)
{
    int res;
    if ((res = defvar_type(op1)) != RET_OK)
    {
        return res;
    }

    if ((res = defvar_type(op2)) != RET_OK)
    {
        return res;
    }
    CODE_APPEND_AND_EOL(" CREATEFRAME")

    CODE_APPEND_AND_EOL(" DEFVAR TF@%1")
    CODE_APPEND(" MOVE TF@%1 ")
    CODE_APPEND("GF@%");
    CODE_APPEND(op1->sem_data.str);
    CODE_APPEND_AND_EOL("$type");

    CODE_APPEND_AND_EOL(" DEFVAR TF@%2")
    CODE_APPEND(" MOVE TF@%2 ")
    CODE_APPEND("GF@%");
    CODE_APPEND(op2->sem_data.str);
    CODE_APPEND_AND_EOL("$type");

    CODE_APPEND_AND_EOL(" DEFVAR TF@%3")
    CODE_APPEND(" MOVE TF@%3 ")
    CODE_APPEND("GF@%");
    CODE_APPEND(op1->sem_data.str);
    CODE_APPEND("\n")

    CODE_APPEND_AND_EOL(" DEFVAR TF@%4")
    CODE_APPEND(" MOVE TF@%4 ")
    CODE_APPEND("GF@%");
    CODE_APPEND(op2->sem_data.str);
    CODE_APPEND("\n")

    switch (rule)
    {
        case R_PLUS:

            CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_plus");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;

        case R_MIN:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_min");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;

        case R_MUL:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_mul");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;

        case R_DIV:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_div");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;

        case R_IDIV:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_idiv");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;

        case R_A:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_a");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;
        case R_EA:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_ea");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;
        case R_EQ:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_eq");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;
        case R_L:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_l");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;
        case R_EL:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_el");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;
        case R_NE:CODE_APPEND_AND_EOL(" CALL $semantics_runtime_check_ne");
            CODE_APPEND(" MOVE GF@%tmp_op");
            CODE_APPEND_VALUE_INT(result);
            CODE_APPEND_AND_EOL(" TF@%retval");
            break;

        default:break;
    }
    return RET_OK;
}

int
defvar_type(sem_t *op)
{
    CODE_APPEND(" TYPE GF@%");
    CODE_APPEND(op->sem_data.str);
    CODE_APPEND("$type ");
    CODE_APPEND("GF@%");
    CODE_APPEND(op->sem_data.str);
    CODE_APPEND("\n");
    return RET_OK;
}

int
compiler_ret_value_comment(int retval)
{
    CODE_APPEND("# compiler finished with return code: ")
    CODE_APPEND_VALUE_INT(retval)
    return RET_OK;
}

int
generate_move_exp_result_to_variable(token_t *token, data_t *data)
{
    CODE_APPEND("MOVE ");
    append_identifier(token, data);
    CODE_APPEND(" GF@%exp_result");
    CODE_APPEND("\n");

    return RET_OK;
}

int
generate_if_begin(char *label)
{
    CODE_APPEND_AND_EOL("CREATEFRAME")
    CODE_APPEND("CALL convert%to%bool\n")
    CODE_APPEND("JUMPIFEQ ")
    CODE_APPEND(label)
    CODE_APPEND(" GF@%exp_result bool@false\n")

    return RET_OK;
}

int
generate_if_else(char *label)
{
    CODE_APPEND("JUMP ")
    CODE_APPEND(label)
    CODE_APPEND("%end\n")

    CODE_APPEND("LABEL ")
    CODE_APPEND(label)
    CODE_APPEND("\n")

    return RET_OK;
}

int
generate_if_end(char *label)
{
    CODE_APPEND("LABEL ")
    CODE_APPEND(label)
    CODE_APPEND("%end\n")

    return RET_OK;
}

int
generate_while_label(char *label, bool first)
{
    if (first)
    {
        CODE_APPEND("DEFVAR LF@counter%")
        CODE_APPEND(label)
        CODE_APPEND("\n")

        CODE_APPEND("MOVE ")
        CODE_APPEND("LF@counter%")
        CODE_APPEND(label)
        CODE_APPEND(" int@0")
        CODE_APPEND("\n")
    }

    CODE_APPEND("LABEL ")
    CODE_APPEND(label)
    CODE_APPEND("\n")

    return RET_OK;
}

int
generate_while_begin(char *label)
{
    CODE_APPEND_AND_EOL("CREATEFRAME")
    CODE_APPEND("CALL convert%to%bool\n")
    CODE_APPEND("JUMPIFEQ ")
    CODE_APPEND(label)
    CODE_APPEND("%end")
    CODE_APPEND(" GF@%exp_result bool@false\n")

    return RET_OK;

}

int
generate_while_end(char *label)
{
    CODE_APPEND("JUMP ")
    CODE_APPEND(label)
    CODE_APPEND("\n")

    CODE_APPEND("LABEL ")
    CODE_APPEND(label)
    CODE_APPEND("%end\n")

    return RET_OK;
}

int
generate_var_declare_while(char *var_id, char *label, int counter, bool is_scope_local)
{

    CODE_APPEND("JUMPIFNEQ declaration%while%")
    CODE_APPEND(label)
    CODE_APPEND_VALUE_INT(counter)
    CODE_APPEND(" LF@counter%")
    CODE_APPEND(label)
    CODE_APPEND(" int@")
    CODE_APPEND_VALUE_INT(counter)
    CODE_APPEND("\n")

    generate_var_declare(var_id, is_scope_local);

    CODE_APPEND("ADD ")
    CODE_APPEND("LF@counter%")
    CODE_APPEND(label)
    CODE_APPEND(" LF@counter%")
    CODE_APPEND(label)
    CODE_APPEND(" int@1")
    CODE_APPEND("\n")

    CODE_APPEND("LABEL declaration%while%")
    CODE_APPEND(label)
    CODE_APPEND_VALUE_INT(counter)
    CODE_APPEND("\n")

    return RET_OK;

}

int
defvar_param(data_t *data)
{
    int res = RET_OK;
    CODE_APPEND("DEFVAR ")
    res = append_identifier_string(data->token->string, data);
    RETURN_IF_ERR(res);
    CODE_APPEND("\nMOVE ")
    res = append_identifier_string(data->token->string, data);
    RETURN_IF_ERR(res);
    CODE_APPEND(" LF@%")
    CODE_APPEND_VALUE_INT(data->function_call_param_count)
    CODE_APPEND("\n")
    return RET_OK;
}

int
retval()
{
    CODE_APPEND_AND_EOL("MOVE LF@%retval GF@%exp_result")
    return RET_OK;
}
