#ifndef BMD_TRANSFORMTOOLS_H
#define BMD_TRANSFORMTOOLS_H

#include <vector>

#include "Matrix44.h"
#include "Vector3.h"
#include "bmdread.h"
#include "shp1.h"
#include "jnt1.h"

Matrix44f frameMatrix(const Frame& f);
void updateMatrixTable(const BModel& bmd, const Packet& currPacket,
                       Matrix44f* matrixTable,
                       std::vector<bool>* isMatrixWeighted = NULL);
Vector3f operator*(const Matrix44f& m, const Vector3f& v);

#endif //BMD_TRANSFORMTOOLS_H
