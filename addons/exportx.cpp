#include "exportx.h"

#include <fstream>

#include "../transformtools.h"

//.x support is very very incomplete (because I can't find a good file format
//spec)

void writeMesh(std::ostream& out, const BModel& bmd, const Batch& barch, int indent)
{
  return;

  std::string inStr(indent, ' ');
  std::string inStr2 = inStr + "  ";

  out << inStr << "Mesh\n" << inStr << "{\n";

  out << inStr2 << "3;\n"
      << inStr2 << "0;0;0;,\n"
      << inStr2 << "1;0;0;,\n"
      << inStr2 << "1;1;0;;\n"
      << inStr2 << "1;\n"
      << inStr2 << "3;0,1,2;;\n";

  out << inStr << "}\n";
}

void writeFrames(std::ostream& out, const BModel& bmd, const SceneGraph& sg, int indent = 0)
{
  bool doWriteFrame = sg.type == 0x10;

  std::string inStr(indent, ' ');
  std::string inStr2 = inStr + "  ";
  std::string inStr3 = inStr2 + "  ";

  //start frame
  if(doWriteFrame)
  {
    out << inStr << "Frame " << bmd.jnt1.frames[sg.index].name << "\n"
        << inStr << "{\n";

    //write frame transform matrix
    /*
    Matrix44f matrix = frameMatrix(bmd.jnt1.frames[sg.index]);
    out << inStr2 << "FrameTransformMatrix\n" << inStr2 << "{\n";
    for(int i = 0; i < 4; ++i)
    {
      out << inStr3;
      for(int j = 0; j < 4; ++j)
        out << matrix[j][i] << ((j == 3)?(i == 3)?";;":",":", "); //everyone loves readable code ;-)
      out << "\n";
    }
    out << inStr2 << "}\n\n";
    //*/
  }

  //write children
  for(int i = 0; i < sg.children.size(); ++i)
    writeFrames(out, bmd, sg.children[i], indent + (doWriteFrame?2:0));

  if(sg.type == 0x12)
  {
    //out << "\n";
    writeMesh(out, bmd, bmd.shp1.batches[sg.index], indent + 2);
  }

  //end frame
  if(doWriteFrame)
  {
    out << inStr << "}\n";
  }
}

void exportAsX(const BModel& bmd, const std::string& filename)
{
  //build scenegraph
  SceneGraph sg;
  buildSceneGraph(bmd.inf1, sg);

  //try to open output file
  std::ofstream out(filename.c_str());

  if(!out)
  {
    warn("exportAsX(): Failed to open output file %s", filename.c_str());
    return;
  }

  //write file header
  out << "xof 0303txt 0032\n\n";

  //write frames
  writeFrames(out, bmd, sg);
}
