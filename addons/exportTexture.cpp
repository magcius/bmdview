#include "exportTexture.h"

#include <cassert>
#include <set>
#include <algorithm>
#include <string.h> // memset

#include "../common.h"

struct ColorCaps
{
  u32 size;
  u32 flags;
  char fourCC[4];
  u32 rgbBitCount;
  u32 rBitMask;
  u32 gBitMask;
  u32 bBitMask;
  u32 aBitMask;
};

struct DdsHeader
{
  char type[4];
  u32 size;
  u32 flags;
  u32 height;
  u32 width;
  u32 linearSize;
  u32 depth;
  u32 numMips;
  u32 unused[11];
  ColorCaps colorCaps;
  u32 caps;
  u32 unused2[4];
};


struct TgaHeader
{
  u8  identsize;          // size of ID field that follows 18 uint8 header (0 usually)
  u8  colourmaptype;      // type of colour map 0=none, 1=has palette
  u8  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

  s16 colourmapstart;     // first colour map entry in palette
  s16 colourmaplength;    // number of colours in palette
  u8  colourmapbits;      // number of bits per palette entry 15,16,24,32

  s16 xstart;             // image x origin
  s16 ystart;             // image y origin
  s16 width;              // image width in pixels
  s16 height;             // image height in pixels
  u8  bits;               // image bits per pixel 8,16,24,32
  u8  descriptor;         // image descriptor bits (vh flip bits)
};


void writeColorCaps(FILE* f, const ColorCaps& cc)
{
  writeDWORD_le(f, cc.size);
  writeDWORD_le(f, cc.flags);
  fwrite(cc.fourCC, 1, 4, f);
  writeDWORD_le(f, cc.rgbBitCount);
  writeDWORD_le(f, cc.rBitMask);
  writeDWORD_le(f, cc.gBitMask);
  writeDWORD_le(f, cc.bBitMask);
  writeDWORD_le(f, cc.aBitMask);
}

void writeDdsHeader(FILE* f, const DdsHeader& h)
{
  fwrite(h.type, 1, 4, f);
  writeDWORD_le(f, h.size);
  writeDWORD_le(f, h.flags);
  writeDWORD_le(f, h.height);
  writeDWORD_le(f, h.width);
  writeDWORD_le(f, h.linearSize);
  writeDWORD_le(f, h.depth);
  writeDWORD_le(f, h.numMips);
  for(int i = 0; i < 11; ++i)
    writeDWORD_le(f, h.unused[i]);
  writeColorCaps(f, h.colorCaps);
  writeDWORD_le(f, h.caps);
  for(int j = 0; j < 4; ++j)
    writeDWORD_le(f, h.unused2[j]);
}

void writeTgaHeader(FILE* f, const TgaHeader& h)
{
  fwrite(&h.identsize, 1, 1, f);
  fwrite(&h.colourmaptype, 1, 1, f);
  fwrite(&h.imagetype, 1, 1, f);

  writeSHORT_le(f, h.colourmapstart);
  writeSHORT_le(f, h.colourmaplength);
  fwrite(&h.colourmapbits, 1, 1, f);

  writeSHORT_le(f, h.xstart);
  writeSHORT_le(f, h.ystart);
  writeSHORT_le(f, h.width);
  writeSHORT_le(f, h.height);
  fwrite(&h.bits, 1, 1, f);
  fwrite(&h.descriptor, 1, 1, f);
}

//in tex1.cpp (TODO: create imageutils header)
void decompressDxt1(u8* dest, const u8* src, int w, int h);

void i8a8ToRgba8(u8* dest, const u8* src, int w, int h)
{
  for(int y = 0; y < h; ++y)
  {
    for(int x = 0; x < w; ++x)
    {
      u8 i8 = src[2*x + 0];
      u8 a8 = src[2*x + 1];
      dest[4*x + 0] = i8;
      dest[4*x + 1] = i8;
      dest[4*x + 2] = i8;
      dest[4*x + 3] = a8;
    }

    src += 2*w;
    dest += 4*w;
  }
}

void flipVertical(std::vector<u8>& vec, int w, int h, int bpp)
{
  assert(w*h*bpp == (int)vec.size());
  std::vector<u8> tmpLine(w*bpp);
  u8* up = &vec[0], * down = &vec[w*(h - 1)*bpp];
  for(int y = 0; y < h/2; ++y)
  {
    std::copy(up, up + w*bpp, tmpLine.begin());
    std::copy(down, down + w*bpp, up);
    std::copy(tmpLine.begin(), tmpLine.end(), down);

    up += w*bpp;
    down -= w*bpp;
  }
}

DdsHeader createDdsHeader(int w, int h, int numMips)
{
  DdsHeader ret;
  memset(&ret, 0, sizeof(ret));

  strncpy(ret.type, "DDS ", 4);
  ret.size = 124;
  ret.flags = 0x21007; //mipmapcount + pixelformat + width + height + caps
  ret.width = w;
  ret.height = h;
  ret.numMips = numMips;
  ret.colorCaps.size = 32;
  ret.caps = 0x401000; //mipmaps + texture
  return ret;
}

void saveTextureDds(const std::string& filename, const Image& img)
{
  DdsHeader h = createDdsHeader(img.width, img.height, img.mipmaps.size());
  switch(img.format)
  {
    case I8:
      h.colorCaps.flags = 0x20000; //luminance
      h.colorCaps.rgbBitCount = 8;
      h.colorCaps.rBitMask = 0xff;
      break;
    case I8_A8:
      h.colorCaps.flags = 0x20001; //luminance + alpha
      h.colorCaps.rgbBitCount = 16;
      h.colorCaps.rBitMask = 0xff;
      h.colorCaps.aBitMask = 0xff00;
      break;
    case RGBA8:
      h.colorCaps.flags = 0x41; //rgb + alpha
      h.colorCaps.rgbBitCount = 32;
      h.colorCaps.rBitMask = 0xff;
      h.colorCaps.gBitMask = 0xff00;
      h.colorCaps.bBitMask = 0xff0000;
      h.colorCaps.aBitMask = 0xff000000;
      break;
    case DXT1:
      h.colorCaps.flags = 0x4; //fourcc
      strncpy(h.colorCaps.fourCC, "DXT1", 4);
      break;
  }

  FILE* file = fopen(filename.c_str(), "wb");
  if(file == NULL)
    return;

  writeDdsHeader(file, h);
  fwrite(&img.imageData[0], 1, img.imageData.size(), file);
  fclose(file);
}

void saveTextureTga(const std::string& filename, const Image& img)
{
  TgaHeader h = { 0, 0, 2, 0, 0, 0, 0, 0, img.width, img.height, 32, 0 };

  FILE* file = fopen(filename.c_str(), "wb");
  if(file == NULL)
    return;

  if(img.format == I8)
  {
    //greyscale
    h.imagetype = 3;
    h.bits = 8;
    writeTgaHeader(file, h);

    //flip vertical
    std::vector<u8> data(h.width*h.height);
    std::copy(img.imageData.begin(), img.imageData.begin() + data.size(),
      data.begin());
    flipVertical(data, h.width, h.height, 1);
    fwrite(&data[0], 1, data.size(), file);
  }
  else
  {
    //convert to rgba8, save
    writeTgaHeader(file, h);

    std::vector<u8> data(h.width*h.height*4);
    switch(img.format)
    {
      case I8_A8: //convert i8a8 to rgba8
        i8a8ToRgba8(&data[0], &img.imageData[0], h.width, h.height);
        break;
      case RGBA8: //rgba8 - write directly
        memcpy(&data[0], &img.imageData[0], data.size());
        break;
      case DXT1: //convert dxt1 to rgba8
        decompressDxt1(&data[0], &img.imageData[0], h.width, h.height);
        break;
    }

    //data is rgba8, targa stores bgra8, convert:
    for(size_t i = 0; i < data.size(); i += 4)
      std::swap(data[i + 0], data[i + 2]);

    //data is top-down, targa stores bottom up
    flipVertical(data, h.width, h.height, 4);

    fwrite(&data[0], 1, data.size(), file);
  }

  fclose(file);
}

void mirrorBlockX(u8* block)
{
  block += 4; //skip "palette"
  for(int i = 0; i < 4; ++i)
  {
    block[i] = (block[i] >> 6) | (block[i] << 6) | ((block[i] & 0x30) >> 2)
      | ((block[i] & 0xc) << 2);
  }
}

const Image* mirrorX(Image& out, const Image& in)
{
  //this assumes that in.width%4 == 0 for dxt1 images

  out.format = in.format;
  out.width = 2*in.width;
  out.height = in.height;
  out.mipmaps.resize(in.mipmaps.size());
  out.sizes.resize(in.sizes.size());
  out.imageData.resize(2*in.imageData.size());

  int sizeSum = 0;
  int w = in.width, h = in.height;
  for(size_t i = 0; i < in.mipmaps.size(); ++i)
  {
    out.sizes[i] = 2*in.sizes[i];
    out.mipmaps[i] = &out.imageData[0] + sizeSum;
    sizeSum += out.sizes[i];


    //copy data in left half, mirror data in right half
    u8* dst = out.mipmaps[i];
    u8* src = in.mipmaps[i];
    if(in.format == DXT1) //dxt1
    {
      int numBlocks = w/4;
      int blockLineLength = numBlocks*8;

      for(int y = 0; y < h; y += 4)
      {
        //left
        memcpy(dst, src, blockLineLength);

        //right
        for(int x = 0; x < numBlocks; ++x)
        {
          memcpy(dst + blockLineLength + x*8, src + (numBlocks - x - 1)*8, 8);
          mirrorBlockX(dst + blockLineLength + x*8);
        }

        dst += 2*blockLineLength;
        src += blockLineLength;
      }
    }
    else
    {
      int pixSize;
      switch(in.format)
      {
        case I8:
          pixSize = 1;
          break;
        case I8_A8:
          pixSize = 2;
          break;
        case RGBA8:
          pixSize = 4;
          break;
      }
      int lineLength = w*pixSize;

      for(int y = 0; y < h; ++y)
      {
        //left
        memcpy(dst, src, lineLength);

        //right
        for(int x = 0; x < w; ++x)
          memcpy(dst + lineLength + x*pixSize, src + (w - x - 1)*pixSize, pixSize);

        dst += 2*lineLength;
        src += lineLength;
      }
    }


    w /= 2;
    h /= 2;
  }

  return &out;
}


void mirrorBlockY(u8* block)
{
  block += 4; //skip "palette"
  std::swap(block[0], block[3]);
  std::swap(block[1], block[2]);
}

const Image* mirrorY(Image& out, const Image& in)
{
  //this assumes that in.height%4 == 0 for dxt1 images

  out.format = in.format;
  out.width = in.width;
  out.height = 2*in.height;
  out.mipmaps.resize(in.mipmaps.size());
  out.sizes.resize(in.sizes.size());
  out.imageData.resize(2*in.imageData.size());

  int sizeSum = 0;
  int w = in.width, h = in.height;
  for(size_t i = 0; i < in.mipmaps.size(); ++i)
  {
    out.sizes[i] = 2*in.sizes[i];
    out.mipmaps[i] = &out.imageData[0] + sizeSum;
    sizeSum += out.sizes[i];


    //copy data in lower half, mirror data in upper half
    u8* dst = out.mipmaps[i];
    u8* src = in.mipmaps[i];
    if(in.format == 14) //dxt1
    {
      int numBlocks = w/4;
      int blockLineLength = numBlocks*8;

      //lower
      memcpy(dst, src, in.sizes[i]);

      //upper
      for(int y = 0; y < (h/4); ++y)
      {
        memcpy(dst + in.sizes[i] + y*blockLineLength,
               src + (h/4 - y - 1)*blockLineLength, blockLineLength);
        for(int x = 0; x < numBlocks; ++x)
          mirrorBlockY(dst + in.sizes[i] + y*blockLineLength + x*8);
      }
    }
    else
    {
      int pixSize;
      switch(in.format)
      {
        case I8:
          pixSize = 1;
          break;
        case I8_A8:
          pixSize = 2;
          break;
        case RGBA8:
          pixSize = 4;
          break;
      }
      int lineLength = w*pixSize;

      //lower
      memcpy(dst, src, lineLength*h);

      for(int y = 0; y < h; ++y)
        memcpy(dst + lineLength*(h + y), src + (h - y - 1)*lineLength,
               lineLength);
    }


    w /= 2;
    h /= 2;
  }

  return &out;
}


void saveTexture(IMAGETYPE imgType, const Image& img,
                 const std::string& filename, bool doMirrorX, bool doMirrorY)
{
  Image tmp, tmp2;
  const Image* imageToUse = &img;

  if(doMirrorX)
    imageToUse = mirrorX(tmp, *imageToUse);
  if(doMirrorY)
    imageToUse = mirrorY(tmp2, *imageToUse);

  switch(imgType)
  {
    case DDS:
      saveTextureDds(filename + ".dds", *imageToUse);
      break;
    case TGA:
      saveTextureTga(filename + ".tga", *imageToUse);
      break;
  }
}

void exportTextures(IMAGETYPE imgType, const Tex1& tex,
                    const std::string& basename)
{
  //export every image only once
  std::set<Image*> isAlreadyExported;

  for(size_t i = 0; i < tex.imageHeaders.size(); ++i)
  {
    const ImageHeader& h = tex.imageHeaders[i];
    if(isAlreadyExported.find(h.data)
       == isAlreadyExported.end())
    {
      isAlreadyExported.insert(h.data);

      char buff[1024]; sprintf(buff, "%02d%s_%d_%d", (int)i, h.name.c_str(),
          h.data->originalFormat, h.data->paletteFormat);
      saveTexture(imgType, *h.data, basename + buff);
    }
  }
}
