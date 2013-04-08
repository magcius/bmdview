#include "evp1.h"

#include <iostream>
using namespace std;

namespace bmd
{

struct Evp1Header
{
  char tag[4]; //'EVP1'
  u32 sizeOfSection;
  u16 count;
  u16 pad;

  //0 - count many bytes, each byte describes how many bones belong to this index
  //1 - sum over all bytes in 0 many shorts (index into some joint stuff? into matrix table?)
  //2 - bone weights table (as many floats as shorts in 1)
  //3 - matrix table (matrix is 3x4 float array)
  u32 offsets[4];
};

};

void readEvp1Header(FILE* f, bmd::Evp1Header& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  readWORD(f, h.count);
  readWORD(f, h.pad);
  for(int i = 0; i < 4; ++i)
    readDWORD(f, h.offsets[i]);
}

void readArray8(FILE* f, vector<int>& arr)
{
  for(size_t i = 0; i < arr.size(); ++i)
  {
    u8 v; fread(&v, 1, 1, f);
    arr[i] = v;
  }
}

void readMatrix(FILE* f, f32 m[3][4])
{
  fread(&m[0][0], 4, 3*4, f);
  for(int j = 0; j < 3; ++j)
    for(int k = 0; k < 4; ++k)
      toFLOAT(m[j][k]);  
}

void dumpEvp1(FILE* f, Evp1& dst)
{
  int evp1Offset = ftell(f), i;

  //read header
  bmd::Evp1Header h;
  readEvp1Header(f, h);

  //read counts array
  fseek(f, evp1Offset + h.offsets[0], SEEK_SET);
  vector<int> counts(h.count);
  readArray8(f, counts);

  //read indices of weighted matrices
  dst.weightedIndices.resize(h.count);
  fseek(f, evp1Offset + h.offsets[1], SEEK_SET);
  int numMatrices = 0;
  for(i = 0; i < h.count; ++i)
  {
    dst.weightedIndices[i].indices.resize(counts[i]);
    for(int j = 0; j < counts[i]; ++j)
    {
      u16 d; fread(&d, 2, 1, f); toWORD(d);
      dst.weightedIndices[i].indices[j] = d;
      numMatrices = max(numMatrices, d + 1);
     }
  }

  //read weights of weighted matrices
  fseek(f, evp1Offset + h.offsets[2], SEEK_SET);
  for(i = 0; i < h.count; ++i)
  {
    dst.weightedIndices[i].weights.resize(counts[i]);
    for(int j = 0; j < counts[i]; ++j)
    {
      float fl; fread(&fl, 4, 1, f); toFLOAT(fl);
      dst.weightedIndices[i].weights[j] = fl;
    }
  }

  //read matrices
  dst.matrices.resize(numMatrices);
  fseek(f, evp1Offset + h.offsets[3], SEEK_SET);
  for(i = 0; i < numMatrices; ++i)
  {
    f32 m[3][4];
    readMatrix(f, m);
    
    Matrix44f& dMat = dst.matrices[i];
    for(int j = 0; j < 3; ++j)
      for(int k = 0; k < 4; ++k)
        dMat[j][k] = m[j][k];
    dMat[3][0] = 0.f; dMat[3][1] = 0.f; dMat[3][2] = 0.f; dMat[3][3] = 1.f;
  }
}

void writeEvp1Info(FILE* f, ostream& out)
{
  out << string(50, '/') << endl
      << "//Evp1 section (incomplete)" << endl
      << string(50, '/') << endl << endl;
  
  bmd::Evp1Header h;
  readEvp1Header(f, h);
  
  out << h.count << " many" << endl << endl;
}
