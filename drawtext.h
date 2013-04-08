#ifndef BMD_DRAWTEXT_H
#define BMD_DRAWTEXT_H BMD_DRAWTEXT_H

#include "GL/glew.h"

#include <string>
#include <cstdarg>

std::string format(const char* s, va_list argList);

void setTextColor3f(float r, float g, float b);
void setTextColor4f(float r, float g, float b, float a);
void setTextColor3fv(float* vals);
void setTextColor4fv(float * vals);
void drawText(const char* s, ...);
void addText(const char* s, ...);

enum TEXT_PARAMETER
{ TEXT_REVERSE_ORDER, TEXT_ALPHA_EFFECT, TEXT_WORD_WRAP };
void setTextParameterb(TEXT_PARAMETER what, bool b);

void setFpsColor3f(float r, float g, float b);
void setFpsColor4f(float r, float g, float b, float a);
void setFpsColor3fv(float* vals);
void setFpsColor4fv(float * vals);
void showFps(bool b = true);

void renderTexts(int w, int h, int fontWidth, int fontHeight, GLuint fontList,
              float fps);

#endif //BMD_DRAWTEXT_H
