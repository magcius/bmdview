#ifndef BMD_TEX1_H
#define BMD_TEX1_H BMD_TEX1_H

#include "common.h"
#include <vector>
#include <string>
#include <iosfwd>

struct Image;
struct ImageHeader
{
  Image* data;
  std::string name;

  /*
    from gx.h:
    0: clamp to edge
    1: repeat
    2: mirror
  */
  u8 wrapS, wrapT;

  u8 minFilter, magFilter;

  //TODO: unknown fields
};

const int I8 = 1;
const int I8_A8 = 3;
const int RGBA8 = 6;
const int DXT1 = 14;

struct Image
{
  int format;
  int width, height;

  std::vector<u8*> mipmaps; //points into imageData
  std::vector<int> sizes; //image data size for each mipmap
  std::vector<u8> imageData;

  //NOTE: palettized images are converted
  //to non-palettized images during load time,
  //i4 is converted to i8, a4i4 and a8i8 is converted to i8a8.
  //r5g5b5a3 and r5g6b5 are converted to rgba8.
  //(that is, only formats 1 (i8), 3* (i8a8), 6 (rgba8)
  //and 14 (dxt1) are used after conversion)

  //TODO: gl image conversions (rgba -> abgr, ai -> ia
  //somewhere else?)

  //TODO: this is temporary and belongs somewhere else:
  unsigned int texId;
  
  int originalFormat, paletteFormat;
};

struct Tex1
{
  std::vector<ImageHeader> imageHeaders;

  //because several image headers might point to the
  //same image data, this data is stored
  //separately to save some memory
  //(this way only about 1/6 of the memory required
  //otherwise is used)
  std::vector<Image > images;
};

void uploadImagesToGl(Tex1& tex1);

void dumpTex1(FILE* f, Tex1& dst);
void writeTex1Info(FILE* f, std::ostream& out);

#endif //BMD_TEX1_H
