#include "inf1.h"

#include <iostream>
using namespace std;

namespace bmd
{

struct Inf1Header
{
  char tag[4]; //'INF1'
  u32 sizeOfSection;
  u16 unknown1;
  u16 pad; //0xffff
  u32 unknown2;
  u32 vertexCount; //number of coords in VTX1 section
  u32 offsetToEntries; //offset relative to Inf1Header start
};

//This stores the scene graph of the file
struct Inf1Entry
{
  //0x10: Joint
  //0x11: Material
  //0x12: Shape (ie. Batch)
  //0x01: Hierarchy down (insert node), new child
  //0x02: Hierarchy up, close child
  //0x00: Terminator
  u16 type;

  //Index into Joint, Material or Shape table
  //always zero for types 0, 1 and 2
  u16 index;
};

};

void readInf1Header(FILE* f, bmd::Inf1Header& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  readWORD(f, h.unknown1);
  readWORD(f, h.pad);
  readDWORD(f, h.unknown2);
  readDWORD(f, h.vertexCount);
  readDWORD(f, h.offsetToEntries);
}

void readInf1Entry(FILE* f, bmd::Inf1Entry& e)
{
  readWORD(f, e.type);
  readWORD(f, e.index);
}

void dumpInf1(FILE* f, Inf1& dst)
{
  long inf1Offset = ftell(f);

  //read header
  bmd::Inf1Header h;
  readInf1Header(f, h);

  dst.numVertices = h.vertexCount;

  //read scene graph
  fseek(f, inf1Offset + h.offsetToEntries, SEEK_SET);

  bmd::Inf1Entry e;
  readInf1Entry(f, e);

  while(e.type != 0)
  {
    Node node = { e.type, e.index };
    dst.scenegraph.push_back(node);

    readInf1Entry(f, e);
  }
}

//the following is only convenience stuff
int buildSceneGraph(const Inf1& inf1, SceneGraph& sg, int j)
{
  for(size_t i = j; i < inf1.scenegraph.size(); ++i)
  {
    const Node& n = inf1.scenegraph[i];

    if(n.type == 1)
      i += buildSceneGraph(inf1, sg.children.back(), i + 1);
    else if(n.type == 2)
      return i - j + 1;
    else if(n.type == 0x10 || n.type == 0x11 || n.type == 0x12)
    {
      SceneGraph t;
      t.type = n.type; t.index = n.index;
      sg.children.push_back(t);
    }
    else
      warn("buildSceneGraph(): unexpected node type %d", n.type);

  }

  //remove dummy node at root
  if(sg.children.size() == 1)
  {
    SceneGraph t = sg.children[0];
    sg = t; //doesn't work without the temporary (msvc bug?)
  }
  else
  {
    sg.type = sg.index = -1;
    warn("buildSceneGraph(): Unexpected size %d", sg.children.size());
  }

  return 0;
}

void writeInf1Info(FILE* f, ostream& out)
{
  out << string(50, '/') << endl
      << "//Inf1 section (TODO: dump decoded scenegraph?)" << endl
      << string(50, '/') << endl << endl;

  long inf1Offset = ftell(f);

  bmd::Inf1Header h;
  readInf1Header(f, h);


  out << h.vertexCount << " vertices in file." << endl 
      << "Scenegraph:" << endl;
      
      
  fseek(f, inf1Offset + h.offsetToEntries, SEEK_SET);

  bmd::Inf1Entry e;
  readInf1Entry(f, e);

  while(e.type != 0)
  {
    out << " (" << e.type << ", " << e.index << ")";
    readInf1Entry(f, e);
  }
  
  out << endl << endl;
}
