#ifndef EXPORT_3DS_H
#define EXPORT_3DS_H EXPORT_3DS_H

#include <string>
#include "../bmdread.h"

//exports a BModel to a .3ds file
void exportAs3ds(const BModel& bmd, const std::string& filename);

#endif //EXPORT_3DS_H
