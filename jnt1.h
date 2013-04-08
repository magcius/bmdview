#ifndef BMD_JNT1_H
#define BMD_JNT1_H BMD_JNT1_H

#include "common.h"

#include "Vector3.h"
#include <vector>
#include <string>
#include <iosfwd>
#include "Matrix44.h"

struct Frame
{
  float sx, sy, sz; //scale
  float rx, ry, rz; //rotation (in degree)
  Vector3f t; //translation
  std::string name;

  //TODO:
  //bounding box, float unknown2

  //experimentally
  u16 unknown;
  u8 unknown3;
  Vector3f bbMin, bbMax;
};

struct Jnt1
{
  std::vector<Frame> frames;

  //the Frames have to be converted to matrices
  //to be usable by gl. isMatrixValid stores
  //if a matrix represents a frame of if the
  //frame has changed since the matrix was
  //built (in animations for example)
  std::vector<Matrix44f> matrices;
  std::vector<bool> isMatrixValid; //TODO: use this

  //TODO: unknown array
};

void dumpJnt1(FILE* f, Jnt1& dst);
void writeJnt1Info(FILE* f, std::ostream& out);

#endif //BMD_JNT1_H
