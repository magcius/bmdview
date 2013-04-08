#include "vtx1.h"

#include <iostream>

using namespace std;

namespace bmd
{

struct Vtx1Header
{
  char tag[4]; //'VTX1'
  u32 sizeOfSection;
  u32 arrayFormatOffset; //for each offsets[i] != 0, an ArrayFormat
                         //is stored for that offset
                         //offset relative to Vtx1Header start

  /*
    content is described by ArrayFormat - for each offset != 0,
    an ArrayFormat struct is stored at Vtx1Header.arrayFormatOffset
  */
  u32 offsets[13]; //offsets relative to Vtx1Header start
};

struct ArrayFormat
{
  //see ogc/gx.h for a more complete list of these values:
  u32 arrayType; //9: coords, a: normal, b: color, d: tex0 (gx.h: "Attribute")
  u32 componentCount; //meaning depends on dataType (gx.h: "CompCount")
  u32 dataType; //3: s16, 4: float, 5: rgba8 (gx.h: "CompType")

  //values i've seem for this: 7, e, 8, b, 0
  //-> number of mantissa bits for fixed point numbers!
  //(position of decimal point)
  u8 decimalPoint;
  u8 unknown3; //seems to be always 0xff
  u16 unknown4; //seems to be always 0xffff
};

};


int getLength(const bmd::Vtx1Header& h, int k)
{
  int startOffset = h.offsets[k];
  for(int i = k + 1; i < 13; ++i)
  {
    if(h.offsets[i] != 0)
      return h.offsets[i] - startOffset;
  }
  return h.sizeOfSection - startOffset;
}

void readVertexArray(Vtx1& arrays, const bmd::ArrayFormat& af, int length,
                     FILE* f, long offset)
{
  fseek(f, offset, SEEK_SET);

  //convert array to float (so we have n + m cases, not n*m)
  vector<float> data;
  switch(af.dataType)
  {
    case 3: //s16 fixed point
    {
      vector<s16> tmp(length/2);
      fread(&tmp[0], 2, length/2, f);
      data.resize(length/2);
      float scale = pow(.5f, af.decimalPoint);
      for(size_t j = 0; j < data.size(); ++j)
      {
        toSHORT(tmp[j]);
        data[j] = tmp[j]*scale;
      }
    }break;

    case 4: //f32
    {
      data.resize(length/4);
      fread(&data[0], 4, length/4, f);
      for(size_t j = 0; j < data.size(); ++j)
        toFLOAT(data[j]);
    }break;

    case 5: //rgb(a)
    {
      vector<u8> tmp(length);
      fread(&tmp[0], 1, length, f);
      data.resize(length);
      for(size_t j = 0; j < data.size(); ++j)
        data[j] = tmp[j];
    }break;

    default:
      warn("vtx1: unknown array data type %d", af.dataType);
      return;
  }

  //stuff floats into appropriate vertex array
  switch(af.arrayType)
  {
    case 9: //positions
    {
      if(af.componentCount == 0) //xy
      {
        arrays.positions.resize(data.size()/2);
        for(size_t j = 0, k = 0; j < arrays.positions.size(); ++j, k += 2)
          arrays.positions[j].setXYZ(data[k], data[k + 1], 0);
      }
      else if(af.componentCount == 1) //xyz
      {
        arrays.positions.resize(data.size()/3);
        for(size_t j = 0, k = 0; j < arrays.positions.size(); ++j, k += 3)
          arrays.positions[j].setXYZ(data[k], data[k + 1], data[k + 2]);
      }
      else
        warn("vtx1: unsupported componentCount for positions array: %d",
          af.componentCount);
    }break;

    case 0xa: //normals
    {
      if(af.componentCount == 0) //xyz
      {
        arrays.normals.resize(data.size()/3);
        for(size_t j = 0, k = 0; j < arrays.normals.size(); ++j, k += 3)
          arrays.normals[j].setXYZ(data[k], data[k + 1], data[k + 2]);
      }
      else
        warn("vtx1: unsupported componentCount for normals array: %d",
          af.componentCount);
    }break;

    case 0xb: //color0
    case 0xc: //color1
    {
      int index = af.arrayType - 0xb;
      if(af.componentCount == 0) //rgb
      {
        arrays.colors[index].resize(data.size()/3);
        for(size_t j = 0, k = 0; j < arrays.colors[index].size(); ++j, k += 3)
          arrays.colors[index][j].setRGBA(data[k], data[k + 1], data[k + 2],
            255.f);
      }
      else if(af.componentCount == 1) //rgba
      {
        arrays.colors[index].resize(data.size()/4);
        for(size_t j = 0, k = 0; j < arrays.colors[index].size(); ++j, k += 4)
          arrays.colors[index][j].setRGBA(data[k], data[k + 1], data[k + 2],
            data[k + 3]);
      }
      else
        warn("vtx1: unsupported componentCount for colors array %d: %d",
          index, af.componentCount);

    }break;

    //texcoords 0 - 7
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    {
      int index = af.arrayType - 0xd;
      if(af.componentCount == 0) //s
      {
        arrays.texCoords[index].resize(data.size());
        for(size_t j = 0; j < arrays.texCoords[index].size(); ++j)
          arrays.texCoords[index][j].setST(data[j], 0);
      }
      else if(af.componentCount == 1) //st
      {
        arrays.texCoords[index].resize(data.size()/2);
        for(size_t j = 0, k = 0; j < arrays.texCoords[index].size(); ++j, k += 2)
          arrays.texCoords[index][j].setST(data[k], data[k + 1]);
      }
      else
        warn("vtx1: unsupported componentCount for texcoords array %d: %d",
          index, af.componentCount);
    }break;


    default:
      warn("vtx1: unknown array type %d", af.arrayType);
  }
}

void readVtx1Header(FILE* f, bmd::Vtx1Header& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  readDWORD(f, h.arrayFormatOffset);
  for(int i = 0; i < 13; ++i)
    readDWORD(f, h.offsets[i]);
}

void readArrayFormat(FILE* f, bmd::ArrayFormat& af)
{
  readDWORD(f, af.arrayType);
  readDWORD(f, af.componentCount);
  readDWORD(f, af.dataType);
  fread(&af.decimalPoint, 1, 1, f);
  fread(&af.unknown3, 1, 1, f);
  readWORD(f, af.unknown4);
}

int countArrays(const bmd::Vtx1Header& h)
{
  int numArrays = 0;
  for(int i = 0; i < 13; ++i)
    if(h.offsets[i] != 0) ++numArrays;
  return numArrays;
}

void dumpVtx1(FILE* f, Vtx1& dst)
{
  int vtx1Offset = ftell(f), i;

  //read header
  bmd::Vtx1Header h;
  readVtx1Header(f, h);

  int numArrays = countArrays(h);

  //read vertex array format descriptions
  vector<bmd::ArrayFormat> formats(numArrays);
  fseek(f, vtx1Offset + h.arrayFormatOffset, SEEK_SET);
  for(i = 0; i < numArrays; ++i)
    readArrayFormat(f, formats[i]);

  //read arrays
  int j = 0;
  for(i = 0; i < 13; ++i)
  {
    if(h.offsets[i] == 0)
      continue;

    bmd::ArrayFormat& currFormat = formats[j];
    int len = getLength(h, i);
    readVertexArray(dst, currFormat, len, f,
      vtx1Offset + h.offsets[i]);

    ++j;
  }
}


void writeVtx1Info(FILE* f, ostream& out)
{
  out << string(50, '/') << endl
      << "//Vtx1 section" << endl
      << string(50, '/') << endl << endl;

  int vtx1Offset = ftell(f);

  bmd::Vtx1Header h;
  readVtx1Header(f, h);

  int numArrays = countArrays(h);
  
  out << numArrays << " formats:" << endl;

  fseek(f, vtx1Offset + h.arrayFormatOffset, SEEK_SET);
  for(int i = 0; i < numArrays; ++i)
  {
    bmd::ArrayFormat af;
    readArrayFormat(f, af);
    
    out << af.arrayType << ", " << af.componentCount << ", " << af.dataType
        << " -- " << (int)af.decimalPoint;
    if(af.unknown3 != 0xff) out << " !" << (int)af.unknown3;
    if(af.unknown4 != 0xffff) out << " !!" << af.unknown3;
    out << endl;
  }
  out << endl;
}
