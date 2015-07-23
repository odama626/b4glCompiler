#include "winasm.h"


using namespace std;

extern string value;
extern void emitLn(string);
extern void postLabel(string);
extern bool inTable(string);
extern void undefined(string);
extern string newLabel();

/////////////////////////////////////////////////////
////////////////////////////////////////////////////
/// CPU SPECIFIC CODES! ///////////////////////////
//////////////////////////////////////////////////
/////////////////////////////////////////////////

//write header info
void header() {
  emitLn("global main");
  emitLn("extern _GetStdHandle@4");
  emitLn("extern _WriteFile@20");
  emitLn("extern _ExitProcess@4");
  emitLn("");
  emitLn("section .data");
  //work around for write function
  // TODO find a way to mov eax into rsi
  // and remove the byte buffer
  emitLn("global_byte_buffer: DB 0");
  emitLn("global_byte_buffer_end:");
}

//write the prolog
void prolog() {
  emitLn("section .text");
  postLabel("main");
}

//write the epilog
void epilog() {
  emitLn("push 0");
  emitLn("call _ExitProcess@4");
}

//Clear the primary register
void Clear() { emitLn("xor rax,rax"); }

//negate the primary register
void Negate() { emitLn("neg rax"); }

//load a constant value to primary register
void LoadConst(string n) {
  emitLn("mov rax," + n);
}

//load a variable to primary register
void LoadVar(string name) {
  stringstream ss;
  if (!inTable(name)) {
      ss << name;
    undefined(ss.str());
  }
  ss << "mov ax,[" << name << "]";
  emitLn(ss.str());
}

//push Primary onto stack
void Push() {
  emitLn("push rax");
}

//add top of stack to primary
void PopAdd() {
  emitLn("pop rbx");
  emitLn("add rax,rbx");
}

//subtract primary from top of stack
void PopSub() {
  emitLn("pop rbx");
  emitLn("sub rax,rbx");
  emitLn("neg rax");
}

//multiply top of stack by primary
void PopMul() {
  emitLn("pop rbx");
  emitLn("mul rbx");
}

//divide top of stack by primary
void PopDiv() {
  emitLn("mov rbx,rax");
  emitLn("pop rax");
  emitLn("div rbx");
}

//store primary to variable
void StoreVar(string name) {
  stringstream ss;
  if (!inTable(name)) {
    ss << name;
    undefined(ss.str());
  }
  ss << "mov [" << name << "], ax";
  emitLn(ss.str());
}

//complement the primary register
void NotIt() { emitLn("not rax");}

//and top of stack with primary
void PopAnd() {
  emitLn("pop rbx");
  emitLn("and rax, rbx");
}

//or top of stack with primary
void PopOr() {
  emitLn("pop rbx");
  emitLn("or rax, rbx");
}

//XOR top of stack with primary
void PopXor() {
  emitLn("pop rbx");
  emitLn("xor rax, rbx");
}

//compare top of stack with primary
void PopCompare() {
  emitLn("pop rbx");
  emitLn("cmp rax, rbx");
}

//set primary equal
void setEqual() {
  string eq = newLabel();
  string neq = newLabel();
  emitLn("jne "+neq);
  emitLn("mov rax, 1");
  emitLn("jmp "+eq);
  postLabel(neq);
  emitLn("mov rax, 0");
  postLabel(eq);
}

//set primary not equal
void setNEqual() {
  string eq = newLabel();
  string neq = newLabel();
  emitLn("je "+neq);
  emitLn("mov rax, 1");
  emitLn("jmp "+eq);
  postLabel(neq);
  emitLn("mov rax, 0");
  postLabel(eq);
}

//set primary to less <
void setLess() {
  string eq = newLabel();
  string neq = newLabel();
  emitLn("jg "+eq);
  emitLn("mov rax, 0"); // set al to false
  emitLn("jmp "+neq);
  postLabel(eq);
  emitLn("mov rax, 1"); // set al to true
  postLabel(neq);
}

//set primary if compare was >
void setGreater() {
  string eq = newLabel();
  string neq = newLabel();
  emitLn("jl "+eq);
  emitLn("mov rax, 0"); // set ax to false
  emitLn("jmp "+neq);
  postLabel(eq);
  emitLn("mov rax, 1"); // set ax to true
  postLabel(neq);
}

//set primary if compare was <=
void setLessOrEqual() {
  string eq = newLabel();
  string neq = newLabel();
  emitLn("jge "+eq);
  emitLn("mov rax, 0"); // set ax to false
  emitLn("jmp "+neq);
  postLabel(eq);
  emitLn("mov rax, 1"); // set ax to true
  postLabel(neq);

}

//set primary if compare was >=
void setGreaterOrEqual() {
  string eq = newLabel();
  string neq = newLabel();
  emitLn("jle "+eq);
  emitLn("mov rax, 0"); // set ax to false
  emitLn("jmp "+neq);
  postLabel(eq);
  emitLn("mov rax, 1"); // set ax to true
  postLabel(neq);
}

//branch unconditional
void branch(string tag) {
  emitLn("JMP "+tag);
}

//branch not equal
void branchFalse(string tag) {
  emitLn("cmp rax, 0");
  emitLn("JE "+tag);
}

//call a subroutine
void call(string s) {
  emitLn("call "+s);
}

//return from subroutine
void Return() {
  emitLn("RET");
}

//read variable to primary register
void readIt(string val) {
  emitLn("mov rax, 3"); // eax = 3 for write
  emitLn("mov rbx, 0"); // standard input
  emitLn("mov rcx, "+val);
  emitLn("mov rdx, 1");
  emitLn("syscall");
}

/*
mov bx 7
mov ah 0eh
mov al, 'x'
int 10h



*/

//write variable from primary register
void writeIt() {
  emitLn("push -11");
  emitLn("call _GetStdHandle@4");
  emitLn("mov rbx, rax");
  emitLn("push 0");
  emitLn("lea rax, [ebp-8]");
  emitLn("push rax");
  emitLn("push (global_byte_buffer_end-global_byte_buffer)");
  emitLn("push global_byte_buffer");
  emitLn("push rbx");
  emitLn("call _WriteFile@20");



  /*//emitLn("mov [global_byte_buffer], al"); // temporary work around
  emitLn("mov ah, 1"); // the system interprets 4 as write
  emitLn("mov rdi, 1"); // standard output (terminal)
  emitLn("mov bx, 7");
  //emitLn("mov al, global_byte_buffer");
  emitLn("mov rdx, 1");
  emitLn("int 10h");*/
}

//adjust the stack pointer upwards by n bytes
void cleanStack(int n) {
  stringstream ss;
  ss << "add rsp, " << n;
  emitLn(ss.str());
}

//////////////////////////////////////////////
/////////////////////////////////////////////
/// END CPU SPECIFIC CODES /////////////////
///////////////////////////////////////////
//////////////////////////////////////////
