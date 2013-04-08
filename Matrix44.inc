#include <cmath>
#include <algorithm> //swap

template <class Num>
Matrix44<Num>::Matrix44()
{
}

template <class Num>
Matrix44<Num>::Matrix44(Num _00, Num _01, Num _02, Num _03,
                   Num _10, Num _11, Num _12, Num _13,
                   Num _20, Num _21, Num _22, Num _23,
                   Num _30, Num _31, Num _32, Num _33)
{
  m[0][0] = _00; m[0][1] = _01; m[0][2] = _02; m[0][3] = _03;
  m[1][0] = _10; m[1][1] = _11; m[1][2] = _12; m[1][3] = _13;
  m[2][0] = _20; m[2][1] = _21; m[2][2] = _22; m[2][3] = _23;
  m[3][0] = _30; m[3][1] = _31; m[3][2] = _32; m[3][3] = _33;
}

template <class Num>
Matrix44<Num>::Matrix44(Num values[4][4])
{
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      m[i][j] = values[i][j];
}

template <class Num>
void Matrix44<Num>::loadIdentity()
{
  m[0][0] = (Num)1; m[0][1] = (Num)0; m[0][2] = (Num)0; m[0][3] = (Num)0;
  m[1][0] = (Num)0; m[1][1] = (Num)1; m[1][2] = (Num)0; m[1][3] = (Num)0;
  m[2][0] = (Num)0; m[2][1] = (Num)0; m[2][2] = (Num)1; m[2][3] = (Num)0;
  m[3][0] = (Num)0; m[3][1] = (Num)0; m[3][2] = (Num)0; m[3][3] = (Num)1;
}

template <class Num>
void Matrix44<Num>::flip()
{
  Num temp;

  temp = m[0][1]; m[0][1] = m[1][0]; m[1][0] = temp;
  temp = m[0][2]; m[0][2] = m[2][0]; m[2][0] = temp;
  temp = m[0][3]; m[0][3] = m[3][0]; m[3][0] = temp;

  temp = m[1][2]; m[1][2] = m[2][1]; m[2][1] = temp;
  temp = m[1][3]; m[1][3] = m[3][1]; m[3][1] = temp;

  temp = m[2][3]; m[2][3] = m[3][2]; m[3][2] = temp;
}

template <class Num>
Matrix44<Num> Matrix44<Num>::transpose() const
{
  Matrix44 ret = *this;
  ret.flip();
  return ret;
}

template <class Num>
Matrix44<Num> Matrix44<Num>::inverse() const
{
  Matrix44 ret = *this;
  if(gaussJordan(ret.m))
    return ret;
  else
    return ZERO;
}

template <class Num>
void Matrix44<Num>::loadTranslateRM(Num tx, Num ty, Num tz)
{
  loadIdentity();
  m[3][0] = tx;
  m[3][1] = ty;
  m[3][2] = tz;
}

template <class Num>
void Matrix44<Num>::loadTranslateLM(Num tx, Num ty, Num tz)
{
  loadIdentity();
  m[0][3] = tx;
  m[1][3] = ty;
  m[2][3] = tz;
}

template <class Num>
void Matrix44<Num>::loadRotateXRM(Num rad)
{
  loadIdentity();
  m[1][1] =  (Num)cos(rad);
  m[2][1] = -(Num)sin(rad);
  m[1][2] =  (Num)sin(rad);
  m[2][2] =  (Num)cos(rad);
}

template <class Num>
void Matrix44<Num>::loadRotateXLM(Num rad)
{
  loadIdentity();
  m[1][1] =  (Num)cos(rad);
  m[1][2] = -(Num)sin(rad);
  m[2][1] =  (Num)sin(rad);
  m[2][2] =  (Num)cos(rad);
}

template <class Num>
void Matrix44<Num>::loadRotateYRM(Num rad)
{
  loadIdentity();
  m[0][0] =  (Num)cos(rad);
  m[2][0] =  (Num)sin(rad);
  m[0][2] = -(Num)sin(rad);
  m[2][2] =  (Num)cos(rad);
}

template <class Num>
void Matrix44<Num>::loadRotateYLM(Num rad)
{
  loadIdentity();
  m[0][0] =  (Num)cos(rad);
  m[0][2] =  (Num)sin(rad);
  m[2][0] = -(Num)sin(rad);
  m[2][2] =  (Num)cos(rad);
}

template <class Num>
void Matrix44<Num>::loadRotateZRM(Num rad)
{
  loadIdentity();
  m[0][0] =  (Num)cos(rad);
  m[1][0] = -(Num)sin(rad);
  m[0][1] =  (Num)sin(rad);
  m[1][1] =  (Num)cos(rad);
}

template <class Num>
void Matrix44<Num>::loadRotateZLM(Num rad)
{
  loadIdentity();
  m[0][0] =  (Num)cos(rad);
  m[0][1] = -(Num)sin(rad);
  m[1][0] =  (Num)sin(rad);
  m[1][1] =  (Num)cos(rad);
}

template <class Num>
void Matrix44<Num>::loadScale(Num sx, Num sy, Num sz)
{
  loadIdentity();
  m[0][0] = sx;
  m[1][1] = sy;
  m[2][2] = sz;
}

template <class Num>
void Matrix44<Num>::loadScale(Num s)
{
  loadScale(s, s, s);
}

template <class Num>
Matrix44<Num> Matrix44<Num>::operator*(const Matrix44& b) const
{
  Matrix44 ret;
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      ret.m[i][j] = m[i][0]*b.m[0][j] + m[i][1]*b.m[1][j] + m[i][2]*b.m[2][j] + m[i][3]*b.m[3][j];

  return ret;
}

//checks if two matrices are exactly equal
template <class Num>
bool Matrix44<Num>::operator==(const Matrix44& b) const
{
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      if(m[i][j] != b.m[i][j])
        return false;

  return true;
}

template <class Num>
Matrix44<Num>::operator Num*()
{
  return &m[0][0];
}

template <class Num>
Matrix44<Num>::operator const Num*() const
{
  return &m[0][0];
}

template <class Num>
Num* Matrix44<Num>::operator[](int i)
{
  return m[i];
}

template <class Num>
const Num* Matrix44<Num>::operator[](int i) const
{
  return m[i];
}

template <class Num>
bool Matrix44<Num>::gaussJordan(Num matrix[4][4])
{
  const int rows = 4;
  int i; //loop counter
  int pivotRows[4];
  int pivotColumns[4];
  bool wasColumnUsed[4];
  for(i = 0; i < rows; ++i)
    wasColumnUsed[i] = false;

  for(int rowCount = 0; rowCount < rows; ++rowCount)
  {
    Num maxVal = (Num)0;
    int currPivotRow, currPivotColumn;
    //search for biggest number in matrix, use it as pivot
    for(i = 0; i < rows; ++i) //loop thru rows
    {
      if(wasColumnUsed[i]) //use only one pivot from each row
        continue;

      for(int j = 0; j < rows; ++j) //loop thru columns
      {
        if(wasColumnUsed[j]) //use only one pivot from each column
          continue;

        Num curr = (Num)fabs(matrix[j][i]);
        if(curr > maxVal)
        {
          maxVal = curr;
          currPivotRow = j;
          currPivotColumn = i;
        }
      }
    }

    if(wasColumnUsed[currPivotColumn])
      return false;
    wasColumnUsed[currPivotColumn] = true;

    //store which pivot was chosen in the rowCount'th run to reswap inverse matrix afterwards
    pivotRows[rowCount] = currPivotRow;
    pivotColumns[rowCount] = currPivotColumn;


    //swap rows to bring pivot on diagonal
    if(currPivotRow != currPivotColumn)
    {
      for(i = 0; i < rows; ++i) std::swap(matrix[currPivotRow][i], matrix[currPivotColumn][i]);
      currPivotRow = currPivotColumn;
    }

    if(matrix[currPivotRow][currPivotColumn] == (Num)0)
      return false;

    //start eliminating the pivot's column in each row
    Num inversePivot = ((Num)1)/matrix[currPivotRow][currPivotColumn];
    matrix[currPivotRow][currPivotColumn] = (Num)1;
    for(i = 0; i < rows; ++i) matrix[currPivotRow][i] *= inversePivot;

    //reduce non-pivot rows
    for(i = 0; i < rows; ++i)
    {
      if(i == currPivotRow) //pivot row is already reduced
        continue;

      Num temp = matrix[i][currPivotColumn];
      matrix[i][currPivotColumn] = (Num)0;
      for(int j = 0; j < rows; ++j) matrix[i][j] -= temp*matrix[currPivotRow][j];
    }
  }

  //unscramble inverse matrix
  for(i = rows - 1; i >= 0; --i)
  {
    if(pivotRows[i] != pivotColumns[i])
    {
      for(int j = 0; j < rows; ++j)
        std::swap(matrix[j][pivotRows[i]], matrix[j][pivotColumns[i]]);
    }
  }

  //done :-)!!

  return true;
}

template <class Num>
const Matrix44<Num> Matrix44<Num>::IDENTITY = Matrix44<Num>
                                 ((Num)1, (Num)0, (Num)0, (Num)0,
                                  (Num)0, (Num)1, (Num)0, (Num)0,
                                  (Num)0, (Num)0, (Num)1, (Num)0,
                                  (Num)0, (Num)0, (Num)0, (Num)1);
template <class Num>
const Matrix44<Num> Matrix44<Num>::ZERO = Matrix44<Num>
                                 ((Num)0, (Num)0, (Num)0, (Num)0,
                                  (Num)0, (Num)0, (Num)0, (Num)0,
                                  (Num)0, (Num)0, (Num)0, (Num)0,
                                  (Num)0, (Num)0, (Num)0, (Num)0);
