//
// Inner Implementation of EggAche Graphics Library
// By BOT Man, 2016
//

#include <windows.h>
#include <windowsx.h>
#include <exception>
#include <string>
#include <unordered_map>
#pragma comment (lib, "Msimg32.lib")

#include "EggAche_impl.h"

// Defined in windows.h but not in MinGW

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

namespace EggAche					// _Window
{
	double _dRatio ()	// _dRatio Wrapper
	{
		return (double) GetSystemMetrics (SM_CYSCREEN) / 1000;
	}

	std::unordered_map<HWND, _Window *> &_mHwnd ()	// _mHwnd Wrapper
	{
		static std::unordered_map<HWND, _Window *> m;
		return m;
	}

	_Window::_Window (ONCLICK fnClick, ONPRESS fnPress, const char * cap_string, int width, int height)
		: _hwnd (NULL), _hEvent (NULL), _fFailed (false), _fClosed (false),
		_fnOnClick (fnClick), _fnOnPress (fnPress), _szCap (cap_string), _cxCanvas (width), _cyCanvas (height)
	{
		if (width < 240 || height < 120)
			throw std::runtime_error ("Err_Window_#1_Too_Small");

#ifdef _MSC_VER
		// Windows SDK only support Unicode version Window Class
		if (_mHwnd ().empty ())
		{
			WNDCLASSW wndclass;
			wndclass.style = CS_HREDRAW | CS_VREDRAW;
			wndclass.lpfnWndProc = _WndProc;
			wndclass.cbClsExtra = 0;
			wndclass.cbWndExtra = 0;
			wndclass.hInstance = (HINSTANCE) GetCurrentProcess ();
			wndclass.hIcon = LoadIcon (NULL, IDI_APPLICATION);
			wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
			wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
			wndclass.lpszMenuName = NULL;
			wndclass.lpszClassName = L"LJN_WNDCLASSA";

			if (!RegisterClassW (&wndclass))
				throw std::runtime_error ("Err_Window_#2_RegClass");
		}
#else
		// MinGW only support ANSI version Window Class
		if (_mHwnd ().empty ())
		{
			WNDCLASSA wndclass;
			wndclass.style = CS_HREDRAW | CS_VREDRAW;
			wndclass.lpfnWndProc = _WndProc;
			wndclass.cbClsExtra = 0;
			wndclass.cbWndExtra = 0;
			wndclass.hInstance = (HINSTANCE) GetCurrentProcess ();
			wndclass.hIcon = LoadIcon (NULL, IDI_APPLICATION);
			wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
			wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
			wndclass.lpszMenuName = NULL;
			wndclass.lpszClassName = "LJN_WNDCLASSA";

			if (!RegisterClassA (&wndclass))
				throw std::runtime_error ("Err_Window_#2_RegClass");
		}
#endif

		this->_hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
		if (!this->_hEvent)
		{
			throw std::runtime_error ("Err_Window_#2_Event");
		}

		auto hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) _NewWindow_Thread,
								(LPVOID) this, 0, NULL);

		if (!hThread)
		{
			CloseHandle (this->_hEvent);
			throw std::runtime_error ("Err_Window_#2_Thread");
		}

		WaitForSingleObject (this->_hEvent, INFINITE);
		CloseHandle (hThread);
		CloseHandle (this->_hEvent);

		if (this->_fFailed)
			throw std::runtime_error ("Err_Window_#3_CreateWindow");
	}

	_Window::_Window (const _Window & origin)
		: _Window (origin._fnOnClick, origin._fnOnPress, origin._szCap.c_str (),
				   origin._cxCanvas, origin._cyCanvas)
	{}

	_Window::~_Window ()
	{
		SendMessage (this->_hwnd, WM_CLOSE, 0, 0);
	}

	bool _Window::Refresh ()
	{
		BeginPaint (this->_hwnd, NULL);
		EndPaint (this->_hwnd, NULL);
		return true;
	}

	void WINAPI _Window::_NewWindow_Thread (_Window *pew)
	{
		MSG msg;

		pew->_hwnd = CreateWindowA ("LJN_WNDCLASSA", pew->_szCap.c_str (),
								   WS_OVERLAPPEDWINDOW,  // & ~WS_THICKFRAME &~WS_MAXIMIZEBOX,
								   CW_USEDEFAULT, CW_USEDEFAULT,  //CW_USEDEFAULT, CW_USEDEFAULT,
								   (int) (pew->_cxCanvas * _dRatio ()) + GetSystemMetrics (SM_CXSIZEFRAME) * 4,
								   (int) (pew->_cyCanvas * _dRatio ())
								   + GetSystemMetrics (SM_CYSIZEFRAME) * 4 + GetSystemMetrics (SM_CYCAPTION),
								   NULL, NULL, (HINSTANCE) GetCurrentProcess (), NULL);
		if (!pew->_hwnd)
		{
			pew->_fFailed = true;
			SetEvent (pew->_hEvent);
		}

		_mHwnd ()[pew->_hwnd] = pew;

		ShowWindow (pew->_hwnd, SW_NORMAL);
		UpdateWindow (pew->_hwnd);
		SetEvent (pew->_hEvent);

		while (GetMessage (&msg, NULL, 0, 0))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}

	LRESULT CALLBACK _Window::_WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int a, b, c;
		switch (message)
		{
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			if (_mHwnd ()[hwnd]->_fnOnClick)
				_mHwnd ()[hwnd]->_fnOnClick (
					GET_X_LPARAM (lParam) * _mHwnd ()[hwnd]->_cxCanvas / _mHwnd ()[hwnd]->_cxClient,
					GET_Y_LPARAM (lParam) * _mHwnd ()[hwnd]->_cyCanvas / _mHwnd ()[hwnd]->_cyClient);
			return 0;

		case WM_CHAR:
			if (_mHwnd ()[hwnd]->_fnOnPress)
				_mHwnd ()[hwnd]->_fnOnPress ((char) wParam);
			return 0;

		case WM_SIZE:
			_mHwnd ()[hwnd]->_cxClient = LOWORD (lParam);
			_mHwnd ()[hwnd]->_cyClient = HIWORD (lParam);

			a = LOWORD (lParam);
			b = HIWORD (lParam);
			c = GetSystemMetrics (SM_CYSCREEN);
			return 0;

		case WM_PAINT:
			_mHwnd ()[hwnd]->Refresh ();
			return 0;

		case WM_DESTROY:
			_mHwnd ()[hwnd]->_fClosed = true;
			_mHwnd ().erase (hwnd);
			_mHwnd ()[NULL] = nullptr;		// _mHwnd ().empty == false

			PostQuitMessage (0);
			return 0;
		}
		return DefWindowProc (hwnd, message, wParam, lParam);
	}
}

namespace EggAche					// _DrawContext
{
	_DrawContext::_DrawContext (unsigned int width, unsigned int height)
		: _hdc (NULL), _hBitmap (NULL), _w (width), _h (height)
	{
		HBRUSH	hBrush;
		RECT	rect;
		HDC		hdcWnd;

		hdcWnd = GetDC (NULL);
		this->_hdc = CreateCompatibleDC (hdcWnd);

		auto cxBmp = (int) (this->_w * _dRatio ());
		auto cyBmp = (int) (this->_h * _dRatio ());

		_hBitmap = CreateCompatibleBitmap (hdcWnd, cxBmp, cyBmp);

		SetMapMode (this->_hdc, MM_ANISOTROPIC);
		SetWindowExtEx (this->_hdc, width, height, NULL);
		SetViewportExtEx (this->_hdc, cxBmp, cyBmp, NULL);

		if (!this->_hdc || !this->_hBitmap)
		{
			if (this->_hBitmap) DeleteObject (this->_hBitmap);
			if (this->_hdc) DeleteDC (this->_hdc);
			ReleaseDC (NULL, hdcWnd);
			throw std::runtime_error ("Err_DC_#0_Bitmap");
		}
		SelectObject (this->_hdc, this->_hBitmap);

		rect.left = rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		hBrush = CreateSolidBrush (_colorMask);
		FillRect (this->_hdc, &rect, hBrush);

		SelectObject (this->_hdc, (HBRUSH) GetStockObject (NULL_BRUSH));
		SelectObject (this->_hdc, (HPEN) GetStockObject (BLACK_PEN));
		SetBkMode (this->_hdc, TRANSPARENT);

		ReleaseDC (NULL, hdcWnd);
		DeleteObject (hBrush);
	}

	_DrawContext::_DrawContext (const _DrawContext & origin)
		: _hdc (NULL), _hBitmap (NULL), _w (origin._w), _h (origin._h)
	{
		RECT	rect;
		HDC		hdcWnd;

		hdcWnd = GetDC (NULL);
		this->_hdc = CreateCompatibleDC (hdcWnd);

		auto cxBmp = (int) (this->_w * _dRatio ());
		auto cyBmp = (int) (this->_h * _dRatio ());

		this->_hBitmap = CreateCompatibleBitmap (hdcWnd, cxBmp, cyBmp);

		SetMapMode (this->_hdc, MM_ANISOTROPIC);
		SetWindowExtEx (this->_hdc, this->_w, this->_h, NULL);
		SetViewportExtEx (this->_hdc, cxBmp, cyBmp, NULL);

		if (!this->_hdc || !this->_hBitmap)
		{
			if (this->_hBitmap) DeleteObject (this->_hBitmap);
			if (this->_hdc) DeleteDC (this->_hdc);
			ReleaseDC (NULL, hdcWnd);
			throw std::runtime_error ("Err_DC_#0_Bitmap");
		}
		SelectObject (this->_hdc, this->_hBitmap);

		rect.left = rect.top = 0;
		rect.right = this->_w;
		rect.bottom = this->_h;

		BitBlt (this->_hdc, 0, 0, this->_w, this->_h, origin._hdc, 0, 0, SRCCOPY);

		SelectObject (this->_hdc, (HBRUSH) GetStockObject (NULL_BRUSH));
		SelectObject (this->_hdc, (HPEN) GetStockObject (BLACK_PEN));
		SetBkMode (this->_hdc, TRANSPARENT);

		ReleaseDC (NULL, hdcWnd);
	}

	_DrawContext::~_DrawContext ()
	{
		HGDIOBJ hObj;

		hObj = SelectObject (this->_hdc, (HBRUSH) GetStockObject (NULL_BRUSH));
		if (hObj != GetStockObject (NULL_BRUSH))
			DeleteObject (hObj);
		hObj = SelectObject (this->_hdc, (HPEN) GetStockObject (BLACK_PEN));
		if (hObj != GetStockObject (BLACK_PEN) && hObj != GetStockObject (NULL_PEN))
			DeleteObject (hObj);

		DeleteObject (this->_hBitmap);
		DeleteDC (this->_hdc);
	}

	const bool _DrawContext::_PaintOn (HDC hdcWnd, int x, int y)
	{
		return !!TransparentBlt (hdcWnd, x, y, this->_w, this->_h,
					this->_hdc, 0, 0, this->_w, this->_h, _colorMask);
	}
		
	const COLORREF _DrawContext::_colorMask = RGB (0, 0, 201);
	const COLORREF _DrawContext::_GetColor (int r, int g, int b)
	{
		r = max (0, min (255, r));
		g = max (0, min (255, g));
		b = max (0, min (255, b));

		if (RGB (r, b, b) != _colorMask)
			return RGB (r, g, b);
		else
			return RGB (r, g, b) + 1;
	}
}