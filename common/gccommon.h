#ifndef GCCOMMON_H
#define GCCOMMON_H GCCOMMON_H

/*
 * On my Intel OS X, BIG_ENDIAN is defined for some reason when <sys/types.h>
 * is included (which it is by <vector> for example), so don't check for it.
 */

#if defined __BIG_ENDIAN__ || (defined __APPLE__ && defined __POWERPC__)
#define GC_BIG_ENDIAN
#endif

//sanity check
#if (defined __LITTLE_ENDIAN__ || defined LITTLE_ENDIAN) && defined GC_BIG_ENDIAN
#error Unable to determine endianness
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#ifdef _MSC_VER
typedef unsigned __int64 u64;
#else
typedef unsigned long long u64;
#endif

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
#ifdef _MSC_VER
typedef signed __int64 s64;
#else
typedef signed long long s64;
#endif

typedef float f32;

inline void toWORD(u16& w)
{
#ifndef GC_BIG_ENDIAN
  u8 w1 = w & 0xFF;
  u8 w2 = w >> 8;
  w = (w1 << 8) | w2;
#endif
}

inline void toSHORT(s16& w)
{
  toWORD(*(u16*)&w);
}

inline void toDWORD(u32& d)
{
#ifndef GC_BIG_ENDIAN
  u8 w1 = d & 0xFF;
  u8 w2 = (d >> 8) & 0xFF;
  u8 w3 = (d >> 16) & 0xFF;
  u8 w4 = d >> 24;
  d = (w1 << 24) | (w2 << 16) | (w3 << 8) | w4;
#endif
}

inline void toFLOAT(f32& f)
{ toDWORD(*(u32*)&f); }


inline u16 aWORD(u16 w)
{
  toWORD(w); return w;
}

inline s16 aSHORT(s16 s)
{
  return aWORD(*(u16*)&s);
}

inline u32 aDWORD(u32 d)
{
  toDWORD(d); return d;
}

inline f32 aFLOAT(f32 f)
{
  toFLOAT(f); return f;
}


inline u16 memWORD(const u8* where)
{
  return (where[0] << 8) | where[1];
}

inline u32 memDWORD(const u8* where)
{
  return (where[0] << 24) | (where[1] << 16) | (where[2] << 8) | where[3];
}


#include <stdio.h>

inline void readWORD(FILE* f, u16& v)
{
  int len;
  len = fread(&v, 2, 1, f);
  len=len;
  toWORD(v);
}

inline void readSHORT(FILE* f, s16& v)
{
  int len;
  len = fread(&v, 2, 1, f);
  len=len;
  toSHORT(v);
}

inline void readDWORD(FILE* f, u32& v)
{
  int len;
  len = fread(&v, 4, 1, f);
  len=len;
  toDWORD(v);
}

inline void readFLOAT(FILE* f, f32& v)
{
  int len;
  len = fread(&v, 4, 1, f);
  len=len;
  toFLOAT(v);
}



inline void toWORD_le(u16& w)
{
#ifdef GC_BIG_ENDIAN
  u8 w1 = w & 0xFF;
  u8 w2 = w >> 8;
  w = (w1 << 8) | w2;
#endif
}

inline void toSHORT_le(s16& w)
{
  toWORD_le(*(u16*)&w);
}

inline void toDWORD_le(u32& d)
{
#ifdef GC_BIG_ENDIAN
  u8 w1 = d & 0xFF;
  u8 w2 = (d >> 8) & 0xFF;
  u8 w3 = (d >> 16) & 0xFF;
  u8 w4 = d >> 24;
  d = (w1 << 24) | (w2 << 16) | (w3 << 8) | w4;
#endif
}

inline void toFLOAT_le(f32& f)
{ toDWORD_le(*(u32*)&f); }


inline u16 aWORD_le(u16 w)
{
  toWORD_le(w); return w;
}

inline s16 aSHORT_le(s16 s)
{
  return aWORD_le(*(u16*)&s);
}

inline u32 aDWORD_le(u32 d)
{
  toDWORD_le(d); return d;
}

inline f32 aFLOAT_le(f32 f)
{
  toFLOAT_le(f); return f;
}

inline u16 memWORD_le(const u8* where)
{
  return where[0] | (where[1] << 8);
}

inline u32 memDWORD_le(const u8* where)
{
  return where[0] | (where[1] << 8) | (where[2] << 16) | (where[3] << 24);
}


#include <stdio.h>

inline void writeWORD_le(FILE* f, u16 v)
{
  toWORD_le(v);
  fwrite(&v, 2, 1, f);
}

inline void writeSHORT_le(FILE* f, s16 v)
{
  toSHORT_le(v);
  fwrite(&v, 2, 1, f);
}

inline void writeDWORD_le(FILE* f, u32 v)
{
  toDWORD_le(v);
  fwrite(&v, 4, 1, f);
}

inline void writeFLOAT_le(FILE* f, f32 v)
{
  toFLOAT_le(v);
  fwrite(&v, 4, 1, f);
}


#endif //GCCOMMON_H
