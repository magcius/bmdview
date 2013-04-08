#include "parameters.h"

using namespace std;

static vector<string> g_parameters;


int getParameterCount()
{
  return g_parameters.size();
}

string getParameter(int num)
{
  return g_parameters[num];
}

void parseParameters(const char* params, vector<string>& dst)
{
  while(*params != '\0')
  {
    while(isspace(*params))
      ++params;

    if(*params == '\"')
    {
      ++params;
      const char* start = params;
      while(*params != '\"' && *params != '\0')
        ++params;
      dst.push_back(string(start, params - start));

      if(*params == '\0')
        break;
      ++params;
    }
    else
    {
      const char* start = params;
      while(!isspace(*params) && *params != '\0')
        ++params;
      dst.push_back(string(start, params - start));

      if(*params == '\0')
        break;
      ++params;
    }
  }
}

void parseParameters(const char* params)
{
  parseParameters(params, g_parameters);
}

void parseParameters(int argc, char* argv[], vector<string>& dst)
{
  // skip executable name
  for(int i = 1; i < argc; ++i)
    dst.push_back(argv[i]);
}

void parseParameters(int argc, char* argv[])
{
  parseParameters(argc, argv, g_parameters);
}
