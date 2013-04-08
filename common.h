#ifndef BMD_COMMON_H
#define BMD_COMMON_H BMD_COMMON_H

#include "common/gccommon.h"
#include <string>
#include <vector>
#include <cstdio>
#include <iosfwd>

#ifdef _MSC_VER
namespace std
{
template<class T>
T min(T a, T b)
{
  return a < b?a:b;
}
}
#endif

const float PI = 3.14159265358979323f;

void log(const char* msg, ...);
void warn(const char* msg, ...);
std::string getString(int pos, FILE* f);
void readStringtable(int pos, FILE* f, std::vector<std::string>& dest);

//does more or less the same as readStringtable(), but writes
//the read data to an ostream instead of writing it into a vector
void writeStringtable(std::ostream& out, FILE* f, int offset);

inline int max(int a, int b)
{
  return a > b?a:b;
}

void setTextColor3f(float r, float g, float b);
void drawText(const char* s, ...);
void setStartupText(const std::string& text);

/**
  Splits a filename into path and basename components.
  "c:\bla/blubb\bar.ext" is split into "c:\bla/blubb\"
  and "bar.ext" for example
*/
void splitPath(const std::string& filename, std::string& folder,
               std::string& basename);

/**
  Splits a basename into name and extension.
  "bar.ext" is split into "bar" and "ext" for example.
*/
void splitName(const std::string& basename, std::string& name,
               std::string& extension);

/**
  Returns true if file with name fileName exists.
  Note that the file fileName could be deleted before you
  open it even if this function returns true.
*/
bool doesFileExist(const std::string& fileName);


#endif //BMD_COMMON_H
