#ifndef BTP_H
#define BTP_H BTP_H

//btp files contain texture animations for bmd/bdl files

#include "../common.h"

#include "../mat3.h"

struct TextureAnimation
{
  std::string name;
  std::vector<int> indices;
  int indexToMat3Table;
};

struct Btp
{
  //std::vector<std::string> stringtable;
  std::vector<TextureAnimation> anims;
};

Btp* readBtp(FILE* f);

void animate(Btp& btp, Mat3& mat3, float time);

#endif //BTP_H
