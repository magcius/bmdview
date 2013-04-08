#ifndef SIMPLE_GL_H
#define SIMPLE_GL_H SIMPLE_GL_H

#include <string>
#include "common.h"

//Functions in this header have to be implemented by the platform-dependent
//implementation

int getWindowWidth();
int getWindowHeight();

bool isInputInProgress();
std::string getCommand();



void flush();


void setStartupText(const std::string& text);

#ifdef _WIN32

#include <windows.h>

const int BKEY_LEFT = VK_LEFT;
const int BKEY_RIGHT = VK_RIGHT;
const int BKEY_UP = VK_UP;
const int BKEY_DOWN = VK_DOWN;
const int BKEY_SHIFT = VK_SHIFT;
const int BKEY_CONTROL = VK_CONTROL;
const int BKEY_TAB = VK_TAB;
const int BKEY_PG_UP = VK_PRIOR;
const int BKEY_PG_DOWN = VK_NEXT;
const int BKEY_F1 = 0x70;
const int BKEY_F2 = 0x71;
const int BKEY_ALT = VK_MENU;
#else
const int BKEY_LEFT = 256;
const int BKEY_RIGHT = 257;
const int BKEY_UP = 258;
const int BKEY_DOWN = 259;
const int BKEY_SHIFT = 260;
const int BKEY_CONTROL = 261;
const int BKEY_TAB = '\t';
const int BKEY_PG_UP = 263;
const int BKEY_PG_DOWN = 264;
const int BKEY_F1 = 265;
const int BKEY_F2 = 266;
const int BKEY_ALT = 267;
const int BKEY_ESCAPE = 0x1b;
#endif

bool isKeyPressed(int key);


#endif //SIMPLE_GL_H
