#ifndef NICO_OGLBLOCK_H
#define NICO_OGLBLOCK_H NICO_OGLBLOCK_H

#include "common.h"
#include "GL/glew.h"

#include <vector>
#include <string>

#include "bmdread.h"

bool hasShaderHardware();
void setTexWrapMode(u8 sMode, u8 tMode);

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

GLenum texFilter(u8 filter);
void setFilters(int magFilter, int minFilter, int mipCount);
void setMaterial(int index, const OglBlock& block, const BModel& bmd);


void loadShaderStrings(OglBlock& block, const std::string& baseName);
void saveShaderStrings(OglBlock& block, const std::string& baseName);
void generateShaderStrings(OglBlock& block, const Mat3& mat);

#endif //NICO_OGLBLOCK_H
