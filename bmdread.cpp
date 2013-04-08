#include "bmdread.h"
#include <string.h> // strncmp
#include <iostream>

using namespace std;


void readBmd(FILE* f, BModel* dst)
{
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

    setStartupText("Parsing " + std::string(tag, 4) + "...");

    if(strncmp(tag, "INF1", 4) == 0)
      dumpInf1(f, dst->inf1);
    else if(strncmp(tag, "VTX1", 4) == 0)
      dumpVtx1(f, dst->vtx1);
    else if(strncmp(tag, "EVP1", 4) == 0)
      dumpEvp1(f, dst->evp1);
    else if(strncmp(tag, "DRW1", 4) == 0)
      dumpDrw1(f, dst->drw1);
    else if(strncmp(tag, "JNT1", 4) == 0)
      dumpJnt1(f, dst->jnt1);
    else if(strncmp(tag, "SHP1", 4) == 0)
      dumpShp1(f, dst->shp1);
    //else if(strncmp(tag, "MAT3", 4) == 0)
	else if(strncmp(tag, "MAT", 3) == 0) //s_forest.bmd has a MAT2 section
      dumpMat3(f, dst->mat3);
    else if(strncmp(tag, "TEX1", 4) == 0)
      dumpTex1(f, dst->tex1);
    else
      warn("readBmd(): Unsupported section \'%c%c%c%c\'",
        tag[0], tag[1], tag[2], tag[3]);

    fseek(f, t, SEEK_SET);

  } while(!feof(f));
}

BModel* loadBmd(FILE* f)
{
  BModel* ret = new BModel;
  readBmd(f, ret);
  return ret;
}

void writeBmdInfo(FILE* f, std::ostream& out)
{
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

    if(strncmp(tag, "INF1", 4) == 0)
      writeInf1Info(f, out);
    else if(strncmp(tag, "VTX1", 4) == 0)
      writeVtx1Info(f, out);
    else if(strncmp(tag, "EVP1", 4) == 0)
      writeEvp1Info(f, out);
    else if(strncmp(tag, "DRW1", 4) == 0)
      writeDrw1Info(f, out);
    else if(strncmp(tag, "JNT1", 4) == 0)
      writeJnt1Info(f, out);
    else if(strncmp(tag, "SHP1", 4) == 0)
      writeShp1Info(f, out);
    //else if(strncmp(tag, "MAT3", 4) == 0)
	else if(strncmp(tag, "MAT", 3) == 0) //s_forest.bmd has a MAT2 section
      writeMat3Info(f, out);
    else if(strncmp(tag, "TEX1", 4) == 0)
      writeTex1Info(f, out);
    else if(strncmp(tag, "MDL3", 4) == 0)
      writeMdl3Info(f, out);

    fseek(f, t, SEEK_SET);
  } while(!feof(f));
}
