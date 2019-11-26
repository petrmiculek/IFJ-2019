#ifndef HEADER_ERROR_STATES
#define HEADER_ERROR_STATES
/**
 * @name IFJ19Compiler
 * @authors xmicul08 (Mičulek Petr)
            xjacko04 (Jacko Daniel)
            xsetin00 (Setinský Jiří)
            xsisma01 (Šišma Vojtěch)
 */

enum RET_CODES
{
    RET_OK = 0,
    RET_LEXICAL_ERROR = 1,
    RET_SYNTAX_ERROR = 2,
    RET_SEMANTICAL_ERROR = 3,
    RET_SEMANTICAL_RUNTIME_ERROR = 4,
    RET_SEMANTICAL_PARAMS_ERROR = 5,
    RET_SEMANTICAL_OTHER_ERROR = 6,
    RET_INTERNAL_ERROR = 99,
    WARNING_NOT_IMPLEMENTED = 404
};

#endif // HEADER_ERROR_STATES
