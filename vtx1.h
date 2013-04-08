#ifndef BMD_VTX1_H
#define BMD_VTX1_H BMD_VTX1_H

#include "common.h"
#include <vector>
#include <iosfwd>
#include "Vector3.h"

struct Color
{
  unsigned char r, g, b, a;

  void setRGBA(float ri, float gi, float bi, float ai)
  {
    r = (unsigned char)(ri + .5f);
    g = (unsigned char)(gi + .5f);
    b = (unsigned char)(bi + .5f);
    a = (unsigned char)(ai + .5f);
  }
};

struct TexCoord
{
  float s, t;

  void setST(float si, float ti)
  {
    s = si;
    t = ti;
  }
};

struct Vtx1
{
  std::vector<Vector3f> positions;
  std::vector<Vector3f> normals;
  std::vector<Color> colors[2];
  std::vector<TexCoord> texCoords[8];
};

void dumpVtx1(FILE* f, Vtx1& dst);
void writeVtx1Info(FILE* f, std::ostream& out);

#endif //BMD_VTX1_H
