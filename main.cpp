#include "simple_gl.h"
#include "simple_gl_common.h"

#include "openfile.h"
#include "bmdread.h"
#include "drawbmd.h"
#include "drawtext.h"
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

void drawString(const Vector3f& p, const char* s, ...)
{
  va_list argList;
  va_start(argList, s);
  string str = format(s, argList);
  va_end(argList);

  glColor3f(0.f, 1.f, 0.f);
  glRasterPos3fv((float*)&p);

  for(size_t i = 0; i < str.size(); ++i)
    glCallList(str[i] + g_fontBase);
}


void drawCoordFrame()
{
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_FOG);
  glDisable(GL_BLEND);

  glBegin(GL_LINES);

     // x-Achse
     glColor3f(1.0f, 0.0f, 0.0f);
     glVertex3f(0.0f, 0.0f, 0.0f);
     glVertex3f(1.0f, 0.0f, 0.0f);

     // y-Achse
     glColor3f(0.0f, 1.0f, 0.0f);
     glVertex3f(0.0f, 0.0f, 0.0f);
     glVertex3f(0.0f, 1.0f, 0.0f);

     // z-Achse
     glColor3f(0.0f, 0.0f, 1.0f);
     glVertex3f(0.0f, 0.0f, 0.0f);
     glVertex3f(0.0f, 0.0f, 1.0f);

  glEnd();

  glPopAttrib();
}

void log(const char* msg, ...)
{
  va_list argList;
  va_start(argList, msg);
  string str = format(msg, argList);
  va_end(argList);

  setTextColor3f(1, 1, 1);
  addText("%s", str.c_str());
}

void warn(const char* msg, ...)
{
  va_list argList;
  va_start(argList, msg);
  string str = format(msg, argList);
  va_end(argList);

  setTextColor3f(1, 0, 0);
  addText("%s", str.c_str());
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
  setTextColor3f(1.f, 1.f, 1.f);
  addText("%s", name.c_str());

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

      setStartupText("Creating OpenGL block (mainly shaders)...");
      uploadImagesToGl(g_models.back().bmd->tex1);
      
      g_models.back().oglBlock = createOglBlock(g_models.back().bmd->mat3, name);

      setStartupText("Building Scenegraph...");
      g_models.back().sceneGraph.children.clear();
      buildSceneGraph(g_models.back().bmd->inf1, g_models.back().sceneGraph);

      //hack: this computes the matrices for the first time.
      //without this call, the screen contains garbage for
      //a short time, because right now, the matrices from
      //the last frame are used to draw the next frame
      //(it's even uglier: a mixture of this frame's and
      //last frame's matrices are used right now -> TODO)
      setStartupText("Setting up matrices...");
      drawBmd(g_models.back(), g_models.back().sceneGraph);
    }


    //debugging stuff:

    //if(g_model != NULL)
      //saveAsOff(name + ".off", *g_model);

    //if(g_model != NULL)
    //  exportAsX(*g_model, name + ".x");

	//if(g_model != NULL)
    //  exportAs3ds(*g_model, name + ".3ds");

    /*
    if(g_model != NULL)
    {
      for(int i = 0; i < g_model->tex1.imageHeaders.size(); ++i)
        saveTexture(DDS, g_model->tex1.imageHeaders[i].name,
                    *g_model->tex1.imageHeaders[i].data);
    }
    */
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
  {
    setStartupText("Loading " + getParameter(i) + "...");
    loadFile(getParameter(i));
  }

  setStartupText("Ready.");

  showFps();

  return true;
}

float g_time = 0.f;

void draw()
{
  prepareCamera();

  glDepthMask(GL_TRUE); //if this was set to false, depth is not cleared...
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 // drawCoordFrame();

  GLenum error;
  while((error = glGetError()) != GL_NO_ERROR)
    addText("GL error: %s", gluErrorString(error));

  setTextColor3f(1.f, .5f, 1.f);
  if(!isKeyPressed(BKEY_F1))
    drawText("F1: show controls");
  else
  {
    drawText("W: wireframe");
    drawText("T: scene graph (with shift: hide joints)");
    drawText("B: bones");
    drawText("C: don't cull");
    drawText("A: don't alphatest");
    drawText("Q: don't blend");
    drawText("E: color weighted vertices [green: multimatrix, red: weighted, white: normal]");
    drawText("H: hide model (use with B)");
    drawText("G: disable glsl shaders");
    drawText("1: Show vertex colors only (ie, no textures)");
    drawText("2: Ignore vertex colors");
    drawText("9: Show batch bounding boxes");
    drawText("0: Show joint bounding boxes");
    drawText("X: disable billboard transforms");
    drawText("F2: Show OpenGL info");
  }

  if(isKeyPressed(BKEY_F2))
  {
    setTextColor3f(.5f, 1.f, 1.f);
    drawText("GL Vendor: %s", glGetString(GL_VENDOR));
    drawText("GL Renderer: %s", glGetString(GL_RENDERER));
    drawText("GL Version: %s", glGetString(GL_VERSION));
    drawText("GL Extensions: %s", glGetString(GL_EXTENSIONS));
  }

  glPolygonMode(GL_FRONT_AND_BACK, isKeyPressed('W')?GL_LINE:GL_FILL);

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
          drawText("number of joints in anim (%d) doesn't match number of joints of model (%d)",
            m.bck->anims.size(), m.bmd->jnt1.frames.size());
        else
          animate(*m.bck, m.bmd->jnt1, animTime);
      }

      if(m.btp != NULL)
        animate(*m.btp, m.bmd->mat3, animTime);
    }
  

    glColor3f(1, 1, 1);
    glPointSize(5);
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
