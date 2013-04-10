#ifndef NICO_OGLBLOCK_H
#define NICO_OGLBLOCK_H NICO_OGLBLOCK_H

#include "common.h"
#include "GL/glew.h"

#include <vector>
#include <string>

#include "bmdread.h"

struct OglMaterial
{
  std::string vertexShaderString;
  std::string fragmentShaderString;

  bool needsColor[2];
  bool needsTexCoord[8];
  //etc


  GLhandleARB vertexShader;
  GLhandleARB fragmentShader;
  GLhandleARB glslProgram;
};

struct OglBlock
{
  std::vector<OglMaterial> materials;
};

OglBlock* createOglBlock(const Mat3& mat, const std::string& baseName = "");
void freeOglBlock(OglBlock*& oglBlock);
void setMaterial(int index, const OglBlock& block, const BModel& bmd);

#endif //NICO_OGLBLOCK_H
