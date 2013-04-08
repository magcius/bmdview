#ifndef NICO_MATRIX44_INC
#define NICO_MATRIX44_INC NICO_MATRIX44_INC

template <class Num>
class Matrix44
{
 public:
  //standard-constructor keeps matrix uninitialized for efficiency
  Matrix44();
  Matrix44(Num _00, Num _01, Num _02, Num _03,
           Num _10, Num _11, Num _12, Num _13,
           Num _20, Num _21, Num _22, Num _23,
           Num _30, Num _31, Num _32, Num _33);
  Matrix44(Num values[4][4]);

  void loadIdentity();
  void flip();
  Matrix44<Num> transpose() const;

  //returns Matrix44::ZERO if no inverse exists
  Matrix44<Num> inverse() const;

  void loadTranslateRM(Num tx, Num ty, Num tz);
  void loadTranslateLM(Num tx, Num ty, Num tz);

  void loadRotateXRM(Num rad);
  void loadRotateXLM(Num rad);

  void loadRotateYRM(Num rad);
  void loadRotateYLM(Num rad);

  void loadRotateZRM(Num rad);
  void loadRotateZLM(Num rad);

  void loadScale(Num sx, Num sy, Num sz);
  void loadScale(Num s);


  Matrix44<Num> operator*(const Matrix44& b) const;

  //checks if two matrices are exactly equal
  bool operator==(const Matrix44& b) const;

  operator Num*();
  operator const Num*() const;

  Num* operator[](int i);
  const Num* operator[](int i) const;

  static const Matrix44<Num> IDENTITY;
  static const Matrix44<Num> ZERO;

 private:
  static bool gaussJordan(Num matrix[4][4]);
  Num m[4][4];
};

#include "Matrix44.inc"

typedef Matrix44<float> Matrix44f;
typedef Matrix44<double> Matrix44d;


#endif //NICO_MATRIX44_INC
