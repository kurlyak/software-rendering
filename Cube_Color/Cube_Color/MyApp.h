//=============================================
//	Ed Kurlyak 2023 Color Interpolation
//=============================================

#ifndef _MYAPP_
#define _MYAPP_

#include <windows.h>
#include <windowsx.h>
#include "MeshManager.h"

#define APPNAME "Color Interpolation Software Rendering"
#define CLASSNAME "Sample"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

class CMyApp
{
public:
	
	int ProgramBegin(HINSTANCE hInstance, int nCmdShow);

private:

	static LRESULT CALLBACK StaticWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND m_hWnd;
	CMeshManager m_MeshManager;
};

#endif