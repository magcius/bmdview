#include "openfile.h"

#include "common.h"

#include <vector>
#include <cstdio>
#include <string.h>
using namespace std;

#ifdef _WIN32
#include <windows.h> //GetTempPath(), GetTempFileName()

std::string getTempFileName()
{
  char pathName[MAX_PATH], fileName[MAX_PATH];
  GetTempPath(MAX_PATH, pathName);
  GetTempFileName(pathName, "yaz0", 0, fileName);
  return fileName;
}

#else

std::string getTempFileName()
{
  char fileName[L_tmpnam];
  tmpnam(fileName); //TODO: better use mkstemp()
  return fileName;
}

#endif


struct Ret
{
  int srcPos, dstPos;
};


Ret decodeYaz0(u8* src, int srcSize, u8* dst, int uncompressedSize)
{
  Ret r = { 0, 0 }; //current read/write positions

  u32 validBitCount = 0; //number of valid bits left in "code" byte
  u8 currCodeByte;
  while(r.dstPos < uncompressedSize)
  {
    //read new "code" byte if the current one is used up
    if(validBitCount == 0)
    {
      if(r.srcPos >= srcSize)
        return r;
      currCodeByte = src[r.srcPos];
      ++r.srcPos;
      validBitCount = 8;
    }

    if((currCodeByte & 0x80) != 0)
    {
      //straight copy
      if(r.srcPos >= srcSize)
        return r;
      dst[r.dstPos] = src[r.srcPos];
      r.dstPos++;
      r.srcPos++;
    }
    else
    {
      //RLE part
      if(r.srcPos >= srcSize - 1)
        return r;
      u8 byte1 = src[r.srcPos];
      u8 byte2 = src[r.srcPos + 1];
      r.srcPos += 2;


      u32 dist = ((byte1 & 0xF) << 8) | byte2;
      u32 copySource = r.dstPos - (dist + 1);

      u32 numBytes = byte1 >> 4;
      if(numBytes == 0)
      {
        if(r.srcPos >= srcSize)
          return r;
        numBytes = src[r.srcPos] + 0x12;
        r.srcPos++;
      }
      else
        numBytes += 2;

      //copy run
      for(size_t i = 0; i < numBytes; ++i)
      {
        if(r.dstPos >= uncompressedSize)
          return r;
        dst[r.dstPos] = dst[copySource];
        copySource++;
        r.dstPos++;
      }
    }

    //use next bit from "code" byte
    currCodeByte <<= 1;
    validBitCount-=1;
  }

  return r;
}

OpenedFile* openFile(const string& name)
{
  FILE* f = fopen(name.c_str(), "rb");
  if(f == NULL)
  {
    fprintf(stderr, "Failed to open \"%s\"\n", name.c_str());
    return NULL;
  }

  OpenedFile* ret = new OpenedFile;

  char buff[4];
  fread(buff, 1, 4, f);
  if(strncmp(buff, "Yaz0", 4) != 0)
  {
    fseek(f, 0, SEEK_SET);
    ret->f = f;
    return ret; //not compressed, return file directly
  }

  //yaz0-compressed file - uncompress to a temporary file,
  //return the uncompressed file

  u32 uncompressedSize;
  fread(&uncompressedSize, 4, 1, f);
  toDWORD(uncompressedSize);

  fseek(f, 0, SEEK_END);
  long compressedSize = ftell(f) - 16; //16 byte header
  fseek(f, 16, SEEK_SET); //seek to start of data

  vector<u8> srcData(compressedSize), dstData(uncompressedSize);
  fread(&srcData[0], 1, compressedSize, f);
  fclose(f);
  Ret r = decodeYaz0(&srcData[0], compressedSize,
                     &dstData[0], uncompressedSize);


  //write decompressed data to a temporary file and
  //return handle to this file
  string tempFileName = getTempFileName();

  f = fopen(tempFileName.c_str(), "wb+");
  if(f == NULL)
  {
    delete ret;
    return NULL;
  }
  fwrite(&dstData[0], 1, r.dstPos, f);
  fseek(f, 0, SEEK_SET);

  ret->f = f;
  ret->tempFileName = tempFileName;
  return ret;
}

void closeFile(OpenedFile* f)
{
  if(f == NULL)
    return;

  fclose(f->f);
  if(f->tempFileName != "") //delete temporary file
    remove(f->tempFileName.c_str());
  delete f;
}
