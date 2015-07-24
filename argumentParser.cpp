#include "argumentParser.h"

extern bool DEBUG_FLAG;
void abort(std::string);
extern int CURRENT_OS;
extern int OS_WINDOWS;
extern int OS_LINUX;
extern std::string sourceFileName;
extern std::string sourceFileBaseName;

bool isFlag(char *arg) {
  return arg[0] == '-';
}

void fixPath(std::string *path) {
  if (CURRENT_OS == OS_WINDOWS) {
    int look = path->find("/");
    while(look != std::string::npos) {
      path->replace(look,1,"\\");
      look = path->find("/");
    }
  }
}

void parseArgs(int argCount, char* args[]) {
  bool hasSourceFile = false;
  if (argCount < 2) {
    abort("no parameters specified");
  }
  for (int i = 1; i < argCount; i++) {
      if (isFlag(args[i])) {
        switch(args[i][1]) {
        case 'd':
          DEBUG_FLAG = true;
          break;
        default:
          std::stringstream ss;
          ss << "unrecognized parameter: \"" << args[i] << "\"";
          abort(ss.str());
        }
      } else {
        sourceFileName = args[i];
        fixPath(&sourceFileName);
        sourceFileBaseName = sourceFileName.substr(0,sourceFileName.find_last_of('.'));
      }
  }
}
