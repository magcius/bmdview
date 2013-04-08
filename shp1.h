#ifndef BMD_SHP1_H
#define BMD_SHP1_H BMD_SHP1_H

#include <vector>
#include <iosfwd>
#include "common.h"
#include "Vector3.h"

struct Index
{
  u16 matrixIndex;
  u16 posIndex;
  u16 normalIndex;
  u16 colorIndex[2];
  u16 texCoordIndex[8];
};

struct Primitive
{
  u8 type;
  std::vector<Index> points;
};

enum
{
  GX_TRIANGLE_STRIP = 0x98,
  GX_TRIANGLE_FAN = 0xa0
};

struct Packet
{
  std::vector<Primitive> primitives;

  std::vector<u16> matrixTable; //maps attribute matrix index to draw array index
};

struct Attributes
{
  bool hasMatrixIndices, hasPositions, hasNormals, hasColors[2], hasTexCoords[8];
};

struct Batch
{
  Attributes attribs;
  std::vector<Packet> packets;

  Vector3f bbMin, bbMax; //experimental
  u8 matrixType; //experimental
};

struct Shp1
{
  std::vector<Batch> batches;

  //TODO: unknown data is missing, ...
};

void dumpShp1(FILE* f, Shp1& dst);
void writeShp1Info(FILE* f, std::ostream& out);

#endif //BMD_SHP1_H
