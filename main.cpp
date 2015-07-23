#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include "tokens.h"


#ifdef __linux
  #include "linuxasm.h"
  const int CURRENT_OS = OS_LINUX;
  #define _popen NULL;
#define _pclose NULL;
#elif _WIN64
  #include "winasm.h"
  const int CURRENT_OS = OS_WIN;
  #define popen NULL;
  #define pclose NULL;
#endif

/*  BNF

<program> ::= PROGRAM <top-level decl> <main> '.'


*/
using namespace std;

const char TAB = '\t';
const char CR = '\r';
const char LF = '\n';
char look;
ifstream *inputFile = NULL;
ofstream *outputFile = NULL;

string sourceFileName;      //name of source file
string sourceFileBaseName;  //name of source file without extension

int lCount;
int lineCount; // source file lineCount

//variable table
map<string,int> variables;

map<string,int> symbolTable;

//variables for function parameters
map<string,int> params;
int paramCount;
int base;


// global variables from tokens_H
extern int token;
extern string value;
extern string keywordList[];

void expression();
void block();
void clearParams();
void boolExpression();
bool inTable(string n);

// read a new character from input stream
void getChar() {
  inputFile->get(look);
}

// report an error
void error(string s) {
  printf("\n");
  printf("Error: %s", s.c_str());
}

// report error and halt
void abort(string s) {
  error(s);
  if (inputFile != NULL) {
    inputFile->close();
    delete inputFile;
  }
  if (outputFile != NULL) {
    outputFile->flush();
    outputFile->close();
    delete outputFile;
  }
  exit(1);
}

//report what we expected
void expected(string s) {
  stringstream ss;
  ss << value << " on line " << lineCount <<" token:" << token;
  abort(s+ " Expected instead of "+ss.str());
}

void undefined(string s) {
  abort("Undefined Identifier "+s);
}

void duplicate(string s) {
  abort("Duplicate Identifier "+s);
}

void dumpSymbolTable() {
  cout << "::Symbol Table::::::::::::::::::::::::" << endl;
  for(map<string,int>::iterator it=symbolTable.begin(); it!=symbolTable.end();it++) {
    cout << it->first << TAB << TAB << TAB;
    switch(it->second) {
    case TYPE_INT:
      cout << "int";
      break;
    case TYPE_CHAR:
      cout << "char";
      break;
    case TYPE_LONG:
      cout << "long";
      break;
    case TYPE_STRING:
      cout << "string";
      break;
    case TYPE_FLOAT:
      cout << "float";
      break;
    case TYPE_SUB:
      cout << "subroutine";
      break;
    default:
      cout << "unknown token " << it->second;
    }
    cout << endl;
  }
  cout << "::::::::::::::::::::::::::::::::::::::" << endl;
}

//make sure current token is an identifier
void checkIdent() {
  if (token != SYM_IDENT)
    expected("Identifier");
}

//recognize white space
bool isWhite(char c) {
  return (c==' ' || c==TAB || c==CR || c==LF || c=='\'');
}

//recognize and skip a comment
void skipComment() {
  while (look != LF && look != CR) {
    getChar();
    if (look == '\'')
      skipComment();
  }
}

//skip over leading white space
void skipWhite() {
  while( !inputFile->eof() && isWhite(look)) {
      if (look == LF)
        lineCount++;

      if (look == '\'')
        skipComment();
      else
        getChar();
  }
}

//skip over a comma
void skipComma() {
  if (look == ',') {
    getChar();
    skipWhite();
  }
}

//find the parameter number
int paramNumber(string n) {
  return params[n];
}

//see if an identifier is a parameter
bool isParam(string n) {
  return params.find(n) != params.end();
}

//recognize a legal variable type
bool isVarType(string v) {
  if (!inTable(v))
    abort("Identifier \""+v+"\" not declared");
  switch(symbolTable[v]) {
  case TYPE_CHAR:
  case TYPE_FLOAT:
  case TYPE_INT:
  case TYPE_LONG:
  case TYPE_STRING:
    return true;
  default:
    return false;
  }
}

//output a string with tab
void emit(string s) {
  s = TAB + s;
  //printf(s.c_str());
  outputFile->write(s.c_str(),s.length());
}



//output a string with tab and crlf
void emitLn(string s) {
  emit(s+"\n");
}

//load a parameter to the primary register
void loadParam(int n) {
  int offset = 16 + 8 * (base - n);
  stringstream ss;
  ss << "mov rax, [rbp";
  if (offset >-1) {
    ss << "+";
  }
  ss << offset << "]";
  emitLn(ss.str());
}

//store a parameter from the primary register
void storeParam(int n) {
  int offset = 16+8*(base-n);
  stringstream ss;
  ss << "mov [rbp";
  if (offset>-1) {
    ss << "+";
  }
  ss << offset << "], rax";
  emitLn(ss.str());
}

// match a specific input character
void match (char x) {
//  newLine();
  if (look == x) {
    getChar();
  } else {
    stringstream ss;
    string temp;
    ss << x;
    ss >> temp;
    expected(temp);
  }
  skipWhite();
}

//generate a unique label
string newLabel() {
  stringstream ss;
  ss << "L" << lCount;
  lCount++;
  return ss.str();
}

//post a label to output
void postLabel(string l) {
  l+=":\n";
  //cout << l;
  outputFile->write(l.c_str(),l.length());
}


// recognize an alpha character
bool isAlpha(char c) {
  if (c >= 'a' && c <= 'z' )
    return true;
  if (c >= 'A' && c <= 'Z')
    return true;
  return false;
}

// recognize a decimal
bool isDigit(char c) {
  if (c >= '0' && c <='9')
    return true;
  return false;
}

//recognize an alphanumeric
bool isAlNum(char c) {
  return (isAlpha(c) || isDigit(c));
}

// get an identifier
void getName() {
  skipWhite();
  if (!isAlpha(look)) {
    expected("Identifier");
  }
  token = SYM_IDENT;
  value = "";
  while (isAlNum(look)) {
    value+=tolower(look);
    getChar();
  }
}

//get a number
void getNum() {
    skipWhite();
  if (!isDigit(look)) {
    expected("Integer");
  }
  token = SYM_DIGIT;
  value = "";
  while (isDigit(look)) {
    value+= look;
    getChar();
  }
}

int tableLookup(string table[], string s, int n) {
  bool found = false;
  int i = n;

  while (i > 0 && !found) {
    if (table[i-1].compare(s) == 0) {
      found = true;
    } else {
      i--;
    }
  }
  if (s == "$") {
    return TYPE_STRING;
  } else if (s == "#") {
    return TYPE_FLOAT;
  }
  return i;
}

//get an operator
void getOp() {
  skipWhite();
  value = look;
  token = tableLookup(operatorList,value, OPERATOR_COUNT) + OPERATOR_OFFSET;
  getChar();
}

//get the next input token
void next() {
  skipWhite();
  if (isAlpha(look)) {
    getName();
  } else if (isDigit(look)) {
    getNum();
  } else {
    getOp();
  }
}

void scan() {
  if (token == SYM_IDENT) {
    token = tableLookup(keywordList, value, KEYWORD_COUNT);
  }
}

void matchString(string x) {
  if (value.compare(x) != 0) {
    expected("\""+x+"\"");
  }
  next();
}

// init
void init(string input) {
  clearParams();
  lCount = 0;
  lineCount = 1;
  inputFile = new ifstream(input);

  if (!inputFile->good()) {
    abort("failed to open file \""+input+"\"\n \
          does the file exist?\n");
  }

  outputFile = new ofstream(sourceFileBaseName+".asm");
  getChar();
  next();
}


//check if character is an addOp
bool isAddOp(int i) {
  switch(i) {
  case OP_ADD:
  case OP_SUB:
    return true;
    break;
  default:
    return false;
    break;
  }
}

//check if character is a multOp
bool isMultOp(int i) {
  switch(i) {
  case OP_MULT:
  case OP_DIV:
    return true;
    break;
  default:
    return false;
    break;
  }
}

//recognize a boolean Orop
bool isOrOp(int c) {
  switch(c) {
  case OP_OR:
  case OP_XOR:
    return true;
  default:
    return false;
  }
}

//recognize a relop
bool isRelOp(int c) {
  switch(c){
  case OP_REL_E:
  case OP_REL_L:
  case OP_REL_G:
    return true;
  default:
    return false;
  }
}

//look for symbol in table
bool inTable(string n) {
  return symbolTable.find(n) != symbolTable.end();
}

//check to see if identifier is in the symbol table
void checkTable(string n) {
  if (!inTable(n)) {
    undefined(n);
  }
}

//check for duplicate identifier
void checkDup(string n) {
  if (inTable(n))
    duplicate(n);
}

//add symbol to table
void addToTable(string n, int type) {
  checkDup(n);
  symbolTable[n] = type;
}

int getTypeFromTable(string n) {
  checkTable(n);
  return variables[n];
}
//load a variable
void loadVariable(string n) {
  if (!inTable(n))
    undefined(n);
  LoadVar(n);
}

//parse and translate a math expression
void factor() {
  if (token == OP_PAR_O) {
    next();
    boolExpression();
    matchString(")");
  } else {
    if (token == SYM_IDENT) {
      if (isParam(value)) {
        loadParam(paramNumber(value));
      } else {
        loadVariable(value);
      }
    } else if (token == SYM_DIGIT) {
        LoadConst(value);
    } else {
      expected("Math factor");
    }
    next();
  }
}

//recognize and translate a multiply
void multiply() {
  next();
  factor();
  PopMul();
}

//recognize and translate a divide
void divide() {
  next();
  factor();
  PopDiv();
}

//get another expression and compare
void compareExpresion() {
  expression();
  PopCompare();
}

//get the next expression and compare
void nextExpression() {
  next();
  compareExpresion();
}

//recognize and translate a relational equals
void equals() {
  nextExpression();
  setEqual();
}

//recognize and translate a not equals
void notEqual() {
  nextExpression();
  setNEqual();
}

//recognize and translate a relational less than or equal
void lessOrEqual() {
  nextExpression();
  setLessOrEqual();
}

//recognize and translate a less than
void Less() {
  next();
  switch(token) {
  case OP_REL_E:
    lessOrEqual();
    break;
  case OP_REL_G:
    notEqual();
    break;
  default:
    compareExpresion();
    setLess();
  }
}

//recognize and translate a greater than
void Greater() {
  next();
  if (token == OP_REL_E) {
    nextExpression();
    setGreaterOrEqual();
  } else {
    compareExpresion();
    setGreater();
  }
}

//parse and translate a relation
void relation() {
  expression();
  if (isRelOp(token)) {
    Push();
    switch(token) {
    case OP_REL_E:
      equals();
      break;
    case OP_REL_L:
      Less();
      break;
    case OP_REL_G:
      Greater();
      break;
    }
  }
}

//parse and translate a boolean factor with leading not
void notFactor() {
  if (token == OP_REL_N) {
    next();
    relation();
    NotIt();
  } else {
    relation();
  }
}

//parse and translate a boolean term
void boolTerm() {
  notFactor();
  while (look == OP_REL_A) {
    Push();
    next();
    notFactor();
    PopAnd();
  }
}

//recognize and translate a boolean or
void boolOr() {
  next();
  boolTerm();
  PopOr();
}

//recognize and translate a exclusive or
void boolXor() {
  next();
  boolTerm();
  PopXor();
}

//parse and translate a boolean expression
void boolExpression() {
  boolTerm();
  while (isOrOp(token)) {
    Push();
    switch(token) {
    case OP_OR:
      boolOr();
      break;
    case OP_XOR:
      boolXor();
      break;
    }
  }
}

//parse and translate a math term
void term() {
  factor();
  while (isMultOp(token)) {
    Push();
    switch(token) {
    case OP_MULT:
      multiply();
      break;
    case OP_DIV:
      divide();
      break;
    }
  }
}

// recognize and translate an add
void add() {
  next();
  term();
  PopAdd();
}

// recognize and translate a subtract
void subtract() {
  next();
  term();
  PopSub();
}

// parse and translate an expression
void expression() {
  if (isAddOp(token)) {
    Clear();
  } else {
    term();
  }
  while(isAddOp(token)) {
    Push();
    switch (token) {
    case OP_ADD:
      add();
      break;
    case OP_SUB:
      subtract();
      break;
    }
  }
}

//recognize and translate an if construct
void doIf() {
  next();
  boolExpression();
  string l1,l2;
  l1 = newLabel();
  l2 = l1;
  branchFalse(l1);
  block();
  if (token == SYM_ELSE) {
    next();
    l2 = newLabel();
    branch(l2);
    postLabel(l1);
    block();
  }
  postLabel(l2);
  matchString("endif");
}

//parse and translate a while statement
void doWhile() {
  next();
  string l1, l2;
  l1 = newLabel();
  l2 = newLabel();
  postLabel(l1);
  boolExpression();
  branchFalse(l2);
  block();
  matchString("wend");
  branch(l1);
  postLabel(l2);
}

//read a single variable
void readVar() {
  checkIdent();
  checkTable(value);
  readIt(value);
  next();
}

//process a read statement
void doRead() {
  next();
  matchString("(");
  readVar();
  while (token == OP_COMMA) {
    next();
    readVar();
  }
  matchString(")");
}

//process a write statement
void doWrite() {
  string name;
  next();
  matchString("(");
  name = value;
  expression();
  writeIt();
  while (token == OP_COMMA) {
    next();
    name = value;
    expression();
    writeIt();
  }
  matchString(")");
}


//parse and translate an assignment statement
void assignment() {
  if (!isParam(value)) {
    checkTable(value);
  }
  string name = value;
  next();
  matchString("=");
  boolExpression();
  if (isParam(name)) {
    storeParam(paramNumber(name));
  } else {
    StoreVar(name);
  }
}

//clear function params list
void clearParams() {
  params.clear();
  paramCount = 0;
}

//match a semicolon
void semi() {
  if (token == OP_SEMICOLON) matchString(";");
}

//add a new parameter to the table
void addParam(string n) {
  if (isParam(n))
    duplicate(n);
  paramCount++;
  params[n] = paramCount;
}


//process a formal parameter
void formalParam() {
  addParam(value);
  next();
}

//get type of symbol
int typeOf(string n) {
  if (isParam(n)) {
    return VAR_PARAM;
  } else {
    return symbolTable[n];
    //return getTypeFromTable(n);
  }
}

//process the formal parameter list of a function
void formalList() {
  matchString("(");
  if (token != OP_PAR_C) {
    //next();
    formalParam();
    while (token == OP_COMMA) {
      next();
      formalParam();
    }
  }
  matchString(")");
  base = paramCount;
  paramCount = paramCount +2;
}

//parse and translate a data declaration
void locDecl() {  // TODO make dim <var> = <val> work
  next();
  if (token != SYM_IDENT)
    expected("Variable Name");

  string name = value;
  string val = "";
  addToTable(name,TYPE_INT);
  next();
  if (token == OP_REL_E) {
    next();
    if (token == OP_SUB) {
      next();
      val+= "0-";
    }
    val+= value;
    next();
  } else {
   val= "0";
  }
  addParam(name);
}

//parse and translate local declarations
int locDecls() {
  int n = 0;
  scan();
  while (token == SYM_DIM) {
    locDecl();
    n++;
    while (token == OP_COMMA) {
      locDecl();
      n++;
    }
    semi();
    scan();
  }
  return n;
  //next();
}

//intro to a subroutine
void subProlog(string name, int locVarCount) {
  postLabel(name);
  emitLn("push rbp");
  emitLn("mov rbp, rsp");
  stringstream ss;
  ss << "sub rsp, " << (8*locVarCount);
  emitLn(ss.str());
}

//ending to a procedure
void subEpilog(int locVarCount) {
  stringstream ss;
  ss <<"add rsp, " << (8*locVarCount);
  emitLn(ss.str());
  emitLn("pop rbp");
  Return();
}

//parse and translate a subroutine
void doSub() {
  string l = newLabel();
  branch(l);
  next();
  string name = value;

  checkDup(name);
  addToTable(name,SYM_SUB);
  next();
  formalList();
  int locVarCount = locDecls();
  subProlog(name,locVarCount);
  block();
  subEpilog(locVarCount);
  postLabel(l);
  matchString("endsub");
  clearParams();
}

//process a parameter
void param() {
  expression();
  Push();
}

//process the parameter list for a subroutine call
int paramList() {
  int n = 0;
  matchString("(");
  if (token != OP_PAR_C) {
    param();
    n++;
    while (token == OP_COMMA) {
      next();
      param();
      n++;
    }
  }
  matchString(")");
  return 8*n;
}

//process a subroutine
void callSub(string name) {
  int n;
  next();
  n = paramList();
  call(name);
  cleanStack(n);
}

//decide if a statement is an assignment or a subroutine call
void assignmentOrSub() {
  int identifierType = typeOf(value);
  switch(identifierType) {
  case SYM_SUB:
    callSub(value);
    //next();
    break;
  case TYPE_INT:
  case TYPE_STRING:
  case TYPE_LONG:
  case TYPE_CHAR:
  case TYPE_FLOAT:
  case VAR_PARAM:
    assignment();
    break;
  default:
    abort("Identifier "+value+" Cannot be used here");
  }
}

bool isTerminator(int i) {
  if (inputFile->eof()) {
    return true;
  }
  switch(i) {
  case SYM_ENDIF:
  case SYM_ELSE:
  case SYM_WEND:
  case SYM_END_MAIN:
  case SYM_END_SUB:
    return true;
    break;
  default:
    return false;
  }
}

//parse and translate a block of statements
void block() {
  scan();
  while (!isTerminator(token)) {
    switch (token) {
    case SYM_IF:
      doIf();
      break;
    case SYM_WHILE:
      doWhile();
      break;
    case SYM_READ:
      doRead();
      break;
    case SYM_WRITE:
      doWrite();
      break;
    case SYM_SUB:
      doSub();
      break;
    case SYM_IDENT:
      assignmentOrSub();
      break;
    default:
      abort(value+" not expected");
    }
    semi();
    scan();
  }
}

int getVarType() {
  if (value == "$")
    return TYPE_STRING;
  if (value == "#")
    return TYPE_FLOAT;
  return TYPE_INT;
}

//allocate storage for a static variable
void allocate(string name, string value) {
  stringstream ss;
  ss << name << ":" << TAB << "DW " << value;
  emitLn(ss.str());
}


//allocate storage for a variable
void alloc() {
  next();
  if (token != SYM_IDENT)
    expected("Variable Name");

  string name = value;
  string val = "";
  next();
  int type = getVarType();

  if (type != TYPE_INT)
    next();

  addToTable(name,type);
  if (token == OP_REL_E) {
    next();
    if (token == OP_SUB) {
      next();
      val+= "0-";
    }
    val+= value;
    next();
  } else {
   val= "0";
  }
  allocate(name, val);
}

//parse and translate global declarations
void topDecls() {
  scan();
  while (token == SYM_DIM) {
    alloc();
    while (token == OP_COMMA) {
      alloc();
    }
    semi();
    scan();
  }
  //next();
}

//parse and translate a program
void prog() {
  //matchString("b4gl"); //handles program header part
  semi();
  header();
  topDecls();
  //matchString("main");
  semi();
  prolog();
  block();
  //matchString("endmain");
  //semi();
  epilog();
}

string exec(string cmd) {  // TODO use _pipe on windows
  FILE* pipe;
  if (CURRENT_OS == OS_LINUX) {
    pipe = popen(cmd.c_str(), "r");
  } else if (CURRENT_OS == OS_WINDOWS) {
    pipe = _popen(cmd.c_str(), "r");
  }
  if (!pipe) return "ERROR";
  char buffer[128];
  string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer,128, pipe) != NULL)
      result += buffer;
  }
  if (CURRENT_OS == OS_LINUX) {
    pclose(pipe);
  } else if (CURRENT_OS == OS_WINDOWS) {
    _pclose(pipe);
  }
  return result;
}

void closeFiles() {
  inputFile->close();
  outputFile->flush();
  outputFile->close();
  delete outputFile;
  delete inputFile;
}

void compile() {
  cout << "compiling" << endl;
  stringstream ss;
  if (CURRENT_OS == OS_LINUX) {
    ss << "nasm -felf64 -o " << sourceFileBaseName << ".o ";
  } else if (CURRENT_OS == OS_WINDOWS) {
    ss << "nasm -fwin64 -o " << sourceFileBaseName << ".obj ";
  }
  ss << sourceFileBaseName << ".asm";
  cout << exec(ss.str());
}

void link() {
  cout << "linking" << endl;
  stringstream ss;
  if (CURRENT_OS == OS_LINUX) {
    ss << "gcc " << sourceFileBaseName << ".o -o " << sourceFileBaseName;
  } else if (CURRENT_OS == OS_WINDOWS) {
    ss << "gcc " << sourceFileBaseName << ".obj -o " << sourceFileBaseName << ".exe";
  }
  cout << exec(ss.str());
}

void execute() {
  cout << "running" << endl << endl;
  stringstream ss;
  if (CURRENT_OS == OS_LINUX) {
    ss << "./";
  }
  ss << sourceFileBaseName;
  cout << exec(ss.str()) << endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "no parameters specified" << std::endl;
    return 1;
  }
  sourceFileName = argv[1];
  sourceFileBaseName = sourceFileName.substr(0,sourceFileName.find_last_of('.'));
  init(argv[1]);
  prog();       //parse program into 8086
  closeFiles(); // close input and output files
  compile();    // invoke assembler
  link();       // invoke the linker
  execute();    // execute the compile program

  dumpSymbolTable();
  return 0;
}

