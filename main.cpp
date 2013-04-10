#include "simple_gl.h"
#include "simple_gl_common.h"

#include "openfile.h"
#include "bmdread.h"
#include "drawbmd.h"
#include "parameters.h"
#include "camera.h"

#include "oglblock.h"

#include "addons/bck.h"
#include "addons/btp.h"
#include "addons/export3ds.h"
#include "addons/exportTexture.h"

#include <cstdarg> //va_list

using namespace std;

extern GLuint g_fontBase;

static char *
_vasprintf (const char *format, va_list args)
{
  int size = vsnprintf(NULL, 0, format, args);
  char *buf = (char *) malloc(size + 1);
  buf[size - 1] = 0;
  vsprintf(buf, format, args);
  return buf;
}

void log(const char* msg, ...)
{
  va_list argList;
  va_start(argList, msg);
  char *str = _vasprintf(msg, argList);
  va_end(argList);
  fprintf(stderr, "log: %s\n", str);
  free(str);
}

void warn(const char* msg, ...)
{
  va_list argList;
  va_start(argList, msg);
  char *str = _vasprintf(msg, argList);
  va_end(argList);
  fprintf(stderr, "warn: %s\n", str);
  free(str);
}

string getExtension(const string& name)
{
  string::size_type pos = name.rfind(".");
  if(pos == string::npos)
    return "";

  return name.substr(pos + 1);
}

std::vector<Model> g_models(1);

void freeModel(Model& m)
{
  freeOglBlock(m.oglBlock);
  
  if(m.btp != NULL)
  {
    delete m.btp;
    m.btp = NULL;
  }

  if(m.bck != NULL)
  {
    delete m.bck;
    m.bck = NULL;
  }

  if(m.bmd != NULL)
  {
    delete m.bmd;
    m.bmd = NULL;
  }

  m.sceneGraph.children.clear();
}

void loadFile(const string& name, bool merge = false)
{
  printf("loading %s", name.c_str());

  OpenedFile* f = openFile(name);
  if(f == NULL)
    return;

  Model& m = g_models.back();

  //for now, recognize file content by extension
  string extension = getExtension(name);
  if(extension == "bmd" || extension == "bdl")
  {
    if(!merge)
    {
      for(size_t i = 0; i < g_models.size(); ++i)
        freeModel(g_models[i]);
      g_models.resize(1);
    }
    else
      g_models.push_back(Model());

    freeModel(g_models.back());
    g_models.back().bmd = loadBmd(f->f);

    if(g_models.back().bmd != NULL)
    {
      g_models.back().bmdFileName = name;

      uploadImagesToGl(g_models.back().bmd->tex1);
     
      g_models.back().oglBlock = createOglBlock(g_models.back().bmd->mat3, name);
      g_models.back().sceneGraph.children.clear();
      buildSceneGraph(g_models.back().bmd->inf1, g_models.back().sceneGraph);

      //hack: this computes the matrices for the first time.
      //without this call, the screen contains garbage for
      //a short time, because right now, the matrices from
      //the last frame are used to draw the next frame
      //(it's even uglier: a mixture of this frame's and
      //last frame's matrices are used right now -> TODO)
      drawBmd(g_models.back(), g_models.back().sceneGraph);
    }
  }
  else if(extension == "bck")
  {
    if(m.bck != NULL) delete m.bck;
    m.bck = readBck(f->f);
  }
  else if(extension == "btp")
  {
    if(m.btp != NULL) delete m.btp;
    m.btp = readBtp(f->f);
  }

  closeFile(f);
}

bool init()
{
  glEnable(GL_LIGHT0);
  glEnable(GL_RESCALE_NORMAL);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glClearColor(77/255.f, 50/255.f, 153/255.f, 1.f);

  walkBack(10*128);

  for(int i = 0; i < getParameterCount(); ++i)
    loadFile(getParameter(i));

  return true;
}

float g_time = 0.f;

void draw()
{
  prepareCamera();

  glDepthMask(GL_TRUE); //if this was set to false, depth is not cleared...
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLenum error;
  while((error = glGetError()) != GL_NO_ERROR)
    fprintf(stderr, "GL error: %s\n", gluErrorString(error));

  for(size_t i = 0; i < g_models.size(); ++i)
  {
    Model& m = g_models[i];

    //update animations:
    g_time += getLastFrameSeconds();
    float animTime = 256*g_time; //TODO: find the right factor for this
    if(m.bmd != NULL)
    {
      if(m.bck != NULL)
      {
        if(m.bck->anims.size() != m.bmd->jnt1.frames.size())
          warn("number of joints in anim (%d) doesn't match number of joints of model (%d)",
            m.bck->anims.size(), m.bmd->jnt1.frames.size());
        else
          animate(*m.bck, m.bmd->jnt1, animTime);
      }

      if(m.btp != NULL)
        animate(*m.btp, m.bmd->mat3, animTime);
    }

    if(m.bmd != NULL)
      drawBmd(m, m.sceneGraph);
  }

  flush();
}

void exit()
{
  for(size_t i = 0; i < g_models.size(); ++i)
    freeModel(g_models[i]);
  g_models.clear();
}
