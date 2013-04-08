#ifndef EXPORT_B3D_H
#define EXPORT_B3D_H EXPORT_B3D_H

#include <string>
#include "../bmdread.h"

#include "bck.h"

//exports a BModel to a blitz3d .b3d file.
//b3d supports bone animation - yay. texture animations are not supported,
//though. anim may be NULL, in this case no animation is written.
void exportAsB3d(const BModel& bmd, const Bck* anim,
                 const std::string& filename);

#endif //EXPORT_B3D_H
