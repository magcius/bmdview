#include "simple_gl_common.h"

#include <cmath>

#include "GL/glew.h"

#include "common.h" //PI
#include "camera.h"
#include "clock.h"

#include "simple_gl.h"

static float g_transSpeed = 5.0f;
static float g_rotSpeed = PI/4.0f;
static float g_lastFrameSeconds = 0.f;
static Clock<20> g_clock;


void nglPerspective(float fovy, float aspect, float z1, float z2)
{
  fovy *= 0.5f; //use only half the angle because interval
                //goes from -u to u (2*u)
  fovy *= PI/180; //convert to radians

  float u = z1*(float)tan(fovy);
  float r = u*aspect;
  glFrustum(-r, r, -u, u, z1, z2);
}

void updateProjectionMatrix(int w, int h)
{
  float aspect = float(w)/float(h);
  
  //initialize matrices and viewport
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float zNear = 0.1f*128, zFar = 2500.0f*128;
  if(w > h)
    nglPerspective(2*22.5f, aspect, zNear, zFar);
  else
  {
    //calculate fovy to keep fovx constant
    float fovy = (float)atan(tan(22.5f*PI/180)/aspect)*(180/PI);
    nglPerspective(2*fovy, aspect, zNear, zFar);
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, w, h);
}

void handleCamera()
{
  float walkSpeed = g_transSpeed*getLastFrameSeconds();
  if(isKeyPressed(BKEY_SHIFT))
    walkSpeed *= 10.0f;
  if(isKeyPressed(BKEY_CONTROL))
    walkSpeed /= 100.0f;

  walkSpeed *= 128;

  if(isKeyPressed(BKEY_LEFT))
    walkLeft(walkSpeed);
  if(isKeyPressed(BKEY_RIGHT))
    walkRight(walkSpeed);
    
  if(isKeyPressed(BKEY_UP) && isKeyPressed(BKEY_ALT))
    walkUp(walkSpeed);
  else
  {
    if(isKeyPressed(BKEY_UP))
      walkForward(walkSpeed);
    if(isKeyPressed(BKEY_PG_UP))
      walkUp(walkSpeed);
  }
  
  if(isKeyPressed(BKEY_DOWN) && isKeyPressed(BKEY_ALT))
    walkDown(walkSpeed);
  else
  {
    if(isKeyPressed(BKEY_DOWN))
      walkBack(walkSpeed);
    if(isKeyPressed(BKEY_PG_DOWN))
      walkDown(walkSpeed);
  }
  
  float rotSpeed = g_rotSpeed*getLastFrameSeconds();

  if(isKeyPressed('J'))
    turnLeft(rotSpeed);
  if(isKeyPressed('L'))
    turnRight(rotSpeed);
  if(isKeyPressed('I'))
    turnUp(rotSpeed);
  if(isKeyPressed('K'))
    turnDown(rotSpeed);
  if(isKeyPressed('U'))
    turnCCW(rotSpeed);
  if(isKeyPressed('O'))
    turnCW(rotSpeed);
}

void setLastFrameSeconds(float s)
{
  g_clock.addValue(s);
  g_lastFrameSeconds = s;
}

float getLastFrameSeconds()
{
  return g_lastFrameSeconds;
}

float getAverageSecondsPerFrame()
{
  return g_clock.getAverage();
}
