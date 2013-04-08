#include "ui.h"

#include <string>
#include <sstream>

#include "addons/export3ds.h"
#include "addons/exportTexture.h"

#include "drawbmd.h"
#include "oglblock.h"

#include <iostream>

#ifdef _WIN32

#include <windows.h>
#undef min
#undef max

#include "textbox.h"


//TODO: this is ugly, do this somehow else
void loadFile(const std::string& name, bool merge = false);
extern HWND getHWnd(); //in simple_gl.cpp

#endif

#include "main.h"

#ifdef _WIN32

void unimplemented(const std::string& str)
{
  MessageBox(getHWnd(), str.c_str(), "Unimplemeted:", MB_OK);
}

std::string getOpenFilename(const std::string& title = "Open",
                            const char* filter = "All Files\0*.*\0")
{
  static bool s_isInitialized = false;
  static OPENFILENAME s_openFileName;
  static char buffer[MAX_PATH];
  if(!s_isInitialized)
  {
    ZeroMemory(&s_openFileName, sizeof(s_openFileName));
    s_openFileName.lStructSize = sizeof(s_openFileName);
    s_openFileName.hwndOwner = getHWnd();
    s_openFileName.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
    s_openFileName.lpstrFile = buffer;
    s_openFileName.nMaxFile = MAX_PATH;
    s_openFileName.lpstrDefExt = "XXX"; //this way extensions are appended
    s_isInitialized = true;
  }

  s_openFileName.lpstrTitle = title.c_str();
  s_openFileName.lpstrFilter = filter;

  if(GetOpenFileName(&s_openFileName))
  {
    return s_openFileName.lpstrFile;
  }
  else
    return "";
}

std::string getSaveFilename(const std::string& title = "Save",
                            const char* filter = "All Files\0*.*\0")
{
  static bool s_isInitialized = false;
  static OPENFILENAME s_saveFileName;
  static char buffer[MAX_PATH];
  if(!s_isInitialized)
  {
    ZeroMemory(&s_saveFileName, sizeof(s_saveFileName));
    s_saveFileName.lStructSize = sizeof(s_saveFileName);
    s_saveFileName.hwndOwner = getHWnd();
    s_saveFileName.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER;
    s_saveFileName.lpstrFile = buffer;
    s_saveFileName.nMaxFile = MAX_PATH;
    s_saveFileName.lpstrDefExt = "XXX"; //this way extensions are appended
    s_isInitialized = true;
  }

  s_saveFileName.lpstrTitle = title.c_str();
  s_saveFileName.lpstrFilter = filter;

  if(GetSaveFileName(&s_saveFileName))
  {
    return s_saveFileName.lpstrFile;
  }
  else
    return "";
}

void menuFileOpenModel()
{
  std::string name = getOpenFilename("Open model",
    "BModel (*.bmd, *.bdl)\0*.bmd;*.bdl\0");
  if(name != "")
    loadFile(name);
}

void menuFileMergeModel()
{
  std::string name = getOpenFilename("Merge model",
    "BModel (*.bmd, *.bdl)\0*.bmd;*.bdl\0");
  if(name != "")
    loadFile(name, true);
}

void menuFileOpenAnimation()
{
  std::string name = getOpenFilename("Open animation",
    "All animations (*.bck, *.btp)\0*.bck;*.btp\0"
    "Bone animation (*.bck)\0*.bck\0"
    "Texture animation (*.btp)\0*.btp\0");
  if(name != "")
    loadFile(name);
}

void menuFileExportModel()
{
  std::string name = getSaveFilename("Export model", "3ds files\0*.3ds\0");
  if(name != "")
    menuFileExportModel(name);
}

void menuFileExportTextures()
{
  std::string filename = getSaveFilename("Enter textures basename",
    "dds files\0*.dds\0tga files\0*.tga\0");
  if(filename != "")
    menuFileExportTextures(filename);
}

#endif

void menuFileExportModel(const std::string& filename)
{
  if(g_models.back().bmd != NULL)
    exportAs3ds(*g_models.back().bmd, filename);
  else
    warn("Failed to export %s, no model loaded", filename.c_str());
}

void menuFileExportTextures(const std::string& filename)
{
  if(g_models.back().bmd != NULL)
  {
    std::string name, extension;
    splitName(filename, name, extension);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   tolower);

    if(extension == "dds")
      exportTextures(DDS, g_models.back().bmd->tex1, name);
    else if(extension == "tga")
      exportTextures(TGA, g_models.back().bmd->tex1, name);
    else
      warn("Failed to export textures, unknown extension \"%s\"",
           extension.c_str());

  }
  else
    warn("Failed to export textures to \"%s\", no model loaded",
         filename.c_str());
}

void menuFileExportShaders()
{
  if(g_models.back().oglBlock != NULL)
    saveShaderStrings(*g_models.back().oglBlock, g_models.back().bmdFileName);
}

void menuFileReimportShaders()
{
  if(g_models.back().oglBlock != NULL)
    loadShaderStrings(*g_models.back().oglBlock, g_models.back().bmdFileName);
}

void menuFileRegenerateShaders()
{
  if(g_models.back().oglBlock != NULL)
    generateShaderStrings(*g_models.back().oglBlock,
      g_models.back().bmd->mat3);
}


void menuDebugSectioninfo()
{
  if(g_models.back().oglBlock != NULL) //model loaded?
  {
    FILE* f = fopen(g_models.back().bmdFileName.c_str(), "rb");

    if(f != NULL)
    {
      std::ostringstream out;
      writeBmdInfo(f, out);
      fclose(f);

#ifdef _WIN32
      TextBox(getHWnd(), out.str().c_str(), "Bmd sections",
        MB_OK | TB_USEFIXEDFONT);
#else
      std::cout << out.str();
#endif
    }
  }
}

