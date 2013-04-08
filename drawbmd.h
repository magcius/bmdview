#ifndef BMD_DRAWBMD_H
#define BMD_DRAWBMD_H BMD_DRAWBMD_H

struct BModel;
struct OglBlock;
struct Bck;
struct Btp;

#include <vector>
#include "inf1.h"

struct Model
{
  Model(BModel* bmdP = NULL, OglBlock* b = NULL)
    : bmd(bmdP), oglBlock(b), bck(NULL), btp(NULL)
  { }

  BModel* bmd;
  OglBlock* oglBlock;
  Bck* bck;
  Btp* btp;
  SceneGraph sceneGraph;
  std::string bmdFileName;
};

void drawBmd(Model& bmd, const SceneGraph& sg);


#endif //BMD_DRAWBMD_H
