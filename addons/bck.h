#ifndef BCK_H
#define BCK_H BCK_H

//bck files contain joint animations for bmd/bdl files

#include "../common.h"

#include "../jnt1.h"

struct Key
{
  float time;
  float value;
  float tangent; //??
};

struct JointAnim
{
  std::vector<Key> scalesX;
  std::vector<Key> scalesY;
  std::vector<Key> scalesZ;

  std::vector<Key> rotationsX;
  std::vector<Key> rotationsY;
  std::vector<Key> rotationsZ;

  std::vector<Key> translationsX;
  std::vector<Key> translationsY;
  std::vector<Key> translationsZ;
};

struct Bck
{
  std::vector<JointAnim> anims;
  int animationLength;
};

Bck* readBck(FILE* f);

//the caller has to ensure that jnt1.frames and bck.anims contain
//the same number of elements
void animate(Bck& bck, Jnt1& jnt1, float time);

#endif //BCK_H
