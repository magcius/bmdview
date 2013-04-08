#ifndef BMD_INF1_H
#define BMD_INF1_H BMD_INF1_H

#include "common.h"
#include <vector>
#include <iosfwd>

struct Node //same as Inf1Entry
{
  u16 type, index;
};

struct Inf1
{
  int numVertices; //no idea what's this good for ;-)

  std::vector<Node> scenegraph;
};

void dumpInf1(FILE* f, Inf1& dst);


//the following is only convenience stuff

struct SceneGraph
{
  int type, index;
  std::vector<SceneGraph> children;
};

int buildSceneGraph(/*in*/ const Inf1& inf1, /*out*/ SceneGraph& sg, int j = 0 /* used internally */);
void writeInf1Info(FILE* f, std::ostream& out);

#endif //BMD_INF1_H
