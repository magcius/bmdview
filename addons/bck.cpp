#include "bck.h"
#include <string.h> // strncmp

using namespace std;

namespace bck
{

struct Ank1Header
{
  char tag[4]; //'ANK1'
  u32 sizeOfSection;

  /*
    0 - play once
    2 - loop
  */
  u8 loopFlags;

  u8 angleMultiplier; //all angles have to multiplied by pow(2, angleMultiplyer)

  u16 animationLength; //in time units

  u16 numJoints; //that many animated joints at offsetToJoints
  u16 scaleCount; //that many floats at offsetToScales
  u16 rotCount;   //that many s16s at offsetToRots
  u16 transCount; //that many floats at offsetToTrans

  u32 offsetToJoints;
  u32 offsetToScales;
  u32 offsetToRots;
  u32 offsetToTrans;
};


//TODO: the following two structs have really silly names, rename them
struct AnimIndex
{
  u16 count;
  u16 index;
  u16 zero; //always 0?? -> no (biawatermill01.bck) TODO: find out what it means
};

struct AnimComponent
{
  AnimIndex s; //scale
  AnimIndex r; //rotation
  AnimIndex t; //translation
};

struct AnimatedJoint
{
  /*
  if count > 1, count*3 floats/shorts stored at index
  (time, value, unk [interpolation info, e.g. tangent??])?

  for shorts, time is a "real" short, no fixedpoint
  */
  AnimComponent x;
  AnimComponent y;
  AnimComponent z;
};

};

void readAnk1Header(FILE* f, bck::Ank1Header& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  fread(&h.loopFlags, 1, 1, f);
  fread(&h.angleMultiplier, 1, 1, f);
  readWORD(f, h.animationLength);
  readWORD(f, h.numJoints);
  readWORD(f, h.scaleCount);
  readWORD(f, h.rotCount);
  readWORD(f, h.transCount);
  readDWORD(f, h.offsetToJoints);
  readDWORD(f, h.offsetToScales);
  readDWORD(f, h.offsetToRots);
  readDWORD(f, h.offsetToTrans);
}

void readAnimIndex(FILE* f, bck::AnimIndex& h)
{
  readWORD(f, h.count);
  readWORD(f, h.index);
  readWORD(f, h.zero);
}

void readAnimComponent(FILE* f, bck::AnimComponent& h)
{
  readAnimIndex(f, h.s);
  readAnimIndex(f, h.r);
  readAnimIndex(f, h.t);
}

void readAnimatedJoint(FILE* f, bck::AnimatedJoint& h)
{
  readAnimComponent(f, h.x);
  readAnimComponent(f, h.y);
  readAnimComponent(f, h.z);
}

template<class T>
void readComp(vector<Key>& dst, const vector<T>& src, const bck::AnimIndex& index)
{
  dst.resize(index.count);

  //violated by biawatermill01.bck
  if(index.zero != 0)
    warn("bck: zero field %d instead of zero", index.zero);
  //TODO: biawatermill01.bck doesn't work, so the "zero"
  //value is obviously something important

  if(index.count <= 0)
  {
    warn("bck1: readComp(): count is <= 0");
    return;
  }
  else if(index.count == 1)
  {
    dst[0].time = 0;
    dst[0].value = src[index.index];
    dst[0].tangent = 0;
  }
  else
  {
    for(int j = 0; j < index.count; ++j)
    {
      dst[j].time = src[index.index + 3*j];
      dst[j].value = src[index.index + 3*j + 1];
      dst[j].tangent = src[index.index + 3*j + 2];
    }
  }
}

void convRotation(vector<Key>& rots, float scale)
{
  for(size_t j = 0; j < rots.size(); ++j)
  {
    rots[j].value *= scale;
    rots[j].tangent *= scale;
  }
}

void dumpAnk1(FILE* f, Bck& bck)
{
  int i;
  long ank1Offset = ftell(f);

  //read header
  bck::Ank1Header h;
  readAnk1Header(f, h);

  bck.animationLength = h.animationLength;

  //read scale floats:
  fseek(f, ank1Offset + h.offsetToScales, SEEK_SET);
  vector<f32> scales(h.scaleCount);
  fread(&scales[0], 4, h.scaleCount, f);
  for(i = 0; i < h.scaleCount; ++i)
    toFLOAT(scales[i]);

  //read rotation s16s:
  fseek(f, ank1Offset + h.offsetToRots, SEEK_SET);
  vector<s16> rotations(h.rotCount);
  fread(&rotations[0], 2, h.rotCount, f);
  for(i = 0; i < h.rotCount; ++i)
    toSHORT(rotations[i]);

  //read translation floats:
  fseek(f, ank1Offset + h.offsetToTrans, SEEK_SET);
  vector<f32> translations(h.transCount);
  fread(&translations[0], 4, h.transCount, f);
  for(i = 0; i < h.transCount; ++i)
    toFLOAT(translations[i]);

  //read joints
  float rotScale = pow(2.f, h.angleMultiplier)*180/32768.f;
  fseek(f, ank1Offset + h.offsetToJoints, SEEK_SET);
  bck.anims.resize(h.numJoints);
  for(i = 0; i < h.numJoints; ++i)
  {
    bck::AnimatedJoint joint;
    readAnimatedJoint(f, joint);

    readComp(bck.anims[i].scalesX, scales, joint.x.s);
    readComp(bck.anims[i].scalesY, scales, joint.y.s);
    readComp(bck.anims[i].scalesZ, scales, joint.z.s);

    readComp(bck.anims[i].rotationsX, rotations, joint.x.r);
    convRotation(bck.anims[i].rotationsX, rotScale);
    readComp(bck.anims[i].rotationsY, rotations, joint.y.r);
    convRotation(bck.anims[i].rotationsY, rotScale);
    readComp(bck.anims[i].rotationsZ, rotations, joint.z.r);
    convRotation(bck.anims[i].rotationsZ, rotScale);

    readComp(bck.anims[i].translationsX, translations, joint.x.t);
    readComp(bck.anims[i].translationsY, translations, joint.y.t);
    readComp(bck.anims[i].translationsZ, translations, joint.z.t);
  }

  //TODO: some zelda files store additional data after
  //the ANK1 block, for example 24_cl_cut07_dash_o.bck
}

Bck* readBck(FILE* f)
{
  Bck* ret = new Bck;

  //skip file header
  fseek(f, 0x20, SEEK_SET);

  u32 size = 0;
  char tag[4];
  int t;

  do
  {
    fseek(f, size, SEEK_CUR);
    t = ftell(f);

    fread(tag, 1, 4, f);
    fread(&size, 4, 1, f);
    toDWORD(size);
    if(size < 8) size = 8; //prevent endless loop on corrupt data

    if(feof(f))
      break;
    fseek(f, t, SEEK_SET);

    if(strncmp(tag, "ANK1", 4) == 0)
      dumpAnk1(f, *ret);
    else
      warn("readBck(): Unsupported section \'%c%c%c%c\'",
        tag[0], tag[1], tag[2], tag[3]);

    fseek(f, t, SEEK_SET);
  } while(!feof(f));

  return ret;
}

//////////////////////////////////////////////////////////////////////


template<class T>
T interpolate(T v1, T d1, T v2, T d2, T t) //t in [0, 1]
{
  //linear interpolation
  //return v1 + t*(v2 - v1);

  //cubic interpolation
  float a = 2*(v1 - v2) + d1 + d2;
  float b = -3*v1 + 3*v2 - 2*d1 - d2;
  float c = d1;
  float d = v1;
  //TODO: yoshi_walk.bck has strange-looking legs...not sure if
  //the following line is to blame, though
  return ((a*t + b)*t + c)*t + d;
}

float getAnimValue(const vector<Key>& keys, float t)
{
  if(keys.size() == 0)
    return 0.f;

  if(keys.size() == 1)
    return keys[0].value;

  int i = 1;
  while(keys[i].time < t)
    ++i;

  float time = (t - keys[i - 1].time)/(keys[i].time - keys[i - 1].time); //scale to [0, 1]
  return interpolate(keys[i - 1].value, keys[i - 1].tangent, keys[i].value, keys[i].tangent, time);
}

//the caller has to ensure that jnt1.frames and bck.anims contain
//the same number of elements
void animate(Bck& bck, Jnt1& jnt1, float ftime)
{
  ftime = fmod(ftime, bck.animationLength);

  //update joints
  for(size_t i = 0; i < jnt1.frames.size(); ++i)
  {
    jnt1.frames[i].sx = getAnimValue(bck.anims[i].scalesX, ftime);
    jnt1.frames[i].sy = getAnimValue(bck.anims[i].scalesY, ftime);
    jnt1.frames[i].sz = getAnimValue(bck.anims[i].scalesZ, ftime);

    //TODO: use quaternion interpolation for rotations?
    jnt1.frames[i].rx = getAnimValue(bck.anims[i].rotationsX, ftime);
    jnt1.frames[i].ry = getAnimValue(bck.anims[i].rotationsY, ftime);
    jnt1.frames[i].rz = getAnimValue(bck.anims[i].rotationsZ, ftime);

    jnt1.frames[i].t.x() = getAnimValue(bck.anims[i].translationsX, ftime);
    jnt1.frames[i].t.y() = getAnimValue(bck.anims[i].translationsY, ftime);
    jnt1.frames[i].t.z() = getAnimValue(bck.anims[i].translationsZ, ftime);
  }
}
