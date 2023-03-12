//==============================================================
//	Ed Kurlyak 2023 BSP Tree Software Triangle Rasterization
//==============================================================

#include "MyApp.h"

//************************************************
//Функция WinMain
//************************************************
int WINAPI WinMain(	HINSTANCE	hInstance,
					HINSTANCE	hPrevInstance,
					LPSTR		lpCmdLine,
					int			nCmdShow)
{
	CMyApp App;

	return App.ProgramBegin(hInstance, nCmdShow);
}

//************************************************
//Статическая функция класса для обработки
//сообщений окна
//************************************************
LRESULT CALLBACK CMyApp::StaticWndProc(	HWND	hWnd,
							UINT	msg,
							WPARAM	wParam,
							LPARAM	lParam)
{

	if ( msg == WM_CREATE )
		SetWindowLong( hWnd, GWL_USERDATA, (LONG)(LONG_PTR)((CREATESTRUCT FAR *)lParam)->lpCreateParams);

	CMyApp *MainWndProc = (CMyApp*)(LONG_PTR)GetWindowLong( hWnd, GWL_USERDATA );
	if (MainWndProc) return MainWndProc->WndProc( hWnd, msg, wParam, lParam );
	
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

//************************************************
//Функция обработки сообщений окна
//************************************************
LRESULT CMyApp::WndProc(	HWND	hWnd,
							UINT	uMsg,
							WPARAM	wParam,
							LPARAM	lParam)
{
	//проверяем сообщения
	switch (uMsg)
	{
		//мы получили сообщение закрыть приложение
		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

//************************************************
//Функция инициализирует главное окно программы
//и запускает цикл обработки сообщений
//************************************************
int CMyApp::ProgramBegin(HINSTANCE	hInstance, int nCmdShow)
{
	WNDCLASS wcl;

	wcl.style			= CS_HREDRAW | CS_VREDRAW;;
	wcl.lpfnWndProc		= (WNDPROC) StaticWndProc;
	wcl.cbClsExtra		= 0L;
	wcl.cbWndExtra		= 0L;
	wcl.hInstance		= GetModuleHandle(NULL);
	wcl.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wcl.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcl.lpszMenuName	= NULL;
	wcl.lpszClassName	= CLASSNAME;
	
	if(!RegisterClass (&wcl)) return 0;

	m_hWnd = CreateWindow(CLASSNAME, APPNAME,
			  WS_OVERLAPPEDWINDOW,
              0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
              NULL, NULL, hInstance, this);

	if (!m_hWnd) return 0;
	
	RECT WindowRect = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };

	AdjustWindowRectEx(&WindowRect,
		GetWindowStyle(m_hWnd),
		GetMenu(m_hWnd) != NULL,
		GetWindowExStyle(m_hWnd));

	UINT WidthScreen = GetSystemMetrics(SM_CXSCREEN);
	UINT HeightScreen = GetSystemMetrics(SM_CYSCREEN);

	UINT WidthX = WindowRect.right - WindowRect.left;
	UINT WidthY = WindowRect.bottom - WindowRect.top;

	UINT PosX = (WidthScreen - WidthX) / 2;
	UINT PosY = (HeightScreen - WidthY) / 2;

	MoveWindow(m_hWnd,
		PosX,
		PosY,
		WidthX,
		WidthY,
		FALSE);

	ShowWindow (m_hWnd, nCmdShow);
	UpdateWindow (m_hWnd);
	SetForegroundWindow(m_hWnd);

	m_MeshManager.Init_Scene(m_hWnd);

	ShowCursor(FALSE);
	
	MSG msg = { 0 };
    
	while (msg.message != WM_QUIT)
    {
	    if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
			TranslateMessage (&msg) ;
            DispatchMessage (&msg) ;
		}

		if(GetKeyState(VK_ESCAPE) & 0xFF00)
			break;

		m_MeshManager.Draw_Scene();
		
	}

	DestroyWindow(m_hWnd);
	UnregisterClass(wcl.lpszClassName, wcl.hInstance);

	return (int) msg.wParam;
}
