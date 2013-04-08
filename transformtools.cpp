#include "transformtools.h"

#include <cassert>

using namespace std;

Matrix44f frameMatrix(const Frame& f)
{
  Matrix44f t, rx, ry, rz, s;
  t.loadTranslateLM(f.t.x(), f.t.y(), f.t.z());
  rx.loadRotateXLM(f.rx/360.f*2*PI);
  ry.loadRotateYLM(f.ry/360.f*2*PI);
  rz.loadRotateZLM(f.rz/360.f*2*PI);
  s.loadScale(f.sx, f.sy, f.sz);

  //this is probably right this way:
  //return t*rz*ry*rx*s; //scales seem to be local only
  return t*rz*ry*rx;

  //experimental:
  if(f.unknown == 0)
    return t*rx*ry*rz;
  else if(f.unknown == 1)
    return t*ry*rz*rx;
  else if(f.unknown == 2)
    return t*rz*ry*rx;
  else
    assert(false);
}

Matrix44f& mad(Matrix44f& r, const Matrix44f& m, float f)
{
  for(int j = 0; j < 3; ++j)
    for(int k = 0; k < 4; ++k)
      r[j][k] += f*m[j][k];
  return r;
}

Matrix44f localMatrix(int i, const BModel& bm)
{
  Matrix44f s;
  s.loadScale(bm.jnt1.frames[i].sx, bm.jnt1.frames[i].sy, bm.jnt1.frames[i].sz);

  //TODO: I don't know which of these two return values are the right ones
  //(if it's the first, then what is scale used for at all?)

  //looks wrong in certain circumstances...
  return bm.jnt1.matrices[i]; //this looks better with vf_064l.bdl (from zelda)
  return bm.jnt1.matrices[i]*s; //this looks a bit better with mario's bottle_in animation
}


void updateMatrixTable(const BModel& bmd, const Packet& currPacket, Matrix44f* matrixTable,
                       vector<bool>* isMatrixWeighted)
{
  for(size_t n = 0; n < currPacket.matrixTable.size(); ++n)
  {
    if(currPacket.matrixTable[n] != 0xffff) //this means keep old entry
    {
      u16 index = currPacket.matrixTable[n];
      if(bmd.drw1.isWeighted[index])
      {
        //TODO: the EVP1 data should probably be used here,
        //figure out how this works (most files look ok
        //without this, but models/ji.bdl is for example
        //broken this way)
        //matrixTable[n] = def;

        //the following _does_ the right thing...it looks
        //ok for all files, but i don't understand why :-P
        //(and this code is slow as hell, so TODO: fix this)

        //NO idea if this is right this way...
        Matrix44f m = Matrix44f::ZERO;
        const MultiMatrix& mm = bmd.evp1.weightedIndices[bmd.drw1.data[index]];
        for(size_t r = 0; r < mm.weights.size(); ++r)
        {
          const Matrix44f& sm1 = bmd.evp1.matrices[mm.indices[r]];
          const Matrix44f& sm2 = localMatrix(mm.indices[r], bmd);
          mad(m, sm2*sm1, mm.weights[r]);
        }
        m[3][3] = 1;

        matrixTable[n] = m;
        if(isMatrixWeighted != NULL)
          (*isMatrixWeighted)[n] = true;
      }
      else
      {
        matrixTable[n] = bmd.jnt1.matrices[bmd.drw1.data[index]];
        if(isMatrixWeighted != NULL)
          (*isMatrixWeighted)[n] = false;
      }
    }
  }
}

Vector3f operator*(const Matrix44f& m, const Vector3f& v)
{
  return Vector3f(
    m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2] + m[0][3],
    m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2] + m[1][3],
    m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2] + m[2][3]
    );
}

