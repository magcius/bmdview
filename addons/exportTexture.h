#ifndef EXPORT_TEXTURE_H
#define EXPORT_TEXTURE_H EXPORT_TEXTURE_H

#include <string>
#include "../bmdread.h"

enum IMAGETYPE
{
  DDS,
  TGA
};

void saveTexture(IMAGETYPE imgType, const Image& img,
                 const std::string& filename, bool mirrorX = false,
                 bool mirrorY = false);

void exportTextures(IMAGETYPE imgType, const Tex1& tex,
                    const std::string& basename);

#endif //EXPORT_TEXTURE_H
