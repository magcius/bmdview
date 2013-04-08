#include "textbox.h"

/*
What can be improved:
- support for a line of text above or below the edit control?
- support icons?
- support for more MB_constants
- support for text input?
- printf functionality?
- modeless dialog functionality?
- custom button labels?
- use i18ned labels?
- enhance keyboard support
- mousewheel support
- make window look more like a message box (no close button, no icon, etc.)
*/

typedef struct _TEXTBOXDATA
{
  UINT type;
  HWND textWin;
  int numButtons;
  int defButton;
  HWND buttons[3];
  char* buttonLabels[3];
  int buttonIds[3];
  const char* text;
  int numLines;
  int maxLineLength;
  int minWidth;
  int minHeight;
  int retVal;

  HWND parent;
} TEXTBOXDATA;

LRESULT CALLBACK textBoxListener(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  TEXTBOXDATA* dat = (TEXTBOXDATA*)GetWindowLong(hWnd, GWL_USERDATA);

  switch(msg)
  {
    case WM_CREATE:
    {
      dat = (TEXTBOXDATA*)((CREATESTRUCT*)lParam)->lpCreateParams;
      SetWindowLong(hWnd, GWL_USERDATA, (LONG)dat);

      HINSTANCE hInst = ((CREATESTRUCT*)lParam)->hInstance;

      HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

      char* labels[] = {  "OK", "", "", "OK", "Cancel", "", "Abort", "Retry", "Ignore",
       "Yes", "No", "Cancel", "Yes", "No", "", "Retry", "Cancel", "" };

      for(int i = 0; i < dat->numButtons; ++i)
      {
        int style = (i == dat->defButton)?BS_DEFPUSHBUTTON:BS_PUSHBUTTON;
        dat->buttons[i] = CreateWindowEx(0, "BUTTON", dat->buttonLabels[i],
          WS_CHILD | WS_VISIBLE | style,
          0, 0, 0, 0,
          hWnd, (HMENU)(367 + i), hInst, NULL);
        SendMessage(dat->buttons[i], WM_SETFONT, (WPARAM)font, 0);
      }

      dat->textWin = CreateWindowEx(0, "EDIT", dat->text,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | ES_READONLY,
        0, 0, 0, 0, hWnd, NULL, hInst, NULL);
      if((dat->type & TB_USEFIXEDFONT) != 0)
        SendMessage(dat->textWin, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), 0);
      else
        SendMessage(dat->textWin, WM_SETFONT, (WPARAM)font, 0);

      SetFocus(hWnd);

      return 0;
    }break;

    case WM_SIZE:
    {
      RECT r;
      GetClientRect(hWnd, &r);

      HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
      HDC dc = GetDC(hWnd);
      HFONT oldFont = (HFONT)SelectObject(dc, font);
      TEXTMETRIC tm;
      GetTextMetrics(dc, &tm);
      SelectObject(dc, oldFont);
      ReleaseDC(hWnd, dc);

      int bw = tm.tmAveCharWidth*19, bh = tm.tmHeight + 10;

      for(int i = 0; i < dat->numButtons; ++i)
        SetWindowPos(dat->buttons[i], NULL,
          r.right/2 - (int)(dat->numButtons*0.5f*bw + (dat->numButtons - 1)*2.5f) + i*(bw + 5), r.bottom - bh - 15,
          bw, bh, SWP_NOZORDER);

      SetWindowPos(dat->textWin, NULL,
        5, 5, r.right - 10, r.bottom - bh - 35, SWP_NOZORDER);

      return 0;
    }break;

    case WM_GETMINMAXINFO:
    {
      MINMAXINFO* info = (MINMAXINFO*)lParam;
      if(dat != NULL)
      {
        info->ptMinTrackSize.x = dat->minWidth;
        info->ptMinTrackSize.y = dat->minHeight;
      }
      return 0;
    }break;

    case WM_COMMAND:
    {
      if(LOWORD(wParam) >= 367 && LOWORD(wParam) <= 369)
      {
        dat->retVal = dat->buttonIds[LOWORD(wParam) - 367];
        PostMessage(hWnd, WM_CLOSE, 0, 0);
      }
      return 0;
    }break;

    case WM_KEYDOWN:
    {
      if((int)wParam == VK_RETURN)
        PostMessage(dat->buttons[dat->defButton], BM_CLICK, 367, 0);
      return 0;
    }break;

    case WM_CLOSE:
    {
      if(dat->parent != NULL)
      {
        //we have to call this before the textbox is destroyed,
        //otherwise the wrong window gets the focus and becomes
        //the foreground window when the textbox window is
        //destroyed.
        EnableWindow(dat->parent, TRUE);
      }

      DestroyWindow(hWnd);

      return 0;
    }break;

    default:
      return DefWindowProc(hWnd, msg, wParam, lParam);
  }
}

int TextBox(HWND parent, const char* text, const char* caption, UINT type)
{
  static bool inited = false;

  if(!inited)
  {
    WNDCLASSEX wc = { sizeof(wc), 0, textBoxListener, 0, 0, GetModuleHandle(NULL),
      NULL /*icon*/, LoadCursor(NULL, IDC_ARROW), GetSysColorBrush(COLOR_MENU),
      NULL, "textboxclass", NULL };
    RegisterClassEx(&wc);
    inited = true;
  }

  TEXTBOXDATA* dat = (TEXTBOXDATA*)malloc(sizeof(TEXTBOXDATA));
  dat->type = type;
  dat->parent = parent;

  //create buttons
  UINT butType = type & MB_TYPEMASK;
  switch(butType)
  {
    case MB_OK:
      dat->numButtons = 1;
      dat->buttonLabels[0] = "OK";
      dat->buttonIds[0] = IDOK;
    break;
    case MB_OKCANCEL:
      dat->numButtons = 2;
      dat->buttonLabels[0] = "OK";
      dat->buttonLabels[1] = "Cancel";
      dat->buttonIds[0] = IDOK;
      dat->buttonIds[1] = IDCANCEL;
    break;
    case MB_ABORTRETRYIGNORE:
      dat->numButtons = 3;
      dat->buttonLabels[0] = "Abort";
      dat->buttonLabels[1] = "Retry";
      dat->buttonLabels[2] = "Ignore";
      dat->buttonIds[0] = IDABORT;
      dat->buttonIds[1] = IDRETRY;
      dat->buttonIds[2] = IDIGNORE;
    break;
    case MB_YESNOCANCEL:
      dat->numButtons = 3;
      dat->buttonLabels[0] = "Yes";
      dat->buttonLabels[1] = "No";
      dat->buttonLabels[2] = "Cancel";
      dat->buttonIds[0] = IDYES;
      dat->buttonIds[1] = IDNO;
      dat->buttonIds[2] = IDCANCEL;
    break;
    case MB_YESNO:
      dat->numButtons = 2;
      dat->buttonLabels[0] = "Yes";
      dat->buttonLabels[1] = "No";
      dat->buttonIds[0] = IDYES;
      dat->buttonIds[1] = IDNO;
    break;
    case MB_RETRYCANCEL:
      dat->numButtons = 2;
      dat->buttonLabels[0] = "Retry";
      dat->buttonLabels[1] = "Cancel";
      dat->buttonIds[0] = IDRETRY;
      dat->buttonIds[1] = IDCANCEL;
    break;
    default:
      dat->numButtons = 1;
      dat->buttonLabels[0] = "OK";
      dat->buttonIds[0] = IDOK;
    break;
  }

  UINT defButton = type & MB_DEFMASK;
  switch(defButton)
  {
    case MB_DEFBUTTON1: dat->defButton = 0; break;
    case MB_DEFBUTTON2: dat->defButton = 1; break;
    case MB_DEFBUTTON3: dat->defButton = 2; break;
    case MB_DEFBUTTON4: dat->defButton = 3; break;
    default: dat->defButton = 0; break;
  }
  if(dat->defButton >= dat->numButtons)
    dat->defButton = 0;


  //count number of newline characters and maximal line length
  int len = strlen(text);
  int lineCount = 0, maxLineLen = 0, lineLen = 0;
  for(int i = 0; i < len; ++i)
  {
    if(text[i] == '\n')
    {
      if(lineLen > maxLineLen)
        maxLineLen = lineLen;
      lineLen = 0;
      ++lineCount;
    }
    else
      ++lineLen;
  }

  dat->numLines = lineCount;
  dat->maxLineLength = maxLineLen;

  //convert '\n' to "\r\n" so for the text control
  char* newText = (char*)malloc(len + lineCount + 1);

  int srcIndx = 0, dstIndx = 0;
  while(text[srcIndx] != '\0')
  {
    if(text[srcIndx] == '\n')
    {
      newText[dstIndx] = '\r';
      ++dstIndx;
    }
    newText[dstIndx] = text[srcIndx];
    ++dstIndx, ++srcIndx;
  }
  newText[dstIndx] = '\0';
  dat->text = newText;

  if(len > 0 && text[len - 1] != '\n')
    ++lineCount;
  if(lineCount < 3)
    lineCount = 3;

  //select font, calculate text rect size
  HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  HDC dc = GetDC(NULL);
  HFONT oldFont = (HFONT)SelectObject(dc, font);
  TEXTMETRIC tm;
  GetTextMetrics(dc, &tm);
  int textWid = tm.tmAveCharWidth, butWid = tm.tmAveCharWidth;
  int textHyt = tm.tmHeight;
  if((type & TB_USEFIXEDFONT) != 0)
  {
    SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
    TEXTMETRIC ttm;
    GetTextMetrics(dc, &ttm);
    textWid = ttm.tmAveCharWidth;
    textHyt = ttm.tmHeight;
  }
  SelectObject(dc, oldFont);
  ReleaseDC(NULL, dc);

  //center window on parent
  RECT parentRect;
  if(parent != NULL)
    GetWindowRect(parent, &parentRect);
  else
    SystemParametersInfo(SPI_GETWORKAREA, 0, &parentRect, 0);
  int parWid = parentRect.right - parentRect.left;
  int parHyt = parentRect.bottom - parentRect.top;

  dat->minWidth = dat->numButtons*butWid*19 + (dat->numButtons - 1)*5
    + 2*GetSystemMetrics(SM_CXSIZEFRAME) + 30;
  dat->minHeight =
    GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYSIZEFRAME) +
    GetSystemMetrics(SM_CYHSCROLL) +
    + 50 + 4*textHyt;

  int wid = min(2*GetSystemMetrics(SM_CXSIZEFRAME) + 15 + maxLineLen*textWid
    + GetSystemMetrics(SM_CXVSCROLL), parWid);
  wid = max(wid, dat->minWidth);
  int hyt = min(
    GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYSIZEFRAME) +
    GetSystemMetrics(SM_CYHSCROLL) +
    + 50 + tm.tmHeight + lineCount*textHyt, parHyt);

  //if parent is topmost, we better are topmost as well
  int extStyle = 0;
  if((type & MB_TOPMOST) != 0)
    extStyle |= WS_EX_TOPMOST;


  //create window
  HWND hWnd = CreateWindowEx(extStyle, "textboxclass", caption,
    WS_OVERLAPPED | WS_CAPTION | WS_POPUPWINDOW | WS_THICKFRAME | WS_MAXIMIZEBOX,
    parentRect.left + (parWid - wid)/2, parentRect.top + (parHyt - hyt)/2, wid, hyt, NULL, NULL, GetModuleHandle(NULL), dat);

  free(newText);

  if(parent != NULL)
  {
    //modal dialog
    EnableWindow(parent, FALSE);
  }

  ShowWindow(hWnd, SW_SHOW);

  MSG msg;
  while(IsWindowVisible(hWnd) && GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  int ret = dat->retVal;
  free(dat);

  return ret;
}
