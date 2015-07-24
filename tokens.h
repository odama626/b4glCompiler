#ifndef TOKENS_H
#define TOKENS_H

#include <string>

//keywords and token types

// constants to determine OS
int OS_LINUX      = 0;
int OS_WINDOWS    = 1;

const int SYM_DIGIT     = -1;
const int SYM_IDENT     = 0; // must be 0 to prevent infinite loops

const int SYM_IF        = 1;
const int SYM_ELSE      = 2;
const int SYM_ENDIF     = 3;
const int SYM_WHILE     = 4;
const int SYM_WEND      = 5;
const int SYM_DIM       = 6;
const int SYM_MAIN      = 7;
const int SYM_END_MAIN  = 8;
const int SYM_READ      = 9;
const int SYM_WRITE     = 10;
const int SYM_SUB       = 11;
const int SYM_END_SUB   = 12;

//const int VAR_INT       = 0; // integers
const int VAR_PARAM     = 10;// sub parameters
        //SYM_SUB       = 11;   subroutines

const int OPERATOR_OFFSET = 999;
const int OP_OR         = 1000; // |
const int OP_XOR        = 1001; // ~
const int OP_ADD        = 1002; // +
const int OP_SUB        = 1003; // -
const int OP_MULT       = 1004; // *
const int OP_DIV        = 1005; // /

const int OP_REL_E      = 1006; // =
const int OP_REL_NE     = 1007; // # <>
const int OP_REL_L      = 1008; // <
const int OP_REL_G      = 1009; // >
const int OP_PAR_O      = 1010; // (
const int OP_PAR_C      = 1011; // )
const int OP_REL_N      = 1012; // !
const int OP_REL_A      = 1013; // &
const int OP_COMMA      = 1014; // ,
const int OP_SEMICOLON  = 1015; // ;

const int TYPE_INT      = 0;
const int TYPE_CHAR     = 1;
const int TYPE_LONG     = 2;
const int TYPE_STRING   = 3;
const int TYPE_FLOAT    = 4;

const int TYPE_SUB      = 11;


const int KEYWORD_COUNT = 12;
const int OPERATOR_COUNT = 16;
std::string operatorList[] = {"|","~","+","-","*","/","=","#","<",">","(",")","!","&", ",",";"};
std::string keywordList[] = {"if", "else", "endif", "while", "wend", "dim", "main", "endmain", "read", "write", "sub", "endsub"};
std::string value;
int token;

#endif // TOKENS_H
