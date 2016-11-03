//
// Windows Implementation of EggAche Graphics Library
// By BOT Man, 2016
//

#include <exception>
#include <string>
#include <unordered_map>
#include <cmath>

#include <Windows.h>
#include <windowsx.h>

#ifdef _MSC_VER

// Only Windows SDK support GDI+
#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")

// Only MSVC support #pragma link
#pragma comment (lib, "Msimg32.lib")

#else

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#endif

#include "EggAche_Impl.h"

namespace EggAche_Impl
{
	// Handle Manager

#ifdef _MSC_VER
	class GdiPlusManager
	{
	private:
		static ULONG_PTR gdiplusToken;
		static size_t refCount;
	public:
		GdiPlusManager ()
		{
			if (refCount == 0)
			{
				Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				Gdiplus::GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, NULL);
			}
			refCount++;
		}

		~GdiPlusManager ()
		{
			refCount--;
			if (refCount == 0)
				Gdiplus::GdiplusShutdown (gdiplusToken);
		}
	};
	size_t GdiPlusManager::refCount = 0;
	ULONG_PTR GdiPlusManager::gdiplusToken = 0;
#endif

	class WindowImpl_Windows;
	class HwndManager
	{
	private:
		static std::unordered_map<HWND, WindowImpl_Windows *> *_hwndMapper;
		static size_t refCount;
	public:
		HwndManager ()
		{
			if (refCount == 0 && _hwndMapper == nullptr)
				_hwndMapper = new std::unordered_map<HWND, WindowImpl_Windows *> ();
			refCount++;
		}

		~HwndManager ()
		{
			refCount--;
			if (refCount == 0)
			{
				delete _hwndMapper;
				_hwndMapper = nullptr;
			}
		}

		static bool isRegClass;

		static std::unordered_map<HWND, WindowImpl_Windows *> *hwndMapper ()
		{
			return _hwndMapper;
		}
	};
	size_t HwndManager::refCount = 0;
	std::unordered_map<HWND, WindowImpl_Windows *> *HwndManager::_hwndMapper = nullptr;
	bool HwndManager::isRegClass = false;

	// Window

	class WindowImpl_Windows : public WindowImpl
	{
	public:
		WindowImpl_Windows (size_t width, size_t height,
							const char *cap_string);
		~WindowImpl_Windows () override;

		bool Draw (const GUIContext *context, size_t x, size_t y) override;

		std::pair<size_t, size_t> GetSize () override;
		bool IsClosed () const override;

		void OnClick (std::function<void (unsigned, unsigned)> fn) override;
		void OnPress (std::function<void (char)> fn) override;
		void OnResized (std::function<void (unsigned, unsigned)> fn) override;
		void OnRefresh (std::function<void ()> fn) override;

		void OnMouseMove (std::function<void (unsigned, unsigned)> fn) override;
		void OnLButtonDown (std::function<void (unsigned, unsigned)> fn) override;
		void OnRButtonDown (std::function<void (unsigned, unsigned)> fn) override;
		void OnLButtonUp (std::function<void (unsigned, unsigned)> fn) override;
		void OnRButtonUp (std::function<void (unsigned, unsigned)> fn) override;
		void OnKeyDown (std::function<void (char)> fn) override;
		void OnKeyUp (std::function<void (char)> fn) override;

	protected:
		size_t		_cxClient, _cyClient;
		std::string	capStr;

		HWND		_hwnd;
		HANDLE		_hEvent;
		bool		_fFailed;

		std::function<void (unsigned, unsigned)> onClick;
		std::function<void (char)> onPress;
		std::function<void (unsigned, unsigned)> onResized;
		std::function<void ()> onRefresh;

		std::function<void (unsigned, unsigned)> onMouseMove;
		std::function<void (unsigned, unsigned)> onLButtonDown;
		std::function<void (unsigned, unsigned)> onRButtonDown;
		std::function<void (unsigned, unsigned)> onLButtonUp;
		std::function<void (unsigned, unsigned)> onRButtonUp;
		std::function<void (char)> onKeyDown;
		std::function<void (char)> onKeyUp;

		HwndManager _hwndManager;
#ifdef _MSC_VER
		GdiPlusManager _gdiplusManager;
#endif

		static void WINAPI _NewWindow_Thread (WindowImpl_Windows *pew);
		static LRESULT CALLBACK _WndProc (HWND, UINT, WPARAM, LPARAM);

		WindowImpl_Windows (const WindowImpl_Windows &) = delete;		// Not allow to copy
		void operator= (const WindowImpl_Windows &) = delete;			// Not allow to copy
	};

	// Context

	class GUIContext_Windows : public GUIContext
	{
	public:
		GUIContext_Windows (size_t width, size_t height);
		~GUIContext_Windows () override;

		bool SetPen (unsigned width,
					 unsigned r = 0,
					 unsigned g = 0,
					 unsigned b = 0) override;

		bool SetBrush (bool isTransparent,
					   unsigned r,
					   unsigned g,
					   unsigned b) override;

		bool SetFont (unsigned size = 18,
					  const char *family = "Consolas",
					  unsigned r = 0,
					  unsigned g = 0,
					  unsigned b = 0) override;

		bool DrawLine (int xBeg, int yBeg, int xEnd, int yEnd) override;

		bool DrawRect (int xBeg, int yBeg, int xEnd, int yEnd) override;

		bool DrawElps (int xBeg, int yBeg, int xEnd, int yEnd) override;

		bool DrawRdRt (int xBeg, int yBeg,
					   int xEnd, int yEnd, int wElps, int hElps) override;

		bool DrawArc (int xLeft, int yTop, int xRight, int yBottom,
					  double angleBeg, double cAngle) override;

		bool DrawChord (int xLeft, int yTop, int xRight, int yBottom,
						double angleBeg, double cAngle) override;

		bool DrawPie (int xLeft, int yTop, int xRight, int yBottom,
					  double angleBeg, double cAngle) override;

		bool DrawTxt (int xBeg, int yBeg, const char *szText) override;
		size_t GetTxtWidth (const char *szText) override;

		bool DrawImg (const char *fileName,
					  int x, int y,
					  int width = -1, int height = -1,
					  int r = -1,
					  int g = -1,
					  int b = -1) override;

		bool DrawImgMask (const char *srcFile,
						  const char *maskFile,
						  unsigned width, unsigned height,
						  int x_pos, int y_pos,
						  unsigned x_src, unsigned y_src,
						  unsigned x_msk, unsigned y_msk) override;

		bool SaveAsJpg (const char *fileName) const override;
		bool SaveAsPng (const char *fileName) const override;
		bool SaveAsBmp (const char *fileName) const override;

		void Clear () override;

		void PaintOnContext (GUIContext *,
							 size_t x, size_t y) const override;

	protected:
		HDC _hdc;
		HBITMAP _hBitmap;
		size_t _w, _h;

#ifdef _MSC_VER
		GdiPlusManager _gdiplusManager;
#endif

		static const COLORREF _colorMask;
		static const COLORREF _GetColor (unsigned r,
										 unsigned g,
										 unsigned b);

#ifdef _MSC_VER
		bool SaveAsImg (const char *fileName,
						const wchar_t *mime) const;
#else
		bool SaveAsImg (
			std::function<bool (BYTE *pData,
								BITMAPINFOHEADER *pbmInfoHeader)> fnSave) const;
#endif
		friend bool WindowImpl_Windows::Draw (const GUIContext *, size_t, size_t);

		GUIContext_Windows (const GUIContext_Windows &) = delete;		// Not allow to copy
		void operator= (const GUIContext_Windows &) = delete;			// Not allow to copy
	};
}

namespace EggAche_Impl
{
	// Factory

	WindowImpl *GUIFactory_Windows::NewWindow (size_t width, size_t height,
											   const char *cap_string)
	{
		return new WindowImpl_Windows (width, height, cap_string);
	}

	GUIContext *GUIFactory_Windows::NewGUIContext (size_t width, size_t height)
	{
		return new GUIContext_Windows (width, height);
	}

	// Window

	WindowImpl_Windows::WindowImpl_Windows (size_t width, size_t height,
											const char *cap_string)
		: _cxClient (width), _cyClient (height), capStr (cap_string),
		_hwnd (NULL), _hEvent (NULL), _fFailed (false)
	{
		if (width < 240 || height < 120)
			throw std::runtime_error ("Err_Window_#1_Too_Small");

#ifdef _MSC_VER
		// Windows SDK only support Unicode version Window Class
		if (!HwndManager::isRegClass)
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
		if (!HwndManager::isRegClass)
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
		HwndManager::isRegClass = true;

		this->_hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
		if (!this->_hEvent)
		{
			throw std::runtime_error ("Err_Window_#2_Event");
		}

		auto hThread = CreateThread (NULL, 0,
			(LPTHREAD_START_ROUTINE) _NewWindow_Thread,
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

	void WindowImpl_Windows::_NewWindow_Thread (WindowImpl_Windows *pew)
	{
		RECT rect { 0 };
		rect.right = (LONG) pew->_cxClient;
		rect.bottom = (LONG) pew->_cyClient;
		if (!AdjustWindowRect (&rect,
							   WS_OVERLAPPEDWINDOW,
							   FALSE))
		{
			pew->_fFailed = true;
			SetEvent (pew->_hEvent);
		}

		pew->_hwnd = CreateWindowA ("LJN_WNDCLASSA", pew->capStr.c_str (),
									WS_OVERLAPPEDWINDOW,
									CW_USEDEFAULT, CW_USEDEFAULT,
									rect.right - rect.left,
									rect.bottom - rect.top,
									NULL, NULL,
									(HINSTANCE) GetCurrentProcess (), NULL);
		if (!pew->_hwnd)
		{
			pew->_fFailed = true;
			SetEvent (pew->_hEvent);
			return;
		}

		(*HwndManager::hwndMapper ())[pew->_hwnd] = pew;

		ShowWindow (pew->_hwnd, SW_NORMAL);
		UpdateWindow (pew->_hwnd);
		SetEvent (pew->_hEvent);

		MSG msg;
		while (GetMessage (&msg, NULL, 0, 0))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}

	WindowImpl_Windows::~WindowImpl_Windows ()
	{
		if (this->_hwnd != NULL)
			SendMessage (this->_hwnd, WM_CLOSE, 0, 0);
	}

	bool WindowImpl_Windows::Draw (const GUIContext *context,
								   size_t x, size_t y)
	{
		if (this->_hwnd == NULL)
			return false;

		auto hdcWnd = GetDC (this->_hwnd);
		if (!hdcWnd)
			return false;

		// Assume that context is GUIContext_Windows
		auto _context = static_cast<const GUIContext_Windows *> (context);
		if (!TransparentBlt (hdcWnd, (int) x, (int) y, (int) (_context->_w), (int) (_context->_h),
							 _context->_hdc, 0, 0, (int) (_context->_w), (int) (_context->_h),
							 GUIContext_Windows::_colorMask))
		{
			ReleaseDC (this->_hwnd, hdcWnd);
			return false;
		}

		ReleaseDC (this->_hwnd, hdcWnd);
		return true;
	}

	std::pair<size_t, size_t> WindowImpl_Windows::GetSize ()
	{
		return std::make_pair (this->_cxClient, this->_cyClient);
	}

	bool WindowImpl_Windows::IsClosed () const
	{
		return this->_hwnd == NULL;
	}

	void WindowImpl_Windows::OnClick (std::function<void (unsigned, unsigned)> fn)
	{
		onClick = std::move (fn);
	}
	void WindowImpl_Windows::OnPress (std::function<void (char)> fn)
	{
		onPress = std::move (fn);
	}
	void WindowImpl_Windows::OnResized (std::function<void (unsigned, unsigned)> fn)
	{
		onResized = std::move (fn);
	}
	void WindowImpl_Windows::OnRefresh (std::function<void ()> fn)
	{
		onRefresh = std::move (fn);
	}

	void WindowImpl_Windows::OnMouseMove (std::function<void (unsigned, unsigned)> fn)
	{
		onMouseMove = std::move (fn);
	}
	void WindowImpl_Windows::OnLButtonDown (std::function<void (unsigned, unsigned)> fn)
	{
		onLButtonDown = std::move (fn);
	}
	void WindowImpl_Windows::OnRButtonDown (std::function<void (unsigned, unsigned)> fn)
	{
		onRButtonDown = std::move (fn);
	}
	void WindowImpl_Windows::OnLButtonUp (std::function<void (unsigned, unsigned)> fn)
	{
		onLButtonUp = std::move (fn);
	}
	void WindowImpl_Windows::OnRButtonUp (std::function<void (unsigned, unsigned)> fn)
	{
		onRButtonUp = std::move (fn);
	}
	void WindowImpl_Windows::OnKeyDown (std::function<void (char)> fn)
	{
		onKeyDown = std::move (fn);
	}
	void WindowImpl_Windows::OnKeyUp (std::function<void (char)> fn)
	{
		onKeyUp = std::move (fn);
	}

	LRESULT CALLBACK WindowImpl_Windows::_WndProc (
		HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto hwndMapper = HwndManager::hwndMapper ();
		if (hwndMapper == nullptr)
			return DefWindowProc (hwnd, message, wParam, lParam);

		switch (message)
		{
		case WM_MOUSEMOVE:
			if ((*hwndMapper)[hwnd]->onMouseMove)
				(*hwndMapper)[hwnd]->onMouseMove (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			return 0;

		case WM_LBUTTONDOWN:
			if ((*hwndMapper)[hwnd]->onLButtonDown)
				(*hwndMapper)[hwnd]->onLButtonDown (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			return 0;

		case WM_RBUTTONDOWN:
			if ((*hwndMapper)[hwnd]->onRButtonDown)
				(*hwndMapper)[hwnd]->onRButtonDown (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			return 0;

		case WM_LBUTTONUP:
			if ((*hwndMapper)[hwnd]->onLButtonUp)
				(*hwndMapper)[hwnd]->onLButtonUp (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			if ((*hwndMapper)[hwnd]->onClick)
				(*hwndMapper)[hwnd]->onClick (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			return 0;

		case WM_RBUTTONUP:
			if ((*hwndMapper)[hwnd]->onRButtonUp)
				(*hwndMapper)[hwnd]->onRButtonUp (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			if ((*hwndMapper)[hwnd]->onClick)
				(*hwndMapper)[hwnd]->onClick (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			return 0;

		case WM_CHAR:
			if ((*hwndMapper)[hwnd]->onPress)
				(*hwndMapper)[hwnd]->onPress ((char) wParam);
			return 0;
		case WM_KEYDOWN:
			if ((*hwndMapper)[hwnd]->onKeyDown && wParam >= 0x41 && wParam <= 0x5A)
				(*hwndMapper)[hwnd]->onKeyDown ((char) (wParam - 0x41) + 'A');
			return 0;
		case WM_KEYUP:
			if ((*hwndMapper)[hwnd]->onKeyUp && wParam >= 0x41 && wParam <= 0x5A)
				(*hwndMapper)[hwnd]->onKeyUp ((char) (wParam - 0x41) + 'A');
			return 0;

		case WM_SIZE:
			(*hwndMapper)[hwnd]->_cxClient = LOWORD (lParam);
			(*hwndMapper)[hwnd]->_cyClient = HIWORD (lParam);
			if ((*hwndMapper)[hwnd]->onResized)
				(*hwndMapper)[hwnd]->onResized
				((unsigned) (*hwndMapper)[hwnd]->_cxClient,
				(unsigned) (*hwndMapper)[hwnd]->_cyClient);
			return 0;

		case WM_PAINT:
			BeginPaint (hwnd, NULL);
			if ((*hwndMapper)[hwnd]->onRefresh)
				(*hwndMapper)[hwnd]->onRefresh ();
			EndPaint (hwnd, NULL);
			return 0;

		case WM_DESTROY:
			(*hwndMapper)[hwnd]->_hwnd = NULL;
			hwndMapper->erase (hwnd);

			PostQuitMessage (0);
			return 0;
		}
		return DefWindowProc (hwnd, message, wParam, lParam);
	}

	// Context

	const COLORREF GUIContext_Windows::_colorMask = RGB (0, 0, 201);
	const COLORREF GUIContext_Windows::_GetColor (unsigned r,
												  unsigned g,
												  unsigned b)
	{
		const auto mMin =
			[] (const int &a, const int &b) { return a < b ? a : b; };

		r = mMin (255, r);
		g = mMin (255, g);
		b = mMin (255, b);

		if (RGB (r, b, b) != _colorMask)
			return RGB (r, g, b);
		else
			return RGB (r, g, b) + 1;
	}

	GUIContext_Windows::GUIContext_Windows (size_t width, size_t height)
		: _hdc (NULL), _hBitmap (NULL), _w (width), _h (height)
	{
		// Get Root HDC
		auto hdcRoot = GetDC (NULL);

		// New Canvas HDC
		_hdc = CreateCompatibleDC (hdcRoot);
		_hBitmap = CreateCompatibleBitmap (hdcRoot, (int) _w, (int) _h);

		if (!_hdc || !_hBitmap)
		{
			if (_hBitmap) DeleteObject (_hBitmap);
			if (_hdc) DeleteDC (_hdc);
			ReleaseDC (NULL, hdcRoot);
			throw std::runtime_error ("Err_DC_#0_Bitmap");
		}
		SelectObject (_hdc, _hBitmap);

		// Release Root HDC
		ReleaseDC (NULL, hdcRoot);

		// Set HDC init Properties
		SetPen (1, 0, 0, 0);
		SetBrush (true, 0, 0, 0);
		SetFont (18, "Consolas", 0, 0, 0);
		SetBkMode (_hdc, TRANSPARENT);

		// Clear
		Clear ();
	}

	GUIContext_Windows::~GUIContext_Windows ()
	{
		HGDIOBJ hObj;

		hObj = SelectObject (this->_hdc,
			(HBRUSH) GetStockObject (NULL_BRUSH));
		if (hObj != GetStockObject (NULL_BRUSH))
			DeleteObject (hObj);

		hObj = SelectObject (this->_hdc,
			(HPEN) GetStockObject (BLACK_PEN));
		if (hObj != GetStockObject (BLACK_PEN) &&
			hObj != GetStockObject (NULL_PEN))
			DeleteObject (hObj);

		hObj = SelectObject (this->_hdc,
			(HPEN) GetStockObject (SYSTEM_FONT));
		if (hObj != GetStockObject (SYSTEM_FONT))
			DeleteObject (hObj);

		DeleteDC (this->_hdc);
		DeleteObject (this->_hBitmap);
	}

	bool GUIContext_Windows::SetPen (unsigned width,
									 unsigned r,
									 unsigned g,
									 unsigned b)
	{
		HPEN hPen;

		if (width == 0)
			hPen = (HPEN) GetStockObject (NULL_PEN);
		else if (width == 1 && r == 0 && g == 0 && b == 0)
			hPen = (HPEN) GetStockObject (BLACK_PEN);
		else
		{
			hPen = CreatePen (PS_SOLID, width, _GetColor (r, g, b));
			if (!hPen) return false;
		}

		auto hObj = SelectObject (_hdc, hPen);
		if (hObj != GetStockObject (BLACK_PEN) &&
			hObj != GetStockObject (NULL_PEN))
			DeleteObject (hObj);

		return true;
	}

	bool GUIContext_Windows::SetBrush (bool isTransparent,
									   unsigned r,
									   unsigned g,
									   unsigned b)
	{
		HBRUSH hBrush;

		if (isTransparent)
			hBrush = (HBRUSH) GetStockObject (NULL_BRUSH);
		else
		{
			hBrush = CreateSolidBrush (_GetColor (r, g, b));
			if (!hBrush) return false;
		}

		auto hObj = SelectObject (_hdc, hBrush);
		if (hObj != GetStockObject (NULL_BRUSH))
			DeleteObject (hObj);

		return true;
	}

	bool GUIContext_Windows::SetFont (unsigned size,
									  const char *family,
									  unsigned r,
									  unsigned g,
									  unsigned b)
	{
		SetTextColor (this->_hdc, _GetColor (r, g, b));

		auto hFont = CreateFontA (size, 0, 0, 0,
								  FW_DONTCARE, FALSE, FALSE, FALSE,
								  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
								  CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
								  DEFAULT_PITCH, family);
		if (!hFont) return false;

		auto hObj = SelectObject (this->_hdc, hFont);
		if (hObj != GetStockObject (SYSTEM_FONT))
			DeleteObject (hObj);

		return true;
	}

	bool GUIContext_Windows::DrawLine (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		if (!MoveToEx (this->_hdc, xBeg, yBeg, NULL))	return false;
		if (!LineTo (this->_hdc, xEnd, yEnd))			return false;
		return true;
	}

	bool GUIContext_Windows::DrawRect (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		return !!Rectangle (this->_hdc, xBeg, yBeg, xEnd, yEnd);
	}

	bool GUIContext_Windows::DrawElps (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		return !!Ellipse (this->_hdc, xBeg, yBeg, xEnd, yEnd);
	}

	bool GUIContext_Windows::DrawRdRt (int xBeg, int yBeg, int xEnd, int yEnd,
									   int wElps, int hElps)
	{
		return !!RoundRect (this->_hdc, xBeg, yBeg, xEnd, yEnd, wElps, hElps);
	}

	void ConvertAngle (double angle, int &x, int &y)
	{
		const unsigned RADIUS = 1000;
		const double PI = 3.14159265;

		while (angle < 0)
			angle += 360;
		while (angle > 360)
			angle -= 360;

		x = (int) (RADIUS * cos (angle * PI / 180));
		y = (int) (RADIUS * sin (angle * PI / 180));
		y = -y;
	}

	bool GUIContext_Windows::DrawArc (int xLeft, int yTop, int xRight, int yBottom,
									  double angleBeg, double cAngle)
	{
		int xBeg, yBeg, xEnd, yEnd;
		if (cAngle > 0)
		{
			ConvertAngle (angleBeg, xBeg, yBeg);
			ConvertAngle (angleBeg + cAngle, xEnd, yEnd);
		}
		else
		{
			ConvertAngle (angleBeg + cAngle, xBeg, yBeg);
			ConvertAngle (angleBeg, xEnd, yEnd);
		}

		auto xCnt = abs (xRight - xLeft) / 2;
		auto yCnt = abs (yBottom - yTop) / 2;
		xBeg += xCnt;
		xEnd += xCnt;
		yBeg += yCnt;
		yEnd += yCnt;

		return !!Arc (this->_hdc, xLeft, yTop, xRight, yBottom, xBeg, yBeg, xEnd, yEnd);
	}

	bool GUIContext_Windows::DrawChord (int xLeft, int yTop, int xRight, int yBottom,
										double angleBeg, double cAngle)
	{
		int xBeg, yBeg, xEnd, yEnd;
		if (cAngle > 0)
		{
			ConvertAngle (angleBeg, xBeg, yBeg);
			ConvertAngle (angleBeg + cAngle, xEnd, yEnd);
		}
		else
		{
			ConvertAngle (angleBeg + cAngle, xBeg, yBeg);
			ConvertAngle (angleBeg, xEnd, yEnd);
		}

		auto xCnt = abs (xRight - xLeft) / 2;
		auto yCnt = abs (yBottom - yTop) / 2;
		xBeg += xCnt;
		xEnd += xCnt;
		yBeg += yCnt;
		yEnd += yCnt;

		return !!Chord (this->_hdc, xLeft, yTop, xRight, yBottom, xBeg, yBeg, xEnd, yEnd);
	}

	bool GUIContext_Windows::DrawPie (int xLeft, int yTop, int xRight, int yBottom,
									  double angleBeg, double cAngle)
	{
		int xBeg, yBeg, xEnd, yEnd;
		if (cAngle > 0)
		{
			ConvertAngle (angleBeg, xBeg, yBeg);
			ConvertAngle (angleBeg + cAngle, xEnd, yEnd);
		}
		else
		{
			ConvertAngle (angleBeg + cAngle, xBeg, yBeg);
			ConvertAngle (angleBeg, xEnd, yEnd);
		}

		auto xCnt = abs (xRight - xLeft) / 2;
		auto yCnt = abs (yBottom - yTop) / 2;
		xBeg += xCnt;
		xEnd += xCnt;
		yBeg += yCnt;
		yEnd += yCnt;

		return !!Pie (this->_hdc, xLeft, yTop, xRight, yBottom, xBeg, yBeg, xEnd, yEnd);
	}

	bool GUIContext_Windows::DrawTxt (int xBeg, int yBeg, const char * szText)
	{
		return !!TextOutA (this->_hdc, xBeg, yBeg, szText, (int) strlen (szText));
	}

	size_t GUIContext_Windows::GetTxtWidth (const char *szText)
	{
		std::string str (szText);
		size_t ret = 0;
		ABCFLOAT abcFloat;
		for (const auto &ch : str)
		{
			if (!GetCharABCWidthsFloatA (this->_hdc, ch, ch, &abcFloat))
				return 0;
			ret += (int) (abcFloat.abcfA + abcFloat.abcfB + abcFloat.abcfC);
		}
		return ret;
	}

#ifdef _MSC_VER_TODO
	// Todo: Edge Smoothing
	// Using GDI+ to Decode Bmp/Jpeg/Png/Gif...
	bool GUIContext_Windows::DrawImg (const char *fileName, int x, int y,
									  int width, int height,
									  int r, int g, int b)  // r, g, b are obmitted
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		auto ret = false;

		Gdiplus::GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, NULL);
		{
			Gdiplus::Graphics graphics (this->_hdc);
			//graphics.SetSmoothingMode (Gdiplus::SmoothingModeHighQuality);

			auto cchW = MultiByteToWideChar (CP_ACP, MB_COMPOSITE,
											 fileName, -1, NULL, 0);
			wchar_t *fileNameW = new wchar_t[cchW];
			MultiByteToWideChar (CP_ACP, MB_COMPOSITE,
								 fileName, -1, fileNameW, cchW);
			Gdiplus::Image image (fileNameW);
			delete[] fileNameW;

			if (width == -1) width = image.GetWidth ();
			if (height == -1) height = image.GetHeight ();

			Gdiplus::ImageAttributes imAtt;
			if (!(r == -1 || g == -1 || b == -1))
				imAtt.SetColorKey (Gdiplus::Color (r, g, b),
								   Gdiplus::Color (r, g, b),
								   Gdiplus::ColorAdjustTypeBitmap);

			auto status = graphics.DrawImage (&image,
											  Gdiplus::Rect (x, y, width, height),
											  0, 0,
											  image.GetWidth (),
											  image.GetHeight (),
											  Gdiplus::UnitPixel,
											  &imAtt);

			if (status == Gdiplus::Ok)
				ret = true;
		}
		Gdiplus::GdiplusShutdown (gdiplusToken);
		return ret;
	}

#else

	bool GUIContext_Windows::DrawImg (const char *fileName, int x, int y,
									  int width, int height, int r, int g, int b)
	{
		auto hBitmapImag = (HBITMAP) LoadImageA (NULL, fileName, IMAGE_BITMAP,
												 0, 0, LR_LOADFROMFILE);
		if (!hBitmapImag) return false;

		BITMAP bitmap;
		GetObject (hBitmapImag, sizeof (BITMAP), &bitmap);

		auto hdcMemImag = CreateCompatibleDC (this->_hdc);
		if (!hdcMemImag)
		{
			DeleteObject (hBitmapImag);
			return false;
		}
		SelectObject (hdcMemImag, hBitmapImag);

		if (width == -1) width = bitmap.bmWidth;
		if (height == -1) height = bitmap.bmHeight;

		if (r == -1 || g == -1 || b == -1)
		{
			if (!StretchBlt (this->_hdc, x, y, width, height,
							 hdcMemImag, 0, 0,
							 bitmap.bmWidth, bitmap.bmHeight,
							 SRCCOPY))
			{
				DeleteDC (hdcMemImag);
				DeleteObject (hBitmapImag);
				return false;
			}
		}
		else
		{
			auto colorMask = RGB (max (0, min (255, r)),
								  max (0, min (255, g)),
								  max (0, min (255, b)));
			if (!TransparentBlt (this->_hdc, x, y, width, height,
								 hdcMemImag, 0, 0,
								 bitmap.bmWidth, bitmap.bmHeight,
								 colorMask))
			{
				DeleteDC (hdcMemImag);
				DeleteObject (hBitmapImag);
				return false;
			}
		}

		DeleteDC (hdcMemImag);
		DeleteObject (hBitmapImag);
		return true;
	}

#endif

	bool GUIContext_Windows::DrawImgMask (
		const char *srcFile,
		const char *maskFile,
		unsigned width, unsigned height,
		int x_pos, int y_pos,
		unsigned x_src, unsigned y_src,
		unsigned x_msk, unsigned y_msk)
	{
		auto hSrcBmp = (HBITMAP) LoadImageA (NULL, srcFile, IMAGE_BITMAP,
											 0, 0, LR_LOADFROMFILE);
		auto hMskBmp = (HBITMAP) LoadImageA (NULL, maskFile, IMAGE_BITMAP,
											 0, 0, LR_LOADFROMFILE);
		if (!hSrcBmp || !hMskBmp)
		{
			if (hSrcBmp) DeleteObject (hSrcBmp);
			if (hMskBmp) DeleteObject (hMskBmp);
			return false;
		}

		auto hdcSrc = CreateCompatibleDC (this->_hdc);
		auto hdcMsk = CreateCompatibleDC (this->_hdc);
		if (!hdcSrc || !hdcMsk)
		{
			if (hdcSrc) DeleteDC (hdcSrc);
			if (hdcMsk) DeleteDC (hdcMsk);
			DeleteObject (hSrcBmp);
			DeleteObject (hMskBmp);
			return false;
		}
		SelectObject (hdcSrc, hSrcBmp);
		SelectObject (hdcMsk, hMskBmp);

		if (!BitBlt (_hdc, x_pos, y_pos, width, height,
					 hdcMsk, x_msk, y_msk, SRCAND))
		{
			DeleteDC (hdcSrc);
			DeleteDC (hdcMsk);
			DeleteObject (hSrcBmp);
			DeleteObject (hMskBmp);
			return false;
		}

		if (!BitBlt (_hdc, x_pos, y_pos, width, height,
					 hdcSrc, x_src, y_src, SRCPAINT))
		{
			DeleteDC (hdcSrc);
			DeleteDC (hdcMsk);
			DeleteObject (hSrcBmp);
			DeleteObject (hMskBmp);
			return false;
		}

		DeleteDC (hdcSrc);
		DeleteDC (hdcMsk);
		DeleteObject (hSrcBmp);
		DeleteObject (hMskBmp);
		return true;
	}

#ifdef _MSC_VER
	// Using GDI+ to Encode Images

	bool GUIContext_Windows::SaveAsImg (const char *fileName,
										const wchar_t *mime) const
	{
		auto ret = false;
		{
			auto GetEncoderClsid = [] (const WCHAR* format, CLSID* pClsid)
			{
				using namespace Gdiplus;

				UINT  num = 0;          // number of image encoders
				UINT  size = 0;         // size of the image encoder array in bytes

				ImageCodecInfo* pImageCodecInfo = NULL;

				GetImageEncodersSize (&num, &size);
				if (size == 0)
					return -1;  // Failure

				pImageCodecInfo = (ImageCodecInfo*) (malloc (size));
				if (pImageCodecInfo == NULL)
					return -1;  // Failure

				GetImageEncoders (num, size, pImageCodecInfo);

				for (size_t j = 0; j < num; ++j)
				{
					if (wcscmp (pImageCodecInfo[j].MimeType, format) == 0)
					{
						*pClsid = pImageCodecInfo[j].Clsid;
						free (pImageCodecInfo);
						return (int) j;  // Success
					}
				}

				free (pImageCodecInfo);
				return -1;  // Failure
			};

			// Get the CLSID of the PNG encoder.
			CLSID encoderClsid;
			if (-1 == GetEncoderClsid (mime, &encoderClsid))
				return false;

			// Get Bitmap Object.
			auto bmp = Gdiplus::Bitmap::FromHBITMAP (this->_hBitmap, NULL);

			// Get File Name in Wide Chars.
			auto cchW = MultiByteToWideChar (CP_ACP, MB_COMPOSITE,
											 fileName, -1, NULL, 0);
			wchar_t *fileNameW = new wchar_t[cchW];
			MultiByteToWideChar (CP_ACP, MB_COMPOSITE,
								 fileName, -1, fileNameW, cchW);

			// Save to File
			auto status = bmp->Save (fileNameW, &encoderClsid, NULL);

			delete[] fileNameW;
			delete bmp;
			ret = (status == Gdiplus::Ok);
		}
		return ret;
	}

	bool GUIContext_Windows::SaveAsJpg (const char *fileName) const
	{
		return SaveAsImg (fileName, L"image/jpeg");
	}

	bool GUIContext_Windows::SaveAsPng (const char *fileName) const
	{
		return SaveAsImg (fileName, L"image/png");
	}

	bool GUIContext_Windows::SaveAsBmp (const char *fileName) const
	{
		return SaveAsImg (fileName, L"image/bmp");
	}

#else
	// MinGW doesn't support GDI+

	bool GUIContext_Windows::SaveAsImg (
		std::function<bool (BYTE *pData,
							BITMAPINFOHEADER *pbmInfoHeader)> fnSave) const
	{
		// Ref:
		// https://msdn.microsoft.com/en-us/library/dd145119(v=vs.85).aspx

		// Retrieve the bitmap color format, width, and height.
		BITMAP bitmap = { 0 };
		GetObject (this->_hBitmap, sizeof (BITMAP), &bitmap);

		// NOT Support Less than 24 bit
		// There is no RGBQUAD array for these formats:
		// 24-bit-per-pixel or 32-bit-per-pixel
		BITMAPINFOHEADER bmInfoHeader = { 0 };

		// Initialize the fields in the BITMAPINFO structure.
		bmInfoHeader.biSize = sizeof (BITMAPINFOHEADER);
		bmInfoHeader.biWidth = bitmap.bmWidth;
		bmInfoHeader.biHeight = bitmap.bmHeight;
		bmInfoHeader.biPlanes = 1;

		// Convert the color format to a count of bits.
		if (bitmap.bmBitsPixel <= 24)
			bmInfoHeader.biBitCount = 24;
		else
			bmInfoHeader.biBitCount = 32;

		// If the bitmap is not compressed, set the BI_RGB flag.
		bmInfoHeader.biCompression = BI_RGB;

		// Compute the number of bytes in the array of color
		// indices and store the result in biSizeImage.
		// The width must be DWORD aligned unless the bitmap is RLE
		// compressed.
		bmInfoHeader.biSizeImage =
			((bmInfoHeader.biWidth * bmInfoHeader.biBitCount + 31) & ~31)
			/ 8 * bmInfoHeader.biHeight;

		// Set biClrImportant to 0, indicating that all of the
		// device colors are important.
		bmInfoHeader.biClrImportant = 0;

		// Allocate Memory for DIB Data.
		auto hDIB = GlobalAlloc (GHND, bmInfoHeader.biSizeImage);
		auto pData = (BYTE *) GlobalLock (hDIB);

		// Get DIB.
		GetDIBits (this->_hdc, this->_hBitmap, 0, (WORD) this->_h, pData,
			(LPBITMAPINFO) &bmInfoHeader, DIB_RGB_COLORS);

		// Save to Bmp or Png
		// Strategy Pattern :-)
		if (!fnSave (pData, &bmInfoHeader))
		{
			GlobalUnlock (hDIB);
			GlobalFree (hDIB);
			return false;
		}

		// Free Memory.
		GlobalUnlock (hDIB);
		GlobalFree (hDIB);
		return true;
	}

	bool GUIContext_Windows::SaveAsBmp (const char *fileName) const
	{
		auto fnSave = [&] (BYTE *pData,
						   BITMAPINFOHEADER *pbmInfoHeader)
		{
			// Initialize BITMAPFILEHEADER.
			BITMAPFILEHEADER bmFileHeader = { 0 };
			bmFileHeader.bfType = 0x4d42;			// 0x42 = "B" 0x4d = "M"
			bmFileHeader.bfOffBits = sizeof (BITMAPFILEHEADER) +
				sizeof (BITMAPINFOHEADER);
			bmFileHeader.bfSize = bmFileHeader.bfOffBits
				+ pbmInfoHeader->biSizeImage;

			auto hFile = CreateFileA (fileName,
									  GENERIC_READ | GENERIC_WRITE,
									  (DWORD) 0,
									  NULL,
									  CREATE_ALWAYS,
									  FILE_ATTRIBUTE_NORMAL,
									  (HANDLE) NULL);
			if (hFile == INVALID_HANDLE_VALUE)
				return false;

			if (!WriteFile (hFile, &bmFileHeader, sizeof (BITMAPFILEHEADER), NULL, NULL))
			{
				CloseHandle (hFile);
				return false;
			}
			if (!WriteFile (hFile, pbmInfoHeader, sizeof (BITMAPINFOHEADER), NULL, NULL))
			{
				CloseHandle (hFile);
				return false;
			}
			if (!WriteFile (hFile, pData, pbmInfoHeader->biSizeImage, NULL, NULL))
			{
				CloseHandle (hFile);
				return false;
			}
			CloseHandle (hFile);
			return true;
		};

		return SaveAsImg (fnSave);
	}

	bool GUIContext_Windows::SaveAsPng (const char *fileName) const
	{
		auto fnConvertAndSave = [&] (BYTE *pData,
									 BITMAPINFOHEADER *pbmInfoHeader)
		{
			// The amount of scanline bytes is width of image times channels,
			// with extra bytes added if needed to make it a multiple of 4 bytes.
			unsigned numChannels = pbmInfoHeader->biBitCount / 8;
			unsigned scanlineBytes = pbmInfoHeader->biWidth * numChannels;
			if (scanlineBytes % 4 != 0)
				scanlineBytes = (scanlineBytes / 4) * 4 + 4;
			unsigned dataSize = scanlineBytes * pbmInfoHeader->biHeight;

			/*
			There are 3 differences between BMP and the raw image buffer for LodePNG:
			-it's upside down
			-it's in BGR instead of RGB format (or BRGA instead of RGBA)
			-each scanline has padding bytes to make it a multiple of 4 if needed
			The 2D for loop below does all these 3 conversions at once.
			*/
			auto rgbArr = new BYTE[pbmInfoHeader->biWidth * pbmInfoHeader->biHeight * 3];
			for (unsigned y = 0; y < pbmInfoHeader->biHeight; y++)
				for (unsigned x = 0; x < pbmInfoHeader->biWidth; x++)
				{
					// pixel start byte position in the BMP
					unsigned bmpos = (pbmInfoHeader->biHeight - y - 1)
						* scanlineBytes + numChannels * x;

					// pixel start byte position in the rgbaArr
					unsigned newpos = 3 * y * pbmInfoHeader->biWidth + 3 * x;

					// Take 24 bit
					rgbArr[newpos + 0] = pData[bmpos + 2];	//R
					rgbArr[newpos + 1] = pData[bmpos + 1];	//G
					rgbArr[newpos + 2] = pData[bmpos + 0];	//B
				}

			stbi_write_png (fileName, pbmInfoHeader->biWidth,
							pbmInfoHeader->biHeight,
							3, rgbArr, pbmInfoHeader->biWidth * 3);

			delete[] rgbArr;
			return true;
		};

		return SaveAsImg (fnConvertAndSave);
	}

	bool GUIContext_Windows::SaveAsJpg (const char * fileName) const
	{
		// Not Implemented
		return false;
	}
#endif

	void GUIContext_Windows::Clear ()
	{
		RECT rect;
		rect.left = rect.top = 0;
		rect.right = (LONG) this->_w;
		rect.bottom = (LONG) this->_h;

		auto hBrush = CreateSolidBrush (_colorMask);
		FillRect (this->_hdc, &rect, hBrush);
		DeleteObject (hBrush);
	}

	void GUIContext_Windows::PaintOnContext (GUIContext *parentContext,
											 size_t x, size_t y) const
	{
		// Assume that context is GUIContext_Windows
		auto _context = static_cast<const GUIContext_Windows *> (parentContext);
		TransparentBlt (_context->_hdc, (int) x, (int) y, (int) this->_w, (int) this->_h,
						this->_hdc, 0, 0, (int) this->_w, (int) this->_h,
						GUIContext_Windows::_colorMask);
	}

	// MsgBox

	void MsgBox_Impl (const char * szTxt, const char * szCap)
	{
		MessageBoxA (NULL, szTxt, szCap, MB_OK);
	}
}
