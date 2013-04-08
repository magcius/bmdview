#include "camera.h"

#include "GL/glew.h"

static Matrix44f g_camera = Matrix44f::IDENTITY;
static Matrix44f stored_camera;

Matrix44f getCameraMatrix()
{
  return g_camera.inverse();
}

void store_camera_matrix(void)
{
	stored_camera = g_camera;
}

void restore_camera_matrix(void)
{
	g_camera = stored_camera;
}

void prepareCamera()
{
  glLoadIdentity();
  glLoadMatrixf(g_camera);
}

void walkLeft(float amount)
{
  Matrix44f m;
  m.loadTranslateRM(amount, 0.0f, 0.0f);
  g_camera = g_camera*m;
}

void walkRight(float amount)
{
  Matrix44f m;
  m.loadTranslateRM(-amount, 0.0f, 0.0f);
  g_camera = g_camera*m;
}

void walkForward(float amount)
{
  Matrix44f m;
  m.loadTranslateRM(0.0f, 0.0f, amount);
  g_camera = g_camera*m;
}

void walkBack(float amount)
{
  Matrix44f m;
  m.loadTranslateRM(0.0f, 0.0f, -amount);
  g_camera = g_camera*m;
}

void walkUp(float amount)
{
  Matrix44f m;
  m.loadTranslateRM(0.0f, -amount, 0.0f);
  g_camera = g_camera*m;
}

void walkDown(float amount)
{
  Matrix44f m;
  m.loadTranslateRM(0.0f, amount, 0.0f);
  g_camera = g_camera*m;
}

void turnLeft(float amount)
{
  Matrix44f m;
  m.loadRotateYRM(-amount);
  g_camera = g_camera*m;
}

void turnRight(float amount)
{
  Matrix44f m;
  m.loadRotateYRM(amount);
  g_camera = g_camera*m;
}

void turnUp(float amount)
{
  Matrix44f m;
  m.loadRotateXRM(-amount);
  g_camera = g_camera*m;
}

void turnDown(float amount)
{
  Matrix44f m;
  m.loadRotateXRM(amount);
  g_camera = g_camera*m;
}

void turnCCW(float amount)
{
  Matrix44f m;
  m.loadRotateZRM(-amount);
  g_camera = g_camera*m;
}

void turnCW(float amount)
{
  Matrix44f m;
  m.loadRotateZRM(amount);
  g_camera = g_camera*m;
}
