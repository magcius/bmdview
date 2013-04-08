#include "Vector3.h"

#include <cmath>
#include <iostream>

template <class Num>
Vector3<Num>::Vector3()
{
}

template <class Num>
Vector3<Num>::Vector3(const Num& x, const Num& y, const Num& z)
{
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

template <class Num>
Vector3<Num>::Vector3(const Num iv[])
{
  v[0] = iv[0];
  v[1] = iv[1];
  v[2] = iv[2];
}

template <class Num>
Num& Vector3<Num>::operator[](int i)
{
  return v[i];
}

template <class Num>
const Num& Vector3<Num>::operator[](int i) const
{
  return v[i];
}

template <class Num>
Num Vector3<Num>::getLength() const
{
  return (Num)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

template <class Num>
Num Vector3<Num>::getLengthSquared() const
{
  return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

template <class Num>
Vector3<Num> Vector3<Num>::normalized() const
{
  Num invLen = ((Num)1)/getLength();
  return Vector3<Num>(v[0]*invLen, v[1]*invLen, v[2]*invLen);
}

template <class Num>
void Vector3<Num>::normalize()
{
  Num invLen = ((Num)1)/getLength();
  v[0] *= invLen;
  v[1] *= invLen;
  v[2] *= invLen;
}

template <class Num>
Vector3<Num> Vector3<Num>::cross(const Vector3& b) const
{
  return Vector3<Num>(v[1]*b.v[2] - v[2]*b.v[1], v[2]*b.v[0] - v[0]*b.v[2], v[0]*b.v[1] - v[1]*b.v[0]);
}

template <class Num>
Num Vector3<Num>::dot(const Vector3& b) const
{
  return v[0]*b.v[0] + v[1]*b.v[1] + v[2]*b.v[2];
}

template <class Num>
void Vector3<Num>::setX(const Num& x)
{
  v[0] = x;
}

template <class Num>
void Vector3<Num>::setY(const Num& y)
{
  v[1] = y;
}

template <class Num>
void Vector3<Num>::setZ(const Num& z)
{
  v[2] = z;
}

template <class Num>
void Vector3<Num>::setXYZ(const Num& x, const Num& y, const Num& z)
{
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

template <class Num>
Num Vector3<Num>::getX() const
{
  return v[0];
}

template <class Num>
Num Vector3<Num>::getY() const
{
  return v[1];
}

template <class Num>
Num Vector3<Num>::getZ() const
{
  return v[2];
}

template <class Num>
Num& Vector3<Num>::x()
{
  return v[0];
}

template <class Num>
Num& Vector3<Num>::y()
{
  return v[1];
}

template <class Num>
Num& Vector3<Num>::z()
{
  return v[2];
}

template <class Num>
const Num& Vector3<Num>::x() const
{
  return v[0];
}

template <class Num>
const Num& Vector3<Num>::y() const
{
  return v[1];
}

template <class Num>
const Num& Vector3<Num>::z() const
{
  return v[2];
}

template <class Num>
Vector3<Num> Vector3<Num>::operator+(const Vector3& b) const
{
  return Vector3<Num>(v[0] + b.v[0], v[1] + b.v[1], v[2] + b.v[2]);
}

template <class Num>
Vector3<Num> Vector3<Num>::operator-(const Vector3& b) const
{
  return Vector3<Num>(v[0] - b.v[0], v[1] - b.v[1], v[2] - b.v[2]);
}

template <class Num>
Vector3<Num> Vector3<Num>::operator-() const
{
  return Vector3<Num>(-v[0], -v[1], -v[2]);
}

template <class Num>
Vector3<Num> Vector3<Num>::operator*(const Num& b) const
{
  return Vector3<Num>(v[0]*b, v[1]*b, v[2]*b);
}

template <class Num>
Vector3<Num> Vector3<Num>::operator/(const Num& b) const
{
  Num i = ((Num)1)/b;
  return Vector3<Num>(v[0]*i, v[1]*i, v[2]*i);
}

template <class T>
Vector3<T> operator*(const T& a, const Vector3<T>& b)
{
  return b*a;
}

template <class Num>
Vector3<Num>& Vector3<Num>::operator +=(const Vector3& b)
{
  v[0] += b.v[0];
  v[1] += b.v[1];
  v[2] += b.v[2];

  return *this;
}

template <class Num>
Vector3<Num>& Vector3<Num>::operator -=(const Vector3& b)
{
  v[0] -= b.v[0];
  v[1] -= b.v[1];
  v[2] -= b.v[2];

  return *this;
}

template <class Num>
Vector3<Num>& Vector3<Num>::operator *=(const Num& s)
{
  v[0] *= s;
  v[1] *= s;
  v[2] *= s;

  return *this;
}

template <class Num>
Vector3<Num>& Vector3<Num>::operator /=(const Num& s)
{
  Num i = ((Num)1)/s;
  v[0] *= i;
  v[1] *= i;
  v[2] *= i;

  return *this;
}

template <class Num>
Vector3<Num>::operator Num*()
{
  return &v[0];
}

template <class Num>
Vector3<Num>::operator const Num*() const
{
  return &v[0];
}

