#ifndef NICO_TEXTBOX_INC
#define NICO_TEXTBOX_INC NICO_TEXTBOX_INC

#define WIN32_LEANANDMEAN
#include <windows.h>

/*
The following MB_styles are supported:
MB_ABORTRETRYIGNORE
MB_OK
MB_OKCANCEL
MB_RETRYCANCEL
MB_YESNO
MB_YESNOCANCEL
MB_DEFBUTTON1
MB_DEFBUTTON2
MB_DEFBUTTON3
MB_DEFBUTTON4 <- even if there are only 3button msgboxs...
MB_TOPMOST

That is, the most important flags are there (only the icons are missing)
*/

/*
   this additional flag specifies that the edit control should
   use a fixed width font, as opposed to the default variable width font
*/
#define TB_USEFIXEDFONT 0x01000000L

/* use like MessageBox(), see its documentation */
int TextBox(HWND parent, const char* text, const char* caption, UINT type);

#endif /* NICO_TEXTBOX_INC */
