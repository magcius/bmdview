#ifndef BMD_PARAMETERS_H
#define BMD_PARAMETERS_H BMD_PARAMETERS_H

#include <string>
#include <vector>

int getParameterCount();
std::string getParameter(int num);
void parseParameters(const char* params);
void parseParameters(int argc, char* argv[]);

#endif //BMD_PARAMETERS_H
