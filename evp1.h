#ifndef BMD_EVP1_H
#define BMD_EVP1_H BMD_EVP1_H

#include "common.h"

#include <iosfwd>

#include "Matrix44.h"

struct MultiMatrix
{
  std::vector<float> weights;
  std::vector<u16> indices; //indices into Evp1.matrices (?)
};

struct Evp1
{
  std::vector<MultiMatrix> weightedIndices;
  std::vector<Matrix44f> matrices;
};

void dumpEvp1(FILE* f, Evp1& dst);
void writeEvp1Info(FILE* f, std::ostream& out);

#endif //BMD_EVP1_H
