#ifndef BMD_BMDREAD_H
#define BMD_BMDREAD_H BMD_BMDREAD_H

#include <vector>
#include <string>
#include <iosfwd>
#include "Vector3.h"
#include "common.h"


#include "inf1.h"
#include "vtx1.h"
#include "evp1.h"
#include "drw1.h"
#include "jnt1.h"
#include "shp1.h"
#include "mat3.h"
#include "tex1.h"

//only in zelda files
#include "mdl3.h"


struct BModel
{
  Inf1 inf1;
  Vtx1 vtx1;
  Drw1 drw1;
  Evp1 evp1;
  Jnt1 jnt1;
  Shp1 shp1;
  Mat3 mat3;
  Tex1 tex1;
};

BModel* loadBmd(FILE* f);
void writeBmdInfo(FILE* f, std::ostream& out);

#endif //BMD_BMDREAD_H
