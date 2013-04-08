#include "tex1.h"
#include <set>
#include <map>
#include <iostream>
#include <string.h>

#include "GL/glew.h"

using namespace std;

namespace bmd
{

// TEX1 /////////////////////////////////////////////////////////////

//header format for 'bmd3' files, seems to be slightly different for 'jpa1'
struct Tex1Header
{
  char tag[4]; //'TEX1'
  u32 sizeOfSection;
  u16 numImages;
  u16 unknown; //padding, usually 0xffff
  u32 textureHeaderOffset; //numImages bti image headers are stored here (see bti spec)
                           //note: several image headers may point to same image data
                           //offset relative to Tex1Header start

  u32 stringTableOffset;   //stores one filename for each image (TODO: details on stringtables)
                           //offset relative to Tex1Header start
};


const int I4 = 0;
const int I8 = 1;
const int A4_I4 = 2;
const int A8_I8 = 3;
const int R5_G6_B5 = 4;
const int A3_RGB5 = 5;
const int ARGB8 = 6;
const int INDEX4 = 8;
const int INDEX8= 9;
const int INDEX14_X2 = 10;
const int S3TC1 = 14;

const int PAL_A8_I8 = 0;
const int PAL_R5_G6_B5 = 1;
const int PAL_A3_RGB5 = 2;

struct TextureHeader
{
  //0 - i4
  //1 - i8
  //2 - a4i4
  //3 - a8i8
  //4 - r5g6b5
  //5 - rgb5a3
  //6 - argb8
  //
  //8 - index4
  //9 - index8
  //10 -index14x2
  //
  //14 - dxt1 compressed
  //(see tpl's format in yagcd for more details)
  u8 format;
  u8 unknown; //0 or cc, 1, 2 (geostar (texmatrix).bmd)
  u16 width;
  u16 height;

  /*
    from gx.h:
    0: clamp to edge
    1: repeat
    2: mirror
  */
  u8 wrapS;
  u8 wrapT;

  u8 unknown3; // 0, 1 (gnd)


  //0 - a8i8
  //1 - r5g6b5
  //2 - rgb5a3
  //(see tpl's palette format in yagcd for more details)
  u8 paletteFormat;
  u16 paletteNumEntries;
  u32 paletteOffset; //palette data


  u32 unknown5; //sometimes 0x1_00_0000 when mipmapCount > 1

  //0 - nearest
  //1 - linear
  //2 - near_mip_near
  //3 - lin_mip_near
  //4 - near_mip_lin
  //5 - lin_mip_lin
  u8 minFilter;
  u8 magFilter; //??

  u16 unknown7; //0 most of the time,
                //sometimes 0x10, 0x18 (mariocap), 0x20, 0x28
  u8 mipmapCount;
  u8 unknown8; //0 (nomips), 1 (nomips), d, 48, 4d, 56 (nomips),
               //58, 61, 8f, da or ff (hmm...0-ff ;-) )
  u16 unknown9; //0 (nomips), 7, 20, 74 (in airport.bmd), ffee, ffe3 (sea.bmd)

  u32 dataOffset; //image data

  //some of the unknown data could be render state?
  //(lod bias, transparent color (? could be in shader as well...), ...)
  /*
  void GX_InitTexObj(GXTexObj *obj,void *img_ptr,u16 wd,u16 ht,u8 fmt,u8 wrap_s,u8 wrap_t,u8 mipmap);
  void GX_InitTexObjCI(GXTexObj *obj,void *img_ptr,u16 wd,u16 ht,u8 fmt,u8 wrap_s,u8 wrap_t,u8 mipmap,u32 tlut_name);
  void GX_InitTexObjLOD(GXTexObj *obj,u8 minfilt,u8 magfilt,f32 minlod,f32 maxlod,f32 lodbias,u8 biasclamp,u8 edgelod,u8 maxaniso);
  void GX_SetTexCoorScaleManually(u8 texcoord,u8 enable,u16 ss,u16 ts);
  void GX_SetTexCoordBias(u8 texcoord,u8 s_enable,u8 t_enable);
  */
};

};

void loadAndConvertImage(FILE* f, const bmd::TextureHeader& h, long baseOffset,
                         Image& curr);

#include "simple_gl.h"

void r5g6b5ToRgba8(u16 srcPixel, u8* dest);

void decompressDxt1(u8* dest, const u8* src, int w, int h)
{
  const u8* runner = src;
  for(int y = 0; y < h; y += 4)
  {
    for(int x = 0; x < w; x += 4)
    {
      u16 color1 = memWORD_le(runner);
      u16 color2 = memWORD_le(runner + 2);
      u32 bits = memDWORD_le(runner + 4);
      runner += 8;

      //prepare color table
      u8 colorTable[4][4];
      r5g6b5ToRgba8(color1, colorTable[0]);
      r5g6b5ToRgba8(color2, colorTable[1]);
      if(color1 > color2)
      {
        colorTable[2][0] = (2*colorTable[0][0] + colorTable[1][0] + 1) / 3;
        colorTable[2][1] = (2*colorTable[0][1] + colorTable[1][1] + 1) / 3;
        colorTable[2][2] = (2*colorTable[0][2] + colorTable[1][2] + 1) / 3;
        colorTable[2][3] = 0xff;

        colorTable[3][0] = (colorTable[0][0] + 2*colorTable[1][0] + 1) / 3;
        colorTable[3][1] = (colorTable[0][1] + 2*colorTable[1][1] + 1) / 3;
        colorTable[3][2] = (colorTable[0][2] + 2*colorTable[1][2] + 1) / 3;
        colorTable[3][3] = 0xff;
      }
      else
      {
        colorTable[2][0] = (colorTable[0][0] + colorTable[1][0] + 1) / 2;
        colorTable[2][1] = (colorTable[0][1] + colorTable[1][1] + 1) / 2;
        colorTable[2][2] = (colorTable[0][2] + colorTable[1][2] + 1) / 2;
        colorTable[2][3] = 0xff;

        //only the alpha value of this color is important...
        colorTable[3][0] = (colorTable[0][0] + 2*colorTable[1][0] + 1) / 3;
        colorTable[3][1] = (colorTable[0][1] + 2*colorTable[1][1] + 1) / 3;
        colorTable[3][2] = (colorTable[0][2] + 2*colorTable[1][2] + 1) / 3;
        colorTable[3][3] = 0x00;
      }

      //decode image
      for(int iy = 0; iy < 4; ++iy)
        for(int ix = 0; ix < 4; ++ix)
        {
          if(x + ix < w && y + iy < h)
          {
            u32 di = 4*((y + iy)*w + x + ix);
            u32 si = bits & 0x3;
            dest[di + 0] = colorTable[si][0];
            dest[di + 1] = colorTable[si][1];
            dest[di + 2] = colorTable[si][2];
            dest[di + 3] = colorTable[si][3];
          }
          bits >>= 2;
        }
    }
  }
}

//checks if a positive number is a power of two
bool isPot(int i)
{
  if(i == 0)
    return false;

  return (i & (i - 1)) == 0;
}

int nextPot(int i)
{
  int ret = 1;
  while(ret < i)
    ret *= 2;
  return ret;
}

bool needsResizing(int w, int h)
{
  return (!isPot(w) || !isPot(h)) && !GLEW_ARB_texture_non_power_of_two;
}

void texImage2d(GLenum format, int level, int w, int h, u8* data)
{
  GLenum internalFormat = format;
  if(format == GL_INTENSITY)
  {
    internalFormat = GL_INTENSITY;
    format = GL_LUMINANCE;
  }

  if(!needsResizing(w, h))
    glTexImage2D(GL_TEXTURE_2D, level, internalFormat,
      w, h, 0, format, GL_UNSIGNED_BYTE, data);
  else
  {
    int nw = nextPot(w);
    int nh = nextPot(h);

    vector<u8> tmp(nw*nh*4); //large enough for every format
    gluScaleImage(format, w, h, GL_UNSIGNED_BYTE, data,
      nw, nh, GL_UNSIGNED_BYTE, &tmp[0]);
    glTexImage2D(GL_TEXTURE_2D, level, internalFormat,
      nw, nh, 0, format, GL_UNSIGNED_BYTE, &tmp[0]);
  }
}

void uploadImageToGl(Image& currImg)
{
  glGenTextures(1, (GLuint*)&currImg.texId);
  glBindTexture(GL_TEXTURE_2D, currImg.texId);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  //bind mipmap levels
  int w = currImg.width;
  int h = currImg.height;
  for(size_t mip = 0; mip < currImg.mipmaps.size(); ++mip)
  {
    switch(currImg.format)
    {
      case I8:
        //texImage2d(GL_LUMINANCE, currImg.width, currImg.height,
        texImage2d(GL_INTENSITY, mip, w, h,
          currImg.mipmaps[mip]);
        break;

      case I8_A8:
        texImage2d(GL_LUMINANCE_ALPHA, mip, w, h,
        //texImage2d(GL_INTENSITY_ALPHA, currImg.width, currImg.height,
          currImg.mipmaps[mip]);
        break;

      case RGBA8:
        texImage2d(GL_RGBA, mip, w, h,
          currImg.mipmaps[mip]);
        break;

      case DXT1: //decompress to rgba8 if texture compression is not supported
                 //or if the texture is non-power-of-two (npot) and needs
                 //to be resized (ie. the card doesn't support npot
                 //textures).
        {
          if(needsResizing(w, h)
            || !GLEW_S3_s3tc)
          {
            //uncompress data, scale it to a pot texture if required
            vector<u8> tmp(w*h*4);
            decompressDxt1(&tmp[0], currImg.mipmaps[mip], w, h);
            if(!needsResizing(w, h))
              glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA, w, h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, &tmp[0]);
            else
            {
              int nw = nextPot(w);
              int nh = nextPot(h);

              vector<u8> tmp2(nw*nh*4);

              gluScaleImage(GL_RGBA, w, h, GL_UNSIGNED_BYTE, &tmp[0],
                nw, nh, GL_UNSIGNED_BYTE, &tmp2[0]);

              glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA, nw, nh, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, &tmp2[0]);
            }
          }
          else
            glCompressedTexImage2D(GL_TEXTURE_2D, mip,
              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, w, h, 0,
              currImg.sizes[mip], currImg.mipmaps[mip]);
        }
        break;

      default:
        warn("tex3: unsupported image format %d", currImg.format);
    }

    w = max(1, w/2);
    h = max(1, h/2);
  }
}

void uploadImagesToGl(Tex1& dst)
{
  for(size_t i = 0; i < dst.images.size(); ++i)
    uploadImageToGl(dst.images[i]);
}

void readTex1Header(FILE* f, bmd::Tex1Header& h)
{
  fread(h.tag, 1, 4, f);
  readDWORD(f, h.sizeOfSection);
  readWORD(f, h.numImages);
  readWORD(f, h.unknown);
  readDWORD(f, h.textureHeaderOffset);
  readDWORD(f, h.stringTableOffset);
}

void readTextureHeader(FILE* f, bmd::TextureHeader& texHeader)
{
  fread(&texHeader.format, 1, 1, f);
  fread(&texHeader.unknown, 1, 1, f);
  readWORD(f, texHeader.width);
  readWORD(f, texHeader.height);
  fread(&texHeader.wrapS, 1, 1, f);
  fread(&texHeader.wrapT, 1, 1, f);
  fread(&texHeader.unknown3, 1, 1, f);
  fread(&texHeader.paletteFormat, 1, 1, f);
  readWORD(f, texHeader.paletteNumEntries);
  readDWORD(f, texHeader.paletteOffset);
  readDWORD(f, texHeader.unknown5);
  fread(&texHeader.minFilter, 1, 1, f);
  fread(&texHeader.magFilter, 1, 1, f);
  readWORD(f, texHeader.unknown7);
  fread(&texHeader.mipmapCount, 1, 1, f);
  fread(&texHeader.unknown8, 1, 1, f);
  readWORD(f, texHeader.unknown9);
  readDWORD(f, texHeader.dataOffset);
}

void dumpTex1(FILE* f, Tex1& dst)
{
  int tex1Offset = ftell(f);

  //read textureblock header
  bmd::Tex1Header h;
  readTex1Header(f, h);

  //read stringtable
  vector<string> stringtable;
  readStringtable(tex1Offset + h.stringTableOffset, f, stringtable);

  if(stringtable.size() != h.numImages)
    warn("tex1: number of strings (%d) doesn't match number of images (%d)",
      stringtable.size(), h.numImages);

  //read all image headers before loading the actual image
  //data, because several headers can refer to the same data
  fseek(f, tex1Offset + h.textureHeaderOffset, SEEK_SET);
  size_t i;
  vector<bmd::TextureHeader> texHeaders(h.numImages);
  map<long, bmd::TextureHeader*> imageOffsets; //detects multiple offsets
  for(i = 0; i < h.numImages; ++i)
  {
    readTextureHeader(f, texHeaders[i]);

    //check if this header refers to existing data
    int effectiveOffset = texHeaders[i].dataOffset + 0x20*i;
    if(imageOffsets.find(effectiveOffset) == imageOffsets.end())
      imageOffsets[effectiveOffset] = &texHeaders[i];
    else
    {
      //debug info
      bmd::TextureHeader& a = texHeaders[i], & b = *imageOffsets[effectiveOffset];
      if(a.width != b.width || a.height != b.height || a.format != b.format
        || a.mipmapCount != b.mipmapCount
        //|| a.wrapS != b.wrapS || a.wrapT != b.wrapT //this is a per-header value (see ji.bdl in zelda)
        )
        warn("tex1: two headers refering to same data have different formats");
        //TODO: what if effective palette offsets differ?
    }
  }

  //read image data
  dst.imageHeaders.resize(h.numImages);
  dst.images.resize(imageOffsets.size());
  map<long, Image*> loadedImages;
  int j = 0;
  for(i = 0; i < h.numImages; ++i)
  {
    dst.imageHeaders[i].wrapS = texHeaders[i].wrapS;
    dst.imageHeaders[i].wrapT = texHeaders[i].wrapT;
    dst.imageHeaders[i].minFilter = texHeaders[i].minFilter;
    dst.imageHeaders[i].magFilter = texHeaders[i].magFilter;

    if(i < stringtable.size()) //should always be true
      dst.imageHeaders[i].name = stringtable[i];

    //check if the image needed by current header is
    //already loaded, if not, load it
    int effectiveOffset = texHeaders[i].dataOffset + 0x20*i;
    map<long, Image*>::iterator it = loadedImages.find(effectiveOffset);
    if(it != loadedImages.end())
      dst.imageHeaders[i].data = it->second;
    else
    {
      Image* curr = &dst.images[j];
      ++j;

      dst.imageHeaders[i].data = curr;
      loadedImages[effectiveOffset] = curr;

      loadAndConvertImage(f, texHeaders[i],
                          tex1Offset + h.textureHeaderOffset + 0x20*i, *curr);
    }
  }
}

//returns how many bytes an image of given format
//and dimensions needs in the file (NOT counting mipmaps)
int getCompressedBufferSize(u8 format, int w, int h)
{
  int w8 = w + (8 - w%8)%8;
  int w4 = w + (4 - w%4)%4;
  int h8 = h + (8 - h%8)%8;
  int h4 = h + (4 - h%4)%4;

  switch(format)
  {
    case bmd::I4:
      return w8*h8/2;
    case bmd::I8:
      return w8*h4;
    case bmd::A4_I4:
      return w8*h4;
    case bmd::A8_I8:
      return w4*h4*2;
    case bmd::R5_G6_B5:
      return w4*h4*2;
    case bmd::A3_RGB5:
      return w4*h4*2;
    case bmd::ARGB8:
      return w4*h4*4;
    case bmd::INDEX4:
      return w8*h8/2;
    case bmd::INDEX8:
      return w8*h4;
    case bmd::INDEX14_X2:
      return w4*h4*2;
    case bmd::S3TC1:
      return w4*h4/2;
    default:
      return -1;
  }
}

u8 getUncompressedBufferFormat(u8 format, u8 paletteFormat)
{
  switch(format)
  {
    case bmd::I4:
    case bmd::I8:
      return I8;
    case bmd::A4_I4: //a4i4 -> i8a8
    case bmd::A8_I8: //a8i8 -> i8a8
      return I8_A8;
    case bmd::R5_G6_B5:
    case bmd::A3_RGB5:
    case bmd::ARGB8:
      return RGBA8;

    case bmd::INDEX4:
    case bmd::INDEX8:
    case bmd::INDEX14_X2:
      switch(paletteFormat)
      {
        case bmd::PAL_A8_I8: //a8i8 -> i8a8
          return I8_A8;
        case bmd::PAL_R5_G6_B5: //r5g6b5 -> rgba8
        case bmd::PAL_A3_RGB5: //rgb5a3 -> rgba8
          return RGBA8;
        default:
          return -1;
      }

    case bmd::S3TC1:
      return DXT1;
    default:
      return -1;
  }
}

//returns how many bytes an image of given format
//and dimensions needs in memory after uncompression etc
int getUncompressedBufferSize(u8 format, int w, int h, u8 paletteFormat)
{
  int w4 = w + (4 - w%4)%4;
  int h4 = h + (4 - h%4)%4;

  switch(getUncompressedBufferFormat(format, paletteFormat))
  {
    case I8:
      return w*h;
    case I8_A8:
      return w*h*2;
    case RGBA8:
      return w*h*4;
    case DXT1:
      return w4*h4/2;
    default:
      return -1;
  }
}


//new, fixed version
void fix8x8Expand(u8* dest, const u8* src, int w, int h)
{
  //convert to i8 during block swapping
  int si = 0;
  for(int y = 0; y < h; y += 8)
    for(int x = 0; x < w; x += 8)
      for(int dy = 0; dy < 8; ++dy)
        for(int dx = 0; dx < 8; dx += 2, ++si)
          if(x + dx < w && y + dy < h)
          {
            //http://www.mindcontrol.org/~hplus/graphics/expand-bits.html
            u8 t = src[si] & 0xf0;
            dest[w*(y + dy) + x + dx] = t | (t >> 4);
            t = (src[si] & 0xf);
            dest[w*(y + dy) + x + dx + 1] = (t << 4) | t;
          }
}


void fix8x8NoExpand(u8* dest, const u8* src, int w, int h)
{
  //convert to i8 during block swapping
  int si = 0;
  for(int y = 0; y < h; y += 8)
    for(int x = 0; x < w; x += 8)
      for(int dy = 0; dy < 8; ++dy)
        for(int dx = 0; dx < 8; dx += 2, ++si)
          if(x + dx < w && y + dy < h)
          {
            //http://www.mindcontrol.org/~hplus/graphics/expand-bits.html
            u8 t = src[si] & 0xf0;
            dest[w*(y + dy) + x + dx] = (t >> 4);
            t = (src[si] & 0xf);
            dest[w*(y + dy) + x + dx + 1] = t;
          }
}


void fix8x4(u8* dest, const u8* src, int w, int h)
{
  int si = 0;
  for(int y = 0; y < h; y += 4)
    for(int x = 0; x < w; x += 8)
      for(int dy = 0; dy < 4; ++dy)
        for(int dx = 0; dx < 8; ++dx, ++si)
          if(x + dx < w && y + dy < h)
            dest[w*(y + dy) + x + dx] = src[si];
}

void fix8x4Expand(u8* dest, const u8* src, int w, int h)
{
  int si = 0;
  for(int y = 0; y < h; y += 4)
    for(int x = 0; x < w; x += 8)
      for(int dy = 0; dy < 4; ++dy)
        for(int dx = 0; dx < 8; ++dx, ++si)
          if(x + dx < w && y + dy < h)
          {
            u8 lum = src[si] & 0xf;
            lum |= lum << 4;
            u8 alpha = src[si] & 0xf0;
            alpha |= (alpha >> 4);
            dest[2*(w*(y + dy) + x + dx)] = lum;
            dest[2*(w*(y + dy) + x + dx) + 1] = alpha;
          }
}

void fix4x4(u8* dest, const u8* src, int w, int h)
{
  int si = 0;
  for(int y = 0; y < h; y += 4)
    for(int x = 0; x < w; x += 4)
      for(int dy = 0; dy < 4; ++dy)
        for(int dx = 0; dx < 4; ++dx, si += 2)
          if(x + dx < w && y + dy < h)
          {
            //without byte swapping the result looks wrong. do tex1 blocks
            //store ai8 instead of ia8?
            int di = 2*(w*(y + dy) + x + dx);
            dest[di + 0] = src[si + 1];
            dest[di + 1] = src[si + 0];
          }
}

void r5g6b5ToRgba8(u16 srcPixel, u8* dest)
{
  u8 r, g, b;
  r = (srcPixel & 0xf100) >> 11;
  g = (srcPixel & 0x7e0) >> 5;
  b = (srcPixel & 0x1f);

  //http://www.mindcontrol.org/~hplus/graphics/expand-bits.html
  r = (r << (8 - 5)) | (r >> (10 - 8));
  g = (g << (8 - 6)) | (g >> (12 - 8));
  b = (b << (8 - 5)) | (b >> (10 - 8));

  dest[0] = r;
  dest[1] = g;
  dest[2] = b;
  dest[3] = 0xff;
}

void fixR5G6B5(u8* dest, const u8* src, int w, int h)
{
  //convert to rgba8 during block swapping
  //4x4 tiles
  int si = 0;
  for(int y = 0; y < h; y += 4)
    for(int x = 0; x < w; x += 4)
      for(int dy = 0; dy < 4; ++dy)
        for(int dx = 0; dx < 4; ++dx, si += 2)
          if(x + dx < w && y + dy < h)
          {
            u16 srcPixel = memWORD(src + si);
            r5g6b5ToRgba8(srcPixel, &dest[4*(w*(y + dy) + x + dx)]);
          }
}

void fixRGBA8(u8* dest, const u8* src, int w, int h)
{
  //2 4x4 input tiles per 4x4 output tile, first stores AR, second GB
  int si = 0;
  for(int y = 0; y < h; y += 4)
    for(int x = 0; x < w; x += 4)
    {
      int dy;

      //to have the texture in the format wanted by opengl,
      //we have to convert from argb to rgba

      //this is AR
      for(dy = 0; dy < 4; ++dy)
        for(int dx = 0; dx < 4; ++dx, si += 2)
          if(x + dx < w && y + dy < h)
          {
            //convert ar to rXXa
            u32 di = 4*(w*(y + dy) + x + dx);
            dest[di + 0] = src[si + 1];
            dest[di + 3] = src[si + 0];
          }

      //this is GB
      for(dy = 0; dy < 4; ++dy)
        for(int dx = 0; dx < 4; ++dx, si += 2)
          if(x + dx < w && y + dy < h)
          {
            //convert gb to XgbX and or with previous value
            u32 di = 4*(w*(y + dy) + x + dx);
            dest[di + 1] = src[si + 0];
            dest[di + 2] = src[si + 1];
          }
    }
}

void rgb5a3ToRgba8(u16 srcPixel, u8* dest)
{
  u8 r, g, b, a;

  //http://www.mindcontrol.org/~hplus/graphics/expand-bits.html
  if((srcPixel & 0x8000) == 0x8000) //rgb5
  {
    a = 0xff;

    r = (srcPixel & 0x7c00) >> 10;
    r = (r << (8-5)) | (r >> (10-8));

    g = (srcPixel & 0x3e0) >> 5;
    g = (g << (8-5)) | (g >> (10-8));

    b = srcPixel & 0x1f;
    b = (b << (8-5)) | (b >> (10-8));
  }
  else //a3rgb4
  {
    a = (srcPixel & 0x7000) >> 12;
    a = (a << (8-3)) | (a << (8-6)) | (a >> (9-8));

    r = (srcPixel & 0xf00) >> 8;
    r = (r << (8-4)) | r;

    g = (srcPixel & 0xf0) >> 4;
    g = (g << (8-4)) | g;

    b = srcPixel & 0xf;
    b = (b << (8-4)) | b;
  }

  dest[0] = r;
  dest[1] = g;
  dest[2] = b;
  dest[3] = a;
}

void fixRgb5A3(u8* dest, const u8* src, int w, int h)
{
  //convert to rgba8 during block swapping
  //4x4 tiles
  int si = 0;
  for(int y = 0; y < h; y += 4)
    for(int x = 0; x < w; x += 4)
      for(int dy = 0; dy < 4; ++dy)
        for(int dx = 0; dx < 4; ++dx, si += 2)
          if(x + dx < w && y + dy < h)
          {
            u16 srcPixel = memWORD(src + si);
            rgb5a3ToRgba8(srcPixel, &dest[4*(w*(y + dy) + x + dx)]);
          }
}

void s3tc1ReverseByte(u8& b)
{
  u8 b1 = b & 0x3;
  u8 b2 = b & 0xc;
  u8 b3 = b & 0x30;
  u8 b4 = b & 0xc0;
  b = (b1 << 6) | (b2 << 2) | (b3 >> 2) | (b4 >> 6);
}

void fixS3TC1(u8* dest, const u8* src, int w, int h)
{
  int s = 0;

  for(int y = 0; y < h/4; y += 2)
    for(int x = 0; x < w/4; x += 2)
      for(int dy = 0; dy < 2; ++dy)
        for(int dx = 0; dx < 2; ++dx, s += 8)
          if(4*(x + dx) < w && 4*(y + dy) < h)
            memcpy(&dest[8*((y + dy)*w/4 + x + dx)], &src[s], 8);

  //s3tc1 on the cube is a bit different from s3tc1 on pc graphic cards:
  for(int k = 0; k < w*h/2; k += 8)
  {
    swap(dest[k], dest[k+1]);
    swap(dest[k+2], dest[k+3]);

    s3tc1ReverseByte(dest[k+4]);
    s3tc1ReverseByte(dest[k+5]);
    s3tc1ReverseByte(dest[k+6]);
    s3tc1ReverseByte(dest[k+7]);
  }
}

int getUnpackedPixSize(u8 paletteFormat)
{
  if(paletteFormat == bmd::PAL_A8_I8)
    return 2;
  return 4;
}

void unpackPixel(int index, u8* dest, const u8* palette, u8 paletteFormat)
{
  switch(paletteFormat)
  {
    case bmd::PAL_A8_I8: //a8i8 -> i8a8
      dest[0] = palette[2*index + 1];
      dest[1] = palette[2*index + 0];
      break;

    case bmd::PAL_R5_G6_B5: //r5g6b5 -> rgba8
      r5g6b5ToRgba8(memWORD(palette + 2*index), dest);
      break;

    case bmd::PAL_A3_RGB5: //rgb5a3 -> rgba8
      rgb5a3ToRgba8(memWORD(palette + 2*index), dest);
      break;
  }
}

void unpack8(u8* dst, const u8* src, int w, int h,
             const u8* palette, u8 paletteFormat)
{
  int pixSize = getUnpackedPixSize(paletteFormat);
  u8* runner = dst;

  for(int y = 0; y < h; ++y)
    for(int x = 0; x < w; ++x, runner += pixSize)
      unpackPixel(src[y*w + x], runner, palette, paletteFormat);
}

void unpack16(u8* dst, const u8* src, int w, int h,
              const u8* palette, u8 paletteFormat)
{
  int pixSize = getUnpackedPixSize(paletteFormat);
  u8* runner = dst;

  for(int y = 0; y < h; ++y)
    for(int x = 0; x < w; ++x, runner += pixSize)
    {
      //fix4x4() swaps words to little endian...
      u16 index = memWORD_le(src + 2*(y*w + x));
      unpackPixel(index & 0x3fff, runner, palette, paletteFormat);
    }
}

//returns new format
u8 readImage(FILE* f, int w, int h, u8 format, u8* palette, u8 paletteFormat, u8* dest)
{
  int srcBufferSize = getCompressedBufferSize(format, w, h);
  vector<u8> srcVec(srcBufferSize);
  u8* src = &srcVec[0];
  fread(src, 1, srcBufferSize, f);

  //do format conversions, unpack blocks
  switch(format)
  {
    case bmd::I4: //i4 -> i8
      fix8x8Expand(dest, src, w, h);
      return I8;

    case bmd::I8: //i8
      fix8x4(dest, src, w, h);
      return I8;

    case bmd::A4_I4: //i4a4 -> i8a8
      fix8x4Expand(dest, src, w, h);
      return I8_A8;

    case bmd::A8_I8: //i8a8
      fix4x4(dest, src, w, h);
      return I8_A8;

    case bmd::R5_G6_B5: //r5g6b5 -> rgba8
      fixR5G6B5(dest, src, w, h);
      return RGBA8;

    case bmd::A3_RGB5: //rgb5a3 -> rgba8
      fixRgb5A3(dest, src, w, h);
      return RGBA8;

    case bmd::ARGB8: //argb8 -> rgba8
      fixRGBA8(dest, src, w, h);
      return RGBA8;


    case bmd::INDEX4:
    case bmd::INDEX8:
    case bmd::INDEX14_X2:
    {
      //needed for palette conversions
      //(*2 for expaned i4->i8 case)
      vector<u8> tmpVec(2*srcBufferSize);
      u8* tmp = &tmpVec[0];

      switch(format)
      {
        case bmd::INDEX4:
          fix8x8NoExpand(tmp, src, w, h);
          unpack8(dest, tmp, w, h, palette, paletteFormat);
          break;

        case bmd::INDEX8:
          fix8x4(tmp, src, w, h);
          unpack8(dest, tmp, w, h, palette, paletteFormat);
          break;

        case bmd::INDEX14_X2:
          fix4x4(tmp, src, w, h);
          unpack16(dest, tmp, w, h, palette, paletteFormat);
          break;
      }

      switch(paletteFormat)
      {
        case bmd::PAL_A8_I8:
          return I8_A8;
        case bmd::PAL_R5_G6_B5:
        case bmd::PAL_A3_RGB5:
          return RGBA8;
        default:
          warn("tex1: unsupported palette format %d", paletteFormat);
          return 0xff; //TODO: ?
      }
    }


    case bmd::S3TC1:
      fixS3TC1(dest, src, w, h);
      return DXT1;

    default:
      warn("unsupported image format %d", format);
      return 0xff; //TODO: ?
  }
}

void loadAndConvertImage(FILE* f, const bmd::TextureHeader& h, long baseOffset,
                         Image& curr)
{
  int i;

  curr.width = h.width;
  curr.height = h.height;
  curr.format = h.format;

  if((h.format == 8 || h.format == 9 || h.format == 10)
    && (h.paletteFormat != 1 && h.paletteFormat != 2)) //never tested such an image,
                                                       //but yagcd says theres also a palette format 0
    warn("found format %d, palette format %d", h.format, h.paletteFormat);


  vector<u8> palette;
  if(h.paletteNumEntries != 0)
  {
    //read palette
    palette.resize(h.paletteNumEntries*2);
    fseek(f, baseOffset + h.paletteOffset, SEEK_SET);
    fread(&palette[0], 2, h.paletteNumEntries, f);
  }

  //calculate required image size
  int totalRequiredSize = 0;
  int wid = h.width, hyt = h.height;
  for(i = 0; i < h.mipmapCount; ++i)
  {
    totalRequiredSize += getUncompressedBufferSize(h.format, wid, hyt, h.paletteFormat);
    wid /= 2; hyt /= 2;
  }

  //get memory for image, set mipmap pointers and load image

  if(h.dataOffset == 0) //TODO: twilight princess does that
    warn("What to do, what to do? (data offset in image is 0)\n");

  fseek(f, baseOffset + h.dataOffset, SEEK_SET);
  curr.imageData.resize(totalRequiredSize);
  totalRequiredSize = 0;
  wid = h.width; hyt = h.height;
  curr.mipmaps.resize(h.mipmapCount);
  curr.sizes.resize(h.mipmapCount);
  for(i = 0; i < h.mipmapCount; ++i)
  {
    curr.mipmaps[i] = &curr.imageData[totalRequiredSize];

    curr.sizes[i] =
      getUncompressedBufferSize(h.format, wid, hyt, h.paletteFormat);

    //read image
    if(h.dataOffset != 0)
      curr.format = readImage(f, wid, hyt, h.format,
        &palette[0], h.paletteFormat, curr.mipmaps[i]);
    else
    {
      //this texture is probably rendered at runtime. for now, fill it with
      //white
      curr.format = getUncompressedBufferFormat(h.format, h.paletteFormat);
      if(curr.format != DXT1)
        memset(curr.mipmaps[i], 0xff, curr.sizes[i]);
      else
      {
        const u8 whiteBlock[] = { 0xff, 0xff, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00 };
        for(int c = 0; c < curr.sizes[i]/8; ++c)
          memcpy(curr.mipmaps[i] + 8*c, whiteBlock, 8);
      }
    }

    totalRequiredSize += getUncompressedBufferSize(h.format, wid, hyt, h.paletteFormat);
    wid /= 2; hyt /= 2;
  }

  curr.originalFormat = h.format;
  curr.paletteFormat = h.paletteFormat;
}

void writeTex1Info(FILE* f, ostream& out)
{
  out << string(50, '/') << endl
      << "//Tex1 section" << endl
      << string(50, '/') << endl << endl;

  int tex1Offset = ftell(f), i;

  //read tex1 header
  bmd::Tex1Header h;
  readTex1Header(f, h);

  //dump stringtable
  writeStringtable(out, f, tex1Offset + h.stringTableOffset);

  //read stringtable
  std::vector<std::string> stringtable;
  readStringtable(tex1Offset + h.stringTableOffset, f, stringtable);

  //image headers
  out << endl << "Image headers" << endl;
  fseek(f, tex1Offset + h.textureHeaderOffset, SEEK_SET);
  for(i = 0; i < h.numImages; ++i)
  {
    bmd::TextureHeader texHead;
    readTextureHeader(f, texHead);

    out << " \"" << stringtable[i] << "\":"
        << endl << "  "
        << " format " << (int)texHead.format
        << " unknown " << (int)texHead.unknown
        << " width " << texHead.width
        << " height " << texHead.height
        << " wrapS " << (int)texHead.wrapS
        << " wrapT " << (int)texHead.wrapT
        << endl << "  "
        << " unknown3 " << (int)texHead.unknown3
        << " palette format " << (int)texHead.paletteFormat
        << " palette num entries " << texHead.paletteNumEntries
        << " palette offset " << texHead.paletteOffset
        << endl << "  "
        << " unknown5 " << texHead.unknown5
        << " min filter " << (int)texHead.minFilter
        << " mag filter " << (int)texHead.magFilter
        << " unknown7 " << texHead.unknown7
        << endl << "  "
        << " mipmapCount " << (int)texHead.mipmapCount
        << " unknown8 " << (int)texHead.unknown8
        << " unknown9 " << texHead.unknown9
        << " dataOffset " << texHead.dataOffset << endl
        << endl;
  }

  out << endl;
}
