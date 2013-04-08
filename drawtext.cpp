#include "drawtext.h"

#include "GL/glew.h"

#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> // vsnprintf
#include <vector>
#include <utility>
using namespace std;

struct TextColor
{
  float r, g, b, a;
};

static vector< pair<string, TextColor> > g_strings, g_staticStrings;
static float g_textColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static float g_fpsColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

static bool g_reverseText = false, g_alphaEffect = false;
static bool g_wordWrap = true, g_drawFps = false;

void setTextColor3f(float r, float g, float b)
{
  g_textColor[0] = r;
  g_textColor[1] = g;
  g_textColor[2] = b;
  g_textColor[3] = 1.0f;
}

void setTextColor4f(float r, float g, float b, float a)
{
  g_textColor[0] = r;
  g_textColor[1] = g;
  g_textColor[2] = b;
  g_textColor[3] = a;
}

void setTextColor3fv(float* vals)
{
  g_textColor[0] = vals[0];
  g_textColor[1] = vals[1];
  g_textColor[2] = vals[2];
  g_textColor[3] = 1.0f;
}

void setTextColor4fv(float * vals)
{
  g_textColor[0] = vals[0];
  g_textColor[1] = vals[1];
  g_textColor[2] = vals[2];
  g_textColor[3] = vals[3];
}

void setTextParameterb(TEXT_PARAMETER what, bool b)
{
  switch(what)
  {
    case TEXT_REVERSE_ORDER:
      g_reverseText = b;
      break;
    case TEXT_ALPHA_EFFECT:
      g_alphaEffect = b;
      break;
    case TEXT_WORD_WRAP:
      g_wordWrap = b;
      break;
  }
}

void setFpsColor3f(float r, float g, float b)
{
  g_fpsColor[0] = r;
  g_fpsColor[1] = g;
  g_fpsColor[2] = b;
  g_fpsColor[3] = 1.0f;
}

void setFpsColor4f(float r, float g, float b, float a)
{
  g_fpsColor[0] = r;
  g_fpsColor[1] = g;
  g_fpsColor[2] = b;
  g_fpsColor[3] = a;
}

void setFpsColor3fv(float* vals)
{
  g_fpsColor[0] = vals[0];
  g_fpsColor[1] = vals[1];
  g_fpsColor[2] = vals[2];
  g_fpsColor[3] = 1.0f;
}

void setFpsColor4fv(float * vals)
{
  g_fpsColor[0] = vals[0];
  g_fpsColor[1] = vals[1];
  g_fpsColor[2] = vals[2];
  g_fpsColor[3] = vals[3];
}

void showFps(bool b)
{
  g_drawFps = b;
}

string format(const char* s, va_list argList)
{
#ifdef _MSC_VER
  #define vsnprintf _vsnprintf
#endif
  string str;
  const int startSize = 161;
  int currSize, maxSize = 1000000;
  char buff1[startSize];
  currSize = startSize - 1;
  int ret = vsnprintf(buff1, currSize, s, argList);
  if(ret >= 0 && ret <= startSize)
    str = string(buff1);
  else
  {
    char* buff2 = NULL;
    while((ret < 0 || ret > currSize + 1)&& currSize < maxSize)
    {
      currSize *= 2;
      buff2 = (char*)realloc(buff2, currSize + 1);
      ret = vsnprintf(buff2, currSize, s, argList);
    }
    str = string(buff2);
  }
  return str;
}

void drawText(const char* s, ...)
{
  //cf. flipcode, morgan mcguires format() cotd

  va_list argList;
  va_start(argList, s);
  string str = format(s, argList);
  va_end(argList);

  TextColor col = { g_textColor[0], g_textColor[1], g_textColor[2], g_textColor[3] };
  g_strings.push_back(make_pair(str, col));
}


void addText(const char* s, ...)
{
  //cf. flipcode, morgan mcguires format() cotd

  va_list argList;
  va_start(argList, s);
  string str = format(s, argList);
  va_end(argList);

  TextColor col = { g_textColor[0], g_textColor[1], g_textColor[2], g_textColor[3] };
  g_staticStrings.push_back(make_pair(str, col));
}

void renderTexts(int w, int h, int fontWidth, int fontHeight, GLuint fontList,
              float fps)
{
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, w - 1, h - 1, 0.0f, 1.0f, -1.0f);
  if(g_alphaEffect)
  {
    glPushAttrib(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  vector< pair<string, TextColor> > strings;
  strings.resize(g_strings.size() + g_staticStrings.size());
  copy(g_staticStrings.begin(), g_staticStrings.end(), strings.begin());
  copy(g_strings.begin(), g_strings.end(), strings.begin() + g_staticStrings.size());
  g_strings.clear();

  int s = strings.size();
  int line = 1, col = 0;
  float currAlpha = 1.0f, stepAlpha = 1.0f/s;
  if(!g_reverseText)
  {
    currAlpha = stepAlpha;
    stepAlpha = -stepAlpha;
  }

  for(int i = 0; i < s; ++i)
  {
    int index = i;
    if(g_reverseText)
      index = s - 1 - i;
    if(g_alphaEffect)
      glColor4f(strings[index].second.r, strings[index].second.g, strings[index].second.b, currAlpha);
    else
      glColor4f(strings[index].second.r, strings[index].second.g, strings[index].second.b, strings[index].second.a);
    glRasterPos2f(10.0f, line*fontHeight);
    string& curr = strings[index].first;

    col = 0;
    int l = curr.length();
    for(int j = 0; j < l; ++j)
    {
      unsigned char currChar = (unsigned char)curr[j];
      if(currChar == '\n')
      {
        ++line;
        col = 0;
        glRasterPos2f(0.0f, line*fontHeight);
      }
      else
      {
        col += fontWidth;
        if(col > w && g_wordWrap)
        {
          ++line;
          col = 0;
          glRasterPos2f(0.0f, line*fontHeight);
        }
        glCallList(currChar + fontList);
      }
    }
    ++line;
    currAlpha -= stepAlpha;
  }

  if(g_drawFps)
  {
    char buff[20];
    sprintf(buff, "FPS: %f", fps);
    glColor4fv(g_fpsColor);
    glRasterPos2f(5, h - 1 - 5);
    glListBase(fontList);
    glCallLists(strlen(buff), GL_UNSIGNED_BYTE, buff);
  }

  glPopAttrib();

  if(g_alphaEffect)
    glPopAttrib();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}
