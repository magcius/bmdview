#include "simple_gl.h"

#include <cmath>
#include <cstdio>
#include <vector>
#include <string>
#include <cassert>
#ifdef _WIN32
    #include <windows.h>
#endif
using namespace std;

#include "Matrix44.h"

#include "resource.h"
#include "ui.h"

#include "drawtext.h"
#include "parameters.h"
#include "simple_gl_common.h"
#include "camera.h"

extern bool init();
extern void draw();
extern void exit();

//in main.cpp (TODO: put that in some header?)
void loadFile(const string& name, bool merge = false);

static bool g_done = false, g_isInited = false, g_initCalled = false;
#ifdef _WIN32
static HINSTANCE g_hInst = NULL;
static HWND g_hWnd = NULL;
static HDC g_hDc = NULL;
static HGLRC g_hContext = NULL;
static GLYPHMETRICS g_glyphMetrics[256];
static TEXTMETRIC g_fontMetrics;
#endif
static int g_width = 0, g_height = 0;
GLuint g_fontBase;

static bool g_isInputOn = false;
static bool g_isInputComplete = true;
static string g_input;

static string g_startupText;

int getWindowWidth()
{
  return g_width;
}

int getWindowHeight()
{
  return g_height;
}

bool isInputInProgress()
{
  return g_isInputOn;
}

string getCommand()
{
  if(g_isInputComplete)
  {
    string ret = g_input;
    g_input = "";
    return ret;
  }
  else
    return "";
}

void flush()
{
  //draw strings
  if(!isKeyPressed(VK_TAB))
  {
    //use g_glyphMetrics[currChar].gmCellIncX for char width?
    renderTexts(g_width, g_height,
            g_fontMetrics.tmAveCharWidth, g_fontMetrics.tmHeight,
            g_fontBase, 1.f/getAverageSecondsPerFrame());
  }

  glFlush();
  SwapBuffers(g_hDc);
}

HWND getHWnd()
{
  return g_hWnd;
}

void setStartupText(const std::string& text)
{
  if(g_isInited)
    return; //otherwise, once the second model is loaded,
            //the message pump in this function would
            //call draw on a half loaded model, causing a crash

  g_startupText = text;
  InvalidateRect(getHWnd(), NULL, TRUE);
  UpdateWindow(getHWnd());

  //pump some messages
  MSG msg;
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

bool isKeyPressed(int key)
{
  return (GetAsyncKeyState(key) & 0x8000) != 0;
}

bool init3D()
{
  //get window dc
  g_hDc = GetDC(g_hWnd);
  if(g_hDc == NULL)
    return false;

  int bpp = GetDeviceCaps(g_hDc, BITSPIXEL);

  //set pixel format
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.cColorBits = (bpp >= 24)?24:16;
  pfd.cAlphaBits = (bpp == 32)?8:0;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cDepthBits = (bpp >= 24)?24:16;
  pfd.cStencilBits = (bpp == 32)?8:0;

  int formatIndex = ChoosePixelFormat(g_hDc, &pfd);

  //debug:
  DescribePixelFormat(g_hDc, formatIndex, sizeof(pfd), &pfd);

  SetPixelFormat(g_hDc, formatIndex, &pfd);

  //create rendering context
  g_hContext = wglCreateContext(g_hDc);
  if(g_hContext == NULL)
    return false;

  //activate context
  if(!wglMakeCurrent(g_hDc, g_hContext))
    return false;;

  //load extensions
  glewInit();

  //load font
  g_fontBase = glGenLists(256);
  //HFONT old = (HFONT)SelectObject(g_hDc, GetStockObject(DEFAULT_GUI_FONT));
  HFONT old = (HFONT)SelectObject(g_hDc, GetStockObject(OEM_FIXED_FONT));
  wglUseFontBitmaps(g_hDc, 0, 256, g_fontBase);
  for(int i = 0; i < 256; ++i)
    GetGlyphOutline(g_hDc, i, GGO_METRICS, &g_glyphMetrics[i], 0, NULL, NULL);
  GetTextMetrics(g_hDc, &g_fontMetrics);
  SelectObject(g_hDc, old);

  return true;
}

void exit3d()
{
  glDeleteLists(g_fontBase, 256);

  wglMakeCurrent(NULL, NULL);
  if(g_hContext != NULL)
  {
    wglDeleteContext(g_hContext);
    g_hContext = NULL;
  }
  if(g_hDc != NULL)
  {
    ReleaseDC(g_hWnd, g_hDc);
    g_hDc = NULL;
  }
}

void centerWindowInWindow(HWND hDlg, HWND hParent)
{
  RECT dlg, parent;
  GetWindowRect(hDlg, &dlg);
  dlg.right -= dlg.left;
  dlg.bottom -= dlg.top;
  GetWindowRect(hParent, &parent);
  parent.right -= parent.left;
  parent.bottom -= parent.top;
  MoveWindow(hDlg,
    parent.left + (parent.right - dlg.right)/2,
    parent.top + (parent.bottom - dlg.bottom)/2,
    dlg.right, dlg.bottom, FALSE);
}

BOOL CALLBACK dialogProcAbout(HWND hDlg, UINT msg, WPARAM wP, LPARAM lP)
{
  switch(msg)
  {
    case WM_INITDIALOG:
    {
      centerWindowInWindow(hDlg, g_hWnd);

      SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_DATE),
        (string("Build:\n") + string(__DATE__)).c_str());

      return TRUE;
    }

    case WM_COMMAND:
      switch(LOWORD(wP))
      {
        case IDOK:
        case IDCANCEL:
          EndDialog(hDlg, 0);
          return TRUE;
      }
      break;
  }
  return FALSE;
}

LRESULT CALLBACK eventListener(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static int lastMouseX = 0, lastMouseY = 0;
  static bool isLDown = false;

  switch(message)
  {
    case WM_SIZE:
    {
      g_width = (int)LOWORD(lParam);
      g_height = (int)HIWORD(lParam);
      updateProjectionMatrix(g_width, g_height);
      return 0;
    }break;

    case WM_PAINT:
    {
      if(!g_isInited)
      {
        RECT r;
        PAINTSTRUCT ps;
        GetClientRect(hWnd, &r);
        HDC hDc = BeginPaint(hWnd, &ps);
        DrawText(hDc, g_startupText.data(), g_startupText.length(),
          &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hWnd, &ps);

        //this allows init() to redraw the window without
        //calling itself
        if(!g_initCalled)
        {
          g_initCalled = true;
          if(!init())
          {
            MessageBox(hWnd, "Something went wrong!",
              "ARGH!!!", MB_OK);

            g_done = true;
          }
          else
          {
            g_isInited = true;
            draw();
          }
        }
      }
      else
      {
        ValidateRect(hWnd, NULL);
        draw();
      }
      return 0;
    }break;

    case WM_LBUTTONDOWN:
    {
      isLDown = true;
      POINTS p = MAKEPOINTS(lParam);
      lastMouseX = p.x;
      lastMouseY = p.y;
      return 0;
    }break;

    case WM_MOUSEMOVE:
    {
      if(!isLDown)
        return 0;

      RECT r;
      GetClientRect(hWnd, &r);
      POINTS pos = MAKEPOINTS(lParam);
      int dx = pos.x - lastMouseX;
      int dy = pos.y - lastMouseY;
      float radX = 2*PI*float(dx)/float(r.right);
      float radY = 2*PI*float(dy)/float(r.bottom);

      turnRight(radX);
      turnDown(radY);

      lastMouseX = pos.x;
      lastMouseY = pos.y;
      return 0;
    }break;

    case WM_LBUTTONUP:
    {
      isLDown = false;
      return 0;
    }break;

    case WM_CHAR:
    {
      switch(LOWORD(wParam))
      {
        case '\r':
        {
          if(g_isInputOn)
          {
            g_isInputOn = false;
            g_isInputComplete = true;
          }
          else
          {
            g_isInputOn = true;
            g_isInputComplete = false;
            g_input = "";
          }
        }break;

        case '\b':
        {
          if(g_isInputOn)
            g_input = g_input.substr(0, g_input.length() - 1);
        }break;

        case 0x1B: //escape
        {
          if(g_isInputOn)
          {
            g_isInputOn = false;
            g_isInputComplete = true;
            g_input = "";
          }
        }break;

        default:
        {
          if(g_isInputOn)
            g_input += (char)LOWORD(wParam);
        }break;
      }
      return 0;
    }break;

    case WM_DROPFILES:
    {
      HDROP hDrop = (HDROP)wParam;
      char buff[1024];
      DragQueryFile(hDrop, 0, buff, 1024);

      loadFile(buff);

      DragFinish(hDrop);
      return 0;
    }break;

    case WM_COMMAND:
    {
      UpdateWindow(hWnd);

      switch(LOWORD(wParam))
      {
        case MENU_FILE_OPEN_MODEL:
          menuFileOpenModel();
          break;

        case MENU_FILE_MERGE_MODEL:
          menuFileMergeModel();
          break;

        case MENU_FILE_OPEN_ANIMATION:
          menuFileOpenAnimation();
          break;

        case MENU_FILE_EXPORT_MODEL:
          menuFileExportModel();
          break;

        case MENU_FILE_EXPORT_TEXTURES:
          menuFileExportTextures();
          break;

        case MENU_FILE_EXPORT_SHADERS:
          menuFileExportShaders();
          break;

        case MENU_FILE_REIMPORT_SHADERS:
          menuFileReimportShaders();
          break;

        case MENU_FILE_REGENERATE_SHADERS:
          menuFileRegenerateShaders();
          break;

        case MENU_FILE_EXIT:
          PostQuitMessage(0);
          break;

        case MENU_HELP_ABOUT:
          DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT),
            g_hWnd, dialogProcAbout);
          break;

        case MENU_DEBUG_SECTIONINFO:
          menuDebugSectioninfo();
          break;
      }
      return 0;
    }break;

    case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE old, LPSTR params, int showCom)
{
  parseParameters(params);

  WNDCLASSEX wc = { sizeof(wc), CS_HREDRAW | CS_VREDRAW, eventListener, 0, 0, hInst,
                    LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON)), LoadCursor(NULL, IDC_ARROW),
                    (HBRUSH)GetStockObject(WHITE_BRUSH),
                    MAKEINTRESOURCE(IDM_MENU), "glclass",
                    LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON))
                  };

  if(!RegisterClassEx(&wc))
    return -1;

  //RECT client = { 0, 0, 720, 576 }; //PAL resolution
  RECT client = { 0, 0, 1200, 650 };
  AdjustWindowRect(&client, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, TRUE);
  client.right -= client.left;
  client.bottom -= client.top;
  HWND hWnd = CreateWindowEx(0, "glclass", "bmdview2", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                             0, 0, client.right, client.bottom,
                             NULL, NULL, hInst, NULL);

  if(hWnd == NULL)
    return -1;

  g_hInst = hInst;
  g_hWnd = hWnd;
  g_done = !init3D();

  ShowWindow(hWnd, showCom); //SW_MAXIMIZE);

  if(g_done)
    MessageBox(hWnd, "OpenGL couldn't be set up", "ARGH!!!", MB_OK);

  setStartupText("Loading...");

  DragAcceptFiles(hWnd, TRUE);

  MSG msg;
  DWORD startTime = GetTickCount();
  while(!g_done)
  {
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if(msg.message == WM_QUIT)
        g_done = true;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      startTime = GetTickCount();

      if(g_isInputComplete)
        handleCamera();
      else
        drawText(g_input.c_str());

      draw();
      setLastFrameSeconds((GetTickCount() - startTime)/1000.0f);
    }
  }

  exit();
  exit3d();

  return msg.wParam;
}
