#include "common.h"

#include <iostream>

std::string getString(int pos, FILE* f)
{
  int t = ftell(f);
  fseek(f, pos, SEEK_SET);

  std::string ret;
  char c;
  while((c = fgetc(f)) != '\0')
    ret.append(1, c);

  fseek(f, t, SEEK_SET);

  return ret;
}

void readStringtable(int pos, FILE* f, std::vector<std::string>& dest)
{
  long oldPos = ftell(f);

  fseek(f, pos, SEEK_SET);

  u16 count; fread(&count, 2, 1, f); toWORD(count);
  fseek(f, 2, SEEK_CUR); //skip pad bytes

  for(int i = 0; i < count; ++i)
  {
    u16 unknown, stringOffset;
    fread(&unknown, 2, 1, f); toWORD(unknown);
    fread(&stringOffset, 2, 1, f); toWORD(stringOffset);
    std::string s = getString(pos + stringOffset, f);
    dest.push_back(s);
  }

  fseek(f, oldPos, SEEK_SET);
}

void writeStringtable(std::ostream& out, FILE* f, int offset)
{
  int p = ftell(f);

  fseek(f, offset, SEEK_SET);

  u16 count; fread(&count, 2, 1, f); toWORD(count);
  fseek(f, 2, SEEK_CUR); //skip pad bytes

  out << "String table (" << count << " entries)" << std::endl;
  for(int i = 0; i < count; ++i)
  {
    u16 unknown, stringOffset;
    fread(&unknown, 2, 1, f); toWORD(unknown);
    fread(&stringOffset, 2, 1, f); toWORD(stringOffset);
    std::string s = getString(offset + stringOffset,
                              f);
    out << "  0x" << std::hex << unknown << " - " << s << std::endl;
  }

  fseek(f, p, SEEK_SET);
}

void splitPath(const std::string& filename, std::string& folder,
               std::string& basename)
{
  std::string::size_type a = filename.rfind('\\');
  std::string::size_type b = filename.rfind('/');
  std::string::size_type c;
  if(a == std::string::npos)
    c = b;
  else if(b == std::string::npos)
    c = a;
  else
    c = std::min(a, b);
  if(c != std::string::npos)
  {
    folder = filename.substr(0, c + 1);
    basename = filename.substr(c + 1);
  }
  else
  {
    folder = "";
    basename = filename;
  }
}

void splitName(const std::string& basename, std::string& name,
               std::string& extension)
{
  std::string::size_type a = basename.rfind('.');
  if(a != std::string::npos)
  {
    name = basename.substr(0, a);
    extension = basename.substr(a + 1);
  }
  else
  {
    name = basename;
    extension = "";
  }
}

bool doesFileExist(const std::string& fileName)
{
  FILE* f = fopen(fileName.c_str(), "rb");
  bool ret = f != NULL;
  if(f != NULL)
    fclose(f);
  return ret;
}
