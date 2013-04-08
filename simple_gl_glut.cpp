#include <iostream>
#include <string>
using namespace std;

#include "GL/glew.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#include "mac.h"
#else
#include <GL/glut.h>
#endif

#include "parameters.h"
#include "simple_gl_common.h"
#include "drawtext.h"
#include "simple_gl.h"
#include "ui.h"
#include "resource.h"
#include "camera.h"

#include "main.h"
/*
#ifndef NO_3D_MOUSE
#include <3DConnexionClient/ConnexionClientAPI.h>

//extern OSErr InstallConnexionHandlers() __attribute__((weak_import));
u16 fConnexionClientID = 0;

#include "camera.h"

void mouse3dHandler(io_connect_t connection, natural_t messageType,
    void *messageArgument)
{
  const float R = 0.005 * M_PI / 360;
  const float V = 0.5;

  static ConnexionDeviceState	lastState;
  ConnexionDeviceState		*state;

  switch(messageType)
  {
    case kConnexionMsgDeviceState:
      state = (ConnexionDeviceState*)messageArgument;
      if(state->client == fConnexionClientID)
      {
        switch (state->command)
        {
          case kConnexionCmdHandleAxis:
            if(state->axis[0] != lastState.axis[0])
              walkRight(V * state->axis[0]);
            if(state->axis[1] != lastState.axis[1])
              walkBack(V * state->axis[1]);
            if(state->axis[2] != lastState.axis[2])
              walkDown(V * state->axis[2]);

            if(state->axis[3] != lastState.axis[3])
              turnUp(R * state->axis[3]);
            if(state->axis[4] != lastState.axis[4])
              turnCCW(R * state->axis[4]);
            if(state->axis[5] != lastState.axis[5])
              turnRight(R * state->axis[5]);

            break;

          //case kConnexionCmdHandleButtons:
            //if(state->buttons != lastState.buttons)
              //cerr << "Buttons " << state->buttons << endl;
            //break;
        }

        memcpy(state, &lastState, (long)sizeof(ConnexionDeviceState));
      }
      break;

    default:
      break;
  }
}

#endif
*/
void setup3dMouse()
{
  //http://archives.devshed.com/forums/bsd-93/framework-weak-linking-weirdness-2357874.html
  //http://www.3dconnexion.com/forum/viewtopic.php?t=1223
  //I guess that happens because the compiler doesn't know that the framework
  //is weak-linked, only the linker knows. Hence, we need to force the compiler
  //to not assume anything about the weakly linked symbols.
  //(note that the _value_, not the pointer itself, needs to be declared
  //volatile for that!)
/*  void* volatile test = (void*)InstallConnexionHandlers; //required!

#ifndef NO_3D_MOUSE
  // Make sure the framework is installed
  if(test != 0)
  {
    OSErr error = InstallConnexionHandlers(mouse3dHandler, 0L, 0L);
    if (error != noErr) return;

    // Use mouse only when we're the frontmost app (this seems to
    // require an Info.plist with a CFBundleSignature of 'bmv2')

    fConnexionClientID = RegisterConnexionClient('bdv2',
        (u8*)"\pbmdview2", kConnexionClientModeTakeOver, kConnexionMaskAll);

    // for non-bundled apps, you need to use this, which always gets mouse
    // data, even when your app is not front-most
    //fConnexionClientID = RegisterConnexionClient(kConnexionClientWildcard,

    cerr << "Installed 3d mouse handler" << endl;
  }
#endif*/
}

void teardown3dMouse()
{/*
#ifndef NO_3D_MOUSE
  //see setup3dMouse for why this is needed
  void* volatile test = (void*)InstallConnexionHandlers; //required!
  if(test != 0)
  {
    // Unregister our client and clean up all handlers
    if(fConnexionClientID) UnregisterConnexionClient(fConnexionClientID);
    CleanupConnexionHandlers();
  }
#endif*/
}

extern bool init();
extern void draw();
extern void exit();


GLuint g_fontBase;
static int g_width, g_height;

const int IS_DOWN_SIZE = 300;
static bool isDown[IS_DOWN_SIZE];

const int REFRESH_MS = 25;

void setStartupText(const std::string& s)
{
  cout << s << endl;
}

bool isKeyPressed(int k)
{
  if(k >= IS_DOWN_SIZE) return false;
  return isDown[k];
}

void flush()
{
  //draw strings
//  if(isKeyPressed(BKEY_TAB))
  {
    renderTexts(g_width, g_height, 7, 12,
             g_fontBase, 1.f/getAverageSecondsPerFrame());
  }

  glutSwapBuffers();
}

void onDisplay()
{
  int start = glutGet(GLUT_ELAPSED_TIME);

  draw();

  setLastFrameSeconds((glutGet(GLUT_ELAPSED_TIME) - start)/1000.f);
  //setLastFrameSeconds(REFRESH_MS/1000.f);
}

void onReshape(int w, int h)
{
  g_width = w; g_height = h;
  updateProjectionMatrix(w, h);
}

void onMenu(int option)
{
  switch(option)
  {
    case MENU_FILE_EXPORT_SHADERS:
      menuFileExportShaders();
      break;

    case MENU_FILE_REIMPORT_SHADERS:
      menuFileReimportShaders();
      break;

    case MENU_FILE_REGENERATE_SHADERS:
      menuFileRegenerateShaders();
      break;

    case MENU_FILE_EXPORT_MODEL:
      menuFileExportModel(g_models.back().bmdFileName + "_export.3ds");
      break;

    case MENU_FILE_EXPORT_TEXTURES:
      menuFileExportTextures(g_models.back().bmdFileName + "_tex.tga");
      break;

    case MENU_DEBUG_SECTIONINFO:
      menuDebugSectioninfo();
      break;
  }
}

void setupMenu()
{
  glutCreateMenu(onMenu);
  glutAddMenuEntry("Export Shaders", MENU_FILE_EXPORT_SHADERS);
  glutAddMenuEntry("Re-Import Shaders", MENU_FILE_REIMPORT_SHADERS);
  glutAddMenuEntry("Regenerate Shaders", MENU_FILE_REGENERATE_SHADERS);

  glutAddMenuEntry("Dump model", MENU_FILE_EXPORT_MODEL);
  glutAddMenuEntry("Dump textures", MENU_FILE_EXPORT_TEXTURES);

  glutAddMenuEntry("Print debug stuff", MENU_DEBUG_SECTIONINFO);

  glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

void init3D()
{
  glewInit();

  g_fontBase = glGenLists(256);
  for(int i = 0; i < 256; ++i)
  {
    glNewList(g_fontBase + i, GL_COMPILE);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, (char)i);
    glEndList();
  }
}

void updateModifiers()
{
  //this is a hack: glut is not able to tell us if ctrl, shift or alt
  //are pressed, they are only sent as modifiers with keyboard events
  //for other keys. so this method can't detect if a modifier is pressed
  //when no other key is pressed at the same time
  int mod = glutGetModifiers();
  isDown[BKEY_SHIFT] = (mod & GLUT_ACTIVE_SHIFT) != 0;
  isDown[BKEY_CONTROL] = (mod & GLUT_ACTIVE_CTRL) != 0;
  isDown[BKEY_ALT] = (mod & GLUT_ACTIVE_ALT) != 0;
}

void onKeyboard(unsigned char key, int x, int y)
{
  updateModifiers();
  isDown[toupper(key)] = true;
	if(key == BKEY_ESCAPE)
		exit(0);
}

void onKeyboardUp(unsigned char key, int x, int y)
{
  updateModifiers();
  isDown[toupper(key)] = false;
}

static int downx=0, downy=0, mousedown=0;

const float movespeed = 100;
void onMouse(int button, int state, int x, int y)
{
	const float frontback = movespeed * .5;
//	printf("onmouse (%d:%d) %d, %d\n", button, state, x, y);
	if(button==3) walkForward(frontback);
	if(button==4) walkBack(frontback);
	if(button==0 || button==2)
	{
		if(state==0)
		{
			downx = x;
			downy = y;
			mousedown = (button==0) ? 1 : 2;
			store_camera_matrix();
		} else
		{
			mousedown = 0;
		}
	}
}

static float turn_angle(int x, int y)
{
	return atan2(y - g_height/2, x - g_width/2);
}

void onMotion(int x, int y)
{
//	printf("onmotion %d, %d\n", x, y);
	updateModifiers();
	int min = (g_width < g_height) ? g_width : g_height;
	const float slidespeed = movespeed * .05 * 1000.0 / min;

	if(mousedown==0)
		return;
	restore_camera_matrix();
	if(isDown[BKEY_SHIFT])
	{
		if(mousedown==1)
		{
			walkLeft(slidespeed * (x - downx));
			walkUp(slidespeed * (y - downy));
		} else
		{
			float a1 = turn_angle(downx, downy);
			float a2 = turn_angle(x, y);
			turnCCW(a2 - a1);
		}
	} else
	{
		float instep = 5000.0;
		float pivotrate = slidespeed*.0003;
		if(mousedown==2)
		{
			instep = 0.0;
			pivotrate *= -1.0;
		}
		walkForward(instep);
		turnRight(pivotrate * ( x - downx));
		turnDown(pivotrate * ( y - downy));
		walkBack(instep);
	}
}

int specialCode(int glutCode)
{
  switch(glutCode)
  {
    case GLUT_KEY_LEFT: return BKEY_LEFT;
    case GLUT_KEY_RIGHT: return BKEY_RIGHT;
    case GLUT_KEY_DOWN: return BKEY_DOWN;
    case GLUT_KEY_UP: return BKEY_UP;
    case GLUT_KEY_PAGE_UP: return BKEY_PG_UP;
    case GLUT_KEY_PAGE_DOWN: return BKEY_PG_DOWN;
    case GLUT_KEY_F1: return BKEY_F1;
    case GLUT_KEY_F2: return BKEY_F2;
    default: return IS_DOWN_SIZE - 1;
  }
}

void onSpecial(int key, int x, int y)
{
  updateModifiers();
  isDown[specialCode(key)] = true;
}

void onSpecialUp(int key, int x, int y)
{
  updateModifiers();
  isDown[specialCode(key)] = false;
}

void onTimer(int val)
{
  handleCamera();
  glutPostRedisplay();
  glutTimerFunc(REFRESH_MS, onTimer, 0);
}

int main(int argc, char* argv[])
{
  //char buff[2048]; getcwd(buff, 2048);
  glutInit(&argc, argv);
  //chdir(buff); //on osx, glutInit changes the pwd to the bundle's resource dir.
               //revert that.
/*
#ifdef __APPLE__
  initMac();
#endif
*/
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
  glutInitWindowSize(1280, 1024);

  glutCreateWindow("bmdview2");

  setupMenu();

  init3D();

  setup3dMouse();

  // if there are parameters on os x, that file will be loaded twice: once
  // in init, once more when the odoc apple even arrives (later). oh well.
  parseParameters(argc, argv);
  if(!init())
  {
    cerr << "Argh. Something went wrong" << endl;
    return -1;
  }

  glutDisplayFunc(onDisplay);
  glutReshapeFunc(onReshape);
  glutKeyboardFunc(onKeyboard);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
  glutKeyboardUpFunc(onKeyboardUp);
  glutSpecialFunc(onSpecial);
  glutSpecialUpFunc(onSpecialUp);

  glutTimerFunc(REFRESH_MS, onTimer, 0);

  glutMainLoop();

  exit();

  teardown3dMouse();
}
