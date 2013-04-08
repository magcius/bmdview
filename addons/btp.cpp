#include "btp.h"
#include <string.h> //strncmp

#include <algorithm> //copy

using namespace std;

namespace btp
{

struct TptHeader
{
  char tag[4]; //'TPT1'
  u32 sizeOfSection;
  u8 unk; //loop type????
  u8 pad;
  u16 unk2; //(shortsPerMaterialAnim - no...sometimes 1 less (?))
  u16 numMaterialAnims;
  u16 numShorts; //(should be product of previous shorts)

  u32 offsetToMatAnims; //- for each materialAnim: u16 numSorts, u16 firstShort, u32 unk
  u32 offsetToShorts; // (shorts are texture indices)
  u32 offsetToIndexTable; //stores for every material to which mat3 index it belongs
  u32 offsetToStringTable;
};

struct MatAnim
{
  u16 count;
  u16 firstIndex;
  u32 unknown;
};

};

void readTptHeader(FILE* f, btp::TptHeader& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  fread(&h.unk, 1, 1, f);
  fread(&h.pad, 1, 1, f);
  readWORD(f, h.unk2);
  readWORD(f, h.numMaterialAnims);
  readWORD(f, h.numShorts);
  readDWORD(f, h.offsetToMatAnims);
  readDWORD(f, h.offsetToShorts);
  readDWORD(f, h.offsetToIndexTable);
  readDWORD(f, h.offsetToStringTable);
}

void readMatAnim(FILE* f, btp::MatAnim& anim)
{
  readWORD(f, anim.count);
  readWORD(f, anim.firstIndex);
  readDWORD(f, anim.unknown);
}

void dumpTpt1(FILE* f, Btp& btp)
{
  long tpt1Offset = ftell(f);
  size_t i;

  //read header
  btp::TptHeader h;
  readTptHeader(f, h);

  //read stringtable
  vector<string> stringtable;
  readStringtable(tpt1Offset + h.offsetToStringTable, f, stringtable);
  if(stringtable.size() != h.numMaterialAnims)
    warn("dumpTpt1: number of strings (%d) doesn't match number of "
      "animated materials (%d)", stringtable.size(), h.numMaterialAnims);

  //read matAnimIndexToMat3Index table
  vector<u16> matAnimIndexToMat3Index(h.numMaterialAnims);
  fseek(f, tpt1Offset + h.offsetToIndexTable, SEEK_SET);
  for(i = 0; i < h.numMaterialAnims; ++i)
    readWORD(f, matAnimIndexToMat3Index[i]);

  //read shorts table
  vector<u16> shorts(h.numShorts);
  fseek(f, tpt1Offset + h.offsetToShorts, SEEK_SET);
  for(i = 0; i < h.numShorts; ++i)
    readWORD(f, shorts[i]);

  //read animations
  btp.anims.resize(h.numMaterialAnims);
  fseek(f, tpt1Offset + h.offsetToMatAnims, SEEK_SET);
  for(i = 0; i < h.numMaterialAnims; ++i)
  {
    btp::MatAnim anim;
    readMatAnim(f, anim);

    if(anim.unknown != 0x00ffffff)
      warn("btp: %d instead of 0x00ffffff for mat anim nr %d", anim.unknown, i);

    if(i < stringtable.size())
      btp.anims[i].name = stringtable[i];
    btp.anims[i].indexToMat3Table = matAnimIndexToMat3Index[i];
    btp.anims[i].indices.resize(anim.count);
    copy(shorts.begin() + anim.firstIndex, shorts.begin() + anim.firstIndex + anim.count,
      btp.anims[i].indices.begin());
  }
}

Btp* readBtp(FILE* f)
{
  Btp* ret = new Btp;

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

    if(strncmp(tag, "TPT1", 4) == 0)
      dumpTpt1(f, *ret);
    else
      warn("readBtp(): Unsupported section \'%c%c%c%c\'",
        tag[0], tag[1], tag[2], tag[3]);

    fseek(f, t, SEEK_SET);
  } while(!feof(f));

  return ret;
}

//////////////////////////////////////////////////////////////////////

void animate(Btp& btp, Mat3& mat3, float ftime)
{
  unsigned int time = (unsigned int)(ftime + .5f);
  for(size_t i = 0; i < btp.anims.size(); ++i)
  {
    TextureAnimation& curr = btp.anims[i];

    unsigned int index = time % curr.indices.size();

    //mat3.materials[curr.indexToMat3Table].texStages[0] = curr.indices[index];
    //mat3.texStageIndexToTextureIndex[curr.indices[index]] = curr.indices[index];
    //mat3.texStageIndexToTextureIndex[curr.indexToMat3Table] = curr.indices[index];

    //works for link and mario, not for peach and yoshi (this line _is_ probably right,
    //but missing on more lookup)
    //mat3.texStageIndexToTextureIndex
    //  [mat3.materials[mat3.indexToMatIndex[curr.indexToMat3Table]].texStages[0]] = curr.indices[index];

    //this is hacky but works for all models - don't use index from file but
    //search an index which has the same name. TODO: do this correctly later,
    //it's good enough for now.
    size_t d;
    for(d = 0; d < mat3.indexToMatIndex.size(); ++d)
      if(mat3.stringtable[d] == curr.name)
        break;
    mat3.texStageIndexToTextureIndex
      [mat3.materials[mat3.indexToMatIndex[d]].texStages[0]] =
        curr.indices[index];

    //drawText("found index %d, real/expected index %d: %s", curr.indexToMat3Table, d, curr.name.c_str());
  }
}
