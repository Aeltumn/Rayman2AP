#pragma once
#include "framework.h"


LRESULT CALLBACK MOD_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void MOD_Print(char*, ...);
void MOD_Main(void);
