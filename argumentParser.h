#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>
#include <sstream>

//parse command line arguments
void parseArgs(int argCount, char* args[]);

//fix os specific paths
void fixPath(std::string *);

#endif // ARGUMENT_PARSER_H
