#ifndef NICO_VECTOR3_INC
#define NICO_VECTOR3_INC NICO_VECTOR3_INC

template <class Num>
class Vector3
{
 public:
  //standard constructor leaves vector uninitialized
  //for efficiency
  Vector3();
  Vector3(const Num& x, const Num& y, const Num& z);
  Vector3(const Num v[]);

  Num& operator[](int i);
  const Num& operator[](int i) const;

  Num getLength() const;
  Num getLengthSquared() const;
  Vector3 normalized() const;
  void normalize();

  Vector3<Num> cross(const Vector3<Num>& b) const;
  Num dot(const Vector3<Num>& b) const;

  void setX(const Num& x);
  void setY(const Num& y);
  void setZ(const Num& z);
  void setXYZ(const Num& x, const Num& y, const Num& z);

  Num getX() const;
  Num getY() const;
  Num getZ() const;

  Num& x();
  Num& y();
  Num& z();

  const Num& x() const;
  const Num& y() const;
  const Num& z() const;

  Vector3<Num> operator+(const Vector3& b) const;
  Vector3<Num> operator-(const Vector3& b) const;
  Vector3<Num> operator-() const;
  Vector3<Num> operator*(const Num& b) const;
  Vector3<Num> operator/(const Num& b) const;

  template <class T>
  friend Vector3<T> operator*(const T& a, const Vector3<T>& b);

  Vector3<Num>& operator +=(const Vector3& b);
  Vector3<Num>& operator -=(const Vector3& b);
  Vector3<Num>& operator *=(const Num& s);
  Vector3<Num>& operator /=(const Num& b);

  operator Num*();
  operator const Num*() const;

 private:
  Num v[3];
};

#include "Vector3.inc"

typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;

#endif //NICO_VECTOR3_INC
