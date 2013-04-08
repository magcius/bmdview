#ifndef BMD_MAT3_H
#define BMD_MAT3_H BMD_MAT3_H

#include "common.h"

#include <iosfwd>

struct MColor
{
  u8 r, g, b, a;
};

struct Color16
{
  s16 r, g, b, a;
};

struct ColorChanInfo
{
  //not sure if this is right
  u8 enable;
  u8 matColorSource;
  u8 litMask;
  u8 diffuseAttenuationFunc;
  u8 attenuationFracFunc;
  u8 ambColorSource;
  u8 pad[2];
};

struct TexGenInfo
{
  u8 texGenType;
  u8 texGenSrc;
  u8 matrix;
};

struct TexMtxInfo
{
  float scaleCenterX, scaleCenterY;
  float scaleU, scaleV;
};

struct TevOrderInfo
{
  u8 texCoordId;
  u8 texMap;
  u8 chanId;
};

struct TevSwapModeInfo
{
  u8 rasSel;
  u8 texSel;
};

struct TevSwapModeTable
{
  u8 r;
  u8 g;
  u8 b;
  u8 a;
};

struct AlphaCompare
{
  u8 comp0, ref0;
  u8 alphaOp;
  u8 comp1, ref1;
};

struct BlendInfo
{
  u8 blendMode;
  u8 srcFactor, dstFactor;
  u8 logicOp;
};

struct ZMode
{
  bool enable;
  u8 zFunc;
  bool enableUpdate;
};


struct TevStageInfo
{
  //GX_SetTevColorIn() arguments
  u8 colorIn[4]; //GX_CC_*

  //GX_SetTevColorOp() arguments
  u8 colorOp;
  u8 colorBias;
  u8 colorScale;
  u8 colorClamp;
  u8 colorRegId;

  //GX_SetTevAlphaIn() arguments
  u8 alphaIn[4]; //GC_CA_*

  //GX_SetTevAlphaOp() arguments
  u8 alphaOp;
  u8 alphaBias;
  u8 alphaScale;
  u8 alphaClamp;
  u8 alphaRegId;
};

struct Material
{
  u8 flag;
  u8 cullIndex;
  u8 numChansIndex;
  u8 texGenCountIndex;
  u8 tevCountIndex;

  u8 zModeIndex;

  u16 color1[2];
  u16 chanControls[4];
  u16 color2[2];

  u16 texGenInfos[8];

  u16 texMtxInfos[8];

  u16 texStages[8];
  //constColor (GX_TEV_KCSEL_K0-3)
  u16 color3[4];
  u8 constColorSel[16]; //0x0c most of the time (const color sel, GX_TEV_KCSEL_*)
  u8 constAlphaSel[16]; //0x1c most of the time (const alpha sel, GX_TEV_KASEL_*)
  u16 tevOrderInfo[16];
  //this is to be loaded into
  //GX_CC_CPREV - GX_CC_A2??
  u16 colorS10[4];
  u16 tevStageInfo[16];
  u16 tevSwapModeInfo[16];
  u16 tevSwapModeTable[4];

  u16 alphaCompIndex;
  u16 blendIndex;
};

struct Mat3
{
  std::vector<MColor> color1;
  std::vector<u8> numChans;
  std::vector<ColorChanInfo> colorChanInfos;
  std::vector<MColor> color2;

  std::vector<Material> materials;
  std::vector<int> indexToMatIndex;
  std::vector<std::string> stringtable;

  std::vector<u32> cullModes;

  std::vector<u8> texGenCounts;
  std::vector<TexGenInfo> texGenInfos;
  std::vector<TexMtxInfo> texMtxInfos;

  std::vector<int> texStageIndexToTextureIndex;
  std::vector<TevOrderInfo> tevOrderInfos;
  std::vector<Color16> colorS10;
  std::vector<MColor> color3;
  std::vector<u8> tevCounts;
  std::vector<TevStageInfo> tevStageInfos;
  std::vector<TevSwapModeInfo> tevSwapModeInfos;
  std::vector<TevSwapModeTable> tevSwapModeTables;
  std::vector<AlphaCompare> alphaCompares;
  std::vector<BlendInfo> blendInfos;
  std::vector<ZMode> zModes;
};

void dumpMat3(FILE* f, Mat3& dst);
void writeMat3Info(FILE* f, std::ostream& out);;

#endif //BMD_MAT3_H
