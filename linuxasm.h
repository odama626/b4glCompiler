#ifndef LINUX_ASM_H
#define LINUX_ASM_H

#include <string>
#include <sstream>
#include <iostream>

//write header info
void header();

//write the prolog
void prolog();

//write the epilog
void epilog();

//Clear the primary register
void Clear();

//negate the primary register
void Negate();

//load a constant value to primary register
void LoadConst(std::string);

//load a variable to primary register
void LoadVar(std::string);

//push Primary onto stack
void Push();

//add top of stack to primary
void PopAdd();

//subtract primary from top of stack
void PopSub();

//multiply top of stack by primary
void PopMul();

//divide top of stack by primary
void PopDiv();

//store primary to variable
void StoreVar(std::string);

//load a parameter to the primary register
void loadParam(int n);

//store a parameter from the primary register
void storeParam(int n);

//complement the primary register
void NotIt();

//and top of stack with primary
void PopAnd();

//or top of stack with primary
void PopOr();

//XOR top of stack with primary
void PopXor();

//compare top of stack with primary
void PopCompare();

//set primary equal
void setEqual();

//set primary not equal
void setNEqual();

//set primary to less <
void setLess();

//set primary if compare was >
void setGreater();

//set primary if compare was <=
void setLessOrEqual();

//set primary if compare was =>
void setGreaterOrEqual();

//branch unconditional
void branch(std::string);

//branch not equal
void branchFalse(std::string);

//call a subroutine
void call(std::string);

//return from subroutine
void Return();

//read variable to primary register
void readIt(std::string);

//write variable from primary register
void writeIt();

//adjust the stack pointer upwards by n bytes
void cleanStack(int);

#endif // LINUX_ASM_H

