#include "jnt1.h"

#include <iostream>
using namespace std;

namespace bmd
{

struct Jnt1Header
{
  char tag[4]; //'JNT1'
  u32 sizeOfSection;
  u16 count; //number of joints
  u16 pad; //padding u16 (?)

  u32 jntEntryOffset; //joints are stored at this place
                      //offset relative to Jnt1Header start

  u32 unknownOffset; //there are count u16's stored at this point,
                     //always the numbers 0 to count - 1 (in that order).
                     //perhaps an index-to-stringtable-index map?
                     //offset relative to Jnt1Header start
  u32 stringTableOffset; //names of joints
};


struct JntEntry
{
  u16 unknown; //no idea how this works...always 0, 1 or 2
               //"matrix type" according to yaz0r - whatever this means ;-)
               //if this is 1 or 2, unknown2 and bounding box is 0.f
               //seems to be always 0 if a joint has direct non-joint children
               //(it's even "iff" - only joints with non-joint children have 0?)

  u8 unknown3; //0 or 1 (0xff in one single file) (inherit parent scale???)

  u8 pad; //always 0xff

  f32 sx, sy, sz; //scale
  s16 rx, ry, rz; //-32768 = -180 deg, 32768 = 180 deg
  u16 pad2; //always 0xffff
  f32 tx, ty, tz; //translation

  f32 unknown2;
  f32 bbMin[3]; //bounding box (?)
  f32 bbMax[3]; //bounding box (?)
};

};

void readJnt1Header(FILE* f, bmd::Jnt1Header& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  readWORD(f, h.count);
  readWORD(f, h.pad);
  readDWORD(f, h.jntEntryOffset);
  readDWORD(f, h.unknownOffset);
  readDWORD(f, h.stringTableOffset);
}

void readJnt1Entry(FILE* f, bmd::JntEntry& e)
{
  readWORD(f, e.unknown);
  fread(&e.unknown3, 1, 1, f);
  fread(&e.pad, 1, 1, f);
  
  readFLOAT(f, e.sx); readFLOAT(f, e.sy); readFLOAT(f, e.sz);
  readSHORT(f, e.rx); readSHORT(f, e.ry); readSHORT(f, e.rz);
  readWORD(f, e.pad2);
  readFLOAT(f, e.tx); readFLOAT(f, e.ty); readFLOAT(f, e.tz);
  readFLOAT(f, e.unknown2);

  int j;
  for(j = 0; j < 3; ++j)
    readFLOAT(f, e.bbMin[j]);
  for(j = 0; j < 3; ++j)
    readFLOAT(f, e.bbMax[j]);
}

void dumpJnt1(FILE* f, Jnt1& dst)
{
  int jnt1Offset = ftell(f);

  //read header
  bmd::Jnt1Header h;
  readJnt1Header(f, h);

  //read stringtable
  vector<string> stringtable;
  readStringtable(jnt1Offset + h.stringTableOffset, f, stringtable);

  if(stringtable.size() != h.count)
    warn("jnt1: number of strings doesn't match number of joints");

  //read joints
  fseek(f, jnt1Offset + h.jntEntryOffset, SEEK_SET);

  dst.frames.resize(h.count);
  dst.matrices.resize(h.count);
  dst.isMatrixValid.resize(h.count);
  fill_n(dst.isMatrixValid.begin(), h.count, false);
  for(size_t i = 0; i < h.count; ++i)
  {
    bmd::JntEntry e;
    readJnt1Entry(f, e);

    Frame& f = dst.frames[i];
    f.sx = e.sx;
    f.sy = e.sy;
    f.sz = e.sz;
    f.rx = e.rx/32768.f*180;
    f.ry = e.ry/32768.f*180;
    f.rz = e.rz/32768.f*180;
    f.t.setXYZ(e.tx, e.ty, e.tz);
    if(i < stringtable.size()) //should always be true
      f.name = stringtable[i];

    f.unknown = e.unknown;
    f.unknown3 = e.unknown3;
    f.bbMin.setXYZ(e.bbMin[0], e.bbMin[1], e.bbMin[2]);
    f.bbMax.setXYZ(e.bbMax[0], e.bbMax[1], e.bbMax[2]);
  }
}

void writeJnt1Info(FILE* f, ostream& out)
{
  out << string(50, '/') << endl
      << "//Jnt1 section" << endl
      << string(50, '/') << endl << endl;

  int jnt1Offset = ftell(f), i;

  //read jnt1 header
  bmd::Jnt1Header h;
  readJnt1Header(f, h);

  out << hex << "pad: " << h.pad << endl << endl;

  //dump stringtable
  writeStringtable(out, f, jnt1Offset + h.stringTableOffset);

  //read stringtable
  std::vector<std::string> stringtable;
  readStringtable(jnt1Offset + h.stringTableOffset, f, stringtable);

  //joint entries
  out << endl << "Joint entries" << endl;
  fseek(f, jnt1Offset + h.jntEntryOffset, SEEK_SET);
  for(i = 0; i < h.count; ++i)
  {
    bmd::JntEntry e;
    readJnt1Entry(f, e);

    out << " \"" << stringtable[i] << "\":"
        << endl << "  "
        << " unknown " << e.unknown
        << " unk3 " << (int)e.unknown3
        << " pad " << (int)e.pad
        << endl << "  "
        << " sx " << e.sx
        << " sy " << e.sy
        << " sz " << e.sz
        << endl << "  "
        << " rx " << e.rx/32768.f*180
        << " ry " << e.ry/32768.f*180
        << " rz " << e.rz/32768.f*180
        << " pad2 " << e.pad2
        << endl << "  "
        << " tx " << e.tx
        << " ty " << e.ty
        << " tz " << e.tz
        << endl << "  "
        << " unknown2 " << e.unknown2
        << endl << "  "
        << " bbMin " << e.bbMin[0] << ", " << e.bbMin[1] << ", " << e.bbMin[2]
        << endl << "  "
        << " bbMax " << e.bbMax[0] << ", " << e.bbMax[1] << ", " << e.bbMax[2]
        << endl << endl;
  }

  out << endl;
}
