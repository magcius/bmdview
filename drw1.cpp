#include "drw1.h"

#include <iostream>

using namespace std;

namespace bmd
{

// DRW1 /////////////////////////////////////////////////////////////

struct Drw1Header
{
  char tag[4];
  u32 sizeOfSection;
  u16 count;
  u16 pad;

  //stores for each matrix if it's weighted (normal (0)/skinned (1) matrix types)
  u32 offsetToIsWeighted;

  //for normal (0) matrices, this is an index into the global matrix
  //table (which stores a matrix for every joint). for skinned
  //matrices (1), I'm not yet totally sure how this works (but it's
  //probably an offset into the Evp1-array)
  u32 offsetToData;
};

};

void readDrw1Header(FILE* f, bmd::Drw1Header& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  readWORD(f, h.count);
  readWORD(f, h.pad);
  readDWORD(f, h.offsetToIsWeighted);
  readDWORD(f, h.offsetToData);  
}

void dumpDrw1(FILE* f, Drw1& dst)
{
  int drw1Offset = ftell(f), i;

  //read header
  bmd::Drw1Header h;
  readDrw1Header(f, h);

  //read bool array
  dst.isWeighted.resize(h.count);
  fseek(f, drw1Offset + h.offsetToIsWeighted, SEEK_SET);
  for(i = 0; i < h.count; ++i)
  {
    u8 v; fread(&v, 1, 1, f);
    if(v == 0)
      dst.isWeighted[i] = false;
    else if(v == 1)
      dst.isWeighted[i] = true;
    else
      warn("drw1: unexpected value in isWeighted array: %d", v);
  }

  //read data array
  dst.data.resize(h.count);
  fseek(f, drw1Offset + h.offsetToData, SEEK_SET);
  for(i = 0; i < h.count; ++i)
    readWORD(f, dst.data[i]);
}

void writeDrw1Info(FILE* f, ostream& out)
{
  out << string(50, '/') << endl
      << "//Drw1 section" << endl
      << string(50, '/') << endl << endl;

  int drw1Offset = ftell(f), i;

  bmd::Drw1Header h;
  readDrw1Header(f, h);

  out << h.count << " many" << endl << endl;
  
  out << "isWeighted:" << endl;
  fseek(f, drw1Offset + h.offsetToIsWeighted, SEEK_SET);
  for(i = 0; i < h.count; ++i)
  {
    u8 v; fread(&v, 1, 1, f);
    out << " " << (int)v;
  }
  out << endl;
  
  out << "Data:" << endl;
  fseek(f, drw1Offset + h.offsetToData, SEEK_SET);
  for(i = 0; i < h.count; ++i)
  {
    u16 v; readWORD(f, v);
    out << " " << v;
  }
  out << endl << endl;
  
}
