#include <iostream>
#include <cstdarg>
using namespace std;

#include "bmdread.h"
#include "openfile.h"

#include "addons/exportTexture.h"
#include "addons/export3ds.h"

//has to be implemented (TODO...)
void setStartupText(const std::string& s)
{
  cout << s << endl;
}

//dito (TODO...)
void warn(const char* msg, ...)
{
  va_list argList;
  va_start(argList, msg);
  char buff[161];
  vsnprintf(buff, 161, msg, argList);
  va_end(argList);
  cout << buff << endl;
}

int main(int argc, char* argv[])
{
  if(argc < 2)
    return -1;

  OpenedFile* f = openFile(argv[1]);
  if(f == 0)
    return -2;

#if 1
  writeBmdInfo(f->f, cout);
#elif 0
  BModel* mod = loadBmd(f->f);
  exportTextures(TGA, mod->tex1, string("testdata/tex") + strchr(argv[1], '/'));
  delete mod;
#elif 1
  BModel* mod = loadBmd(f->f);
  exportAs3ds(*mod, string("testdata/3ds")
              + strchr(argv[1], '/') + string(".3ds"));
  delete mod;
#else
  BModel* mod = loadBmd(f->f);
  for(int i = 0; i < mod->tex1.images.size(); ++i)
  {
    Image& img = mod->tex1.images[i];
    cout << img.originalFormat << ' ' << img.paletteFormat << ' '
         << img.width << ' ' << img.height << endl;
  }
  delete mod;
#endif

  closeFile(f);
  
}
