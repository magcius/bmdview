#ifndef BMD_UI_H
#define BMD_UI_H BMD_UI_H

#include <string>

void menuFileOpenModel();
void menuFileMergeModel();
void menuFileOpenAnimation();
void menuFileExportModel();
void menuFileExportModel(const std::string& filename);
void menuFileExportTextures();
void menuFileExportTextures(const std::string& filename);
void menuFileExportShaders();
void menuFileReimportShaders();
void menuFileRegenerateShaders();

void menuDebugSectioninfo();

#endif //BMD_UI_H
