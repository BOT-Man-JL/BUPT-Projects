//
// Implementation of EggAche Graphics Library
// By BOT Man, 2016
//

#include <windows.h>
#include <windowsx.h>
#pragma comment (lib, "Msimg32.lib")

#include "EggAche.h"
#include "EggAche_impl.h"

// Defined in windows.h but not in MinGW

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

namespace EggAche					// Window
{
	Window::Window (int width, int height, const char * cap_string,
					ONCLICK fnClick, ONPRESS fnPress)
		: _Window (fnClick, fnPress, cap_string, width, height)
	{}

	Window::Window (const Window &origin)
		: _Window (origin), ly (origin.ly)
	{}

	Window::~Window ()
	{}

	void Window::AddEgg (Egg & e)
	{
		ly.push_back (&e);
		ly.unique ();
	}

	std::list<Egg*>& Window::GetLayer ()
	{
		return this->ly;
	}

	bool Window::Refresh ()
	{
		if (this->_fClosed)
			return false;

		// Validate
		_Window::Refresh ();

		RECT	rect;
		HDC		hdc, hdcWnd;
		HBITMAP	hBitmap;

		hdcWnd = GetDC (this->_hwnd);
		if (!hdcWnd) return false;

		hdc = CreateCompatibleDC (hdcWnd);
		hBitmap = CreateCompatibleBitmap (hdcWnd, this->_cxClient, this->_cyClient);

		if (!hdc || !hBitmap)
		{
			if (hBitmap) DeleteObject (hBitmap);
			if (hdc) DeleteDC (hdc);
			ReleaseDC (this->_hwnd, hdcWnd);
			return false;
		}
		SelectObject (hdc, hBitmap);

		rect.left = rect.top = 0;
		rect.right = this->_cxClient;
		rect.bottom = this->_cyClient;
		FillRect (hdc, &rect, (HBRUSH) GetStockObject (WHITE_BRUSH));

		SaveDC (hdc);
		SetMapMode (hdc, MM_ANISOTROPIC);
		SetWindowExtEx (hdc, this->_cxCanvas, this->_cyCanvas, NULL);
		SetViewportExtEx (hdc, this->_cxClient, this->_cyClient, NULL);

		for (auto i : this->ly)
			if (!i->_PaintOn (hdc, i->GetX (), i->GetY ()))
			{
				if (hBitmap) DeleteObject (hBitmap);
				if (hdc) DeleteDC (hdc);
				ReleaseDC (this->_hwnd, hdcWnd);
				return false;
			}

		RestoreDC (hdc, -1);
		if (!BitBlt (hdcWnd, 0, 0, this->_cxClient, this->_cyClient,
					 hdc, 0, 0, SRCCOPY))
		{
			DeleteObject (hBitmap);
			DeleteDC (hdc);
			ReleaseDC (this->_hwnd, hdcWnd);
			return false;
		}

		DeleteObject (hBitmap);
		DeleteDC (hdc);
		ReleaseDC (this->_hwnd, hdcWnd);
		return true;
	}

	bool Window::IsClosed () const
	{
		return this->_fClosed;
	}
}

namespace EggAche					// MsgBox
{
	void MsgBox (const char *szTxt, const char *szCap)
	{
		MessageBoxA (NULL, szTxt, szCap, MB_OK);
	}
}

namespace EggAche					// Egg
{
	bool Egg::SetPen (unsigned int width, unsigned int r, unsigned int g, unsigned int b)
	{
		HPEN		hPen;
		HGDIOBJ		hObj;
		COLORREF	color = _GetColor (r, g, b);

		if (r == -1 || g == -1 || b == -1 || width == 0)
		{
			hObj = SelectObject (this->_hdc, (HPEN) GetStockObject (NULL_PEN));
			if (hObj != GetStockObject (BLACK_PEN) && hObj != GetStockObject (NULL_PEN))
				DeleteObject (hObj);
			return true;
		}

		hPen = CreatePen (PS_SOLID, max (0, width), color);
		if (!hPen) return false;

		hObj = SelectObject (this->_hdc, hPen);
		if (hObj != GetStockObject (BLACK_PEN) && hObj != GetStockObject (NULL_PEN))
			DeleteObject (hObj);

		return true;
	}

	bool Egg::SetBrush (unsigned int r, unsigned int g, unsigned int b)
	{
		HBRUSH		hBrush;
		HGDIOBJ		hObj;
		COLORREF	color = _GetColor (r, g, b);

		if (r == -1 || g == -1 || b == -1)
		{
			hObj = SelectObject (this->_hdc, (HPEN) GetStockObject (NULL_BRUSH));
			if (hObj != GetStockObject (NULL_BRUSH))
				DeleteObject (hObj);
			return true;
		}

		hBrush = CreateSolidBrush (color);
		if (!hBrush) return false;

		hObj = SelectObject (this->_hdc, hBrush);
		if (hObj != GetStockObject (NULL_BRUSH))
			DeleteObject (hObj);

		return true;
	}

	bool Egg::DrawLine (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		if (!MoveToEx (this->_hdc, xBeg, yBeg, NULL))	return false;
		if (!LineTo (this->_hdc, xEnd, yEnd))			return false;
		return true;
	}

	bool Egg::DrawRect (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		return !!Rectangle (this->_hdc, xBeg, yBeg, xEnd, yEnd);
	}

	bool Egg::DrawElps (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		return !!Ellipse (this->_hdc, xBeg, yBeg, xEnd, yEnd);
	}

	bool Egg::DrawRdRt (int xBeg, int yBeg, int xEnd, int yEnd, int wElps, int hElps)
	{
		return !!RoundRect (this->_hdc, xBeg, yBeg, xEnd, yEnd, wElps, hElps);
	}

	bool Egg::DrawArc (int xLeft, int yTop, int xRight, int yBottom, int xBeg, int yBeg, int xEnd, int yEnd)
	{
		return !!Arc (this->_hdc, xLeft, yTop, xRight, yBottom, xBeg, yBeg, xEnd, yEnd);
	}

	bool Egg::DrawChord (int xLeft, int yTop, int xRight, int yBottom, int xBeg, int yBeg, int xEnd, int yEnd)
	{
		return !!Chord (this->_hdc, xLeft, yTop, xRight, yBottom, xBeg, yBeg, xEnd, yEnd);
	}

	bool Egg::DrawPie (int xLeft, int yTop, int xRight, int yBottom, int xBeg, int yBeg, int xEnd, int yEnd)
	{
		return !!Pie (this->_hdc, xLeft, yTop, xRight, yBottom, xBeg, yBeg, xEnd, yEnd);
	}

	bool Egg::DrawTxt (int xBeg, int yBeg, const char * szText)
	{
		return !!TextOutA (this->_hdc, xBeg, yBeg, szText, (int) strlen (szText));
	}

	bool Egg::DrawBmp (const char * szPath, int x, int y,
					   int width, int height, int r, int g, int b)
	{
		HDC			hdcMemImag;
		HBITMAP		hBitmapImag;
		BITMAP		bitmap;
		COLORREF	colorMask;

		hBitmapImag = (HBITMAP) LoadImageA (NULL, szPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (!hBitmapImag) return false;
		GetObject (hBitmapImag, sizeof (BITMAP), &bitmap);

		hdcMemImag = CreateCompatibleDC (this->_hdc);
		if (!hdcMemImag)
		{
			DeleteObject (hBitmapImag);
			return false;
		}
		SelectObject (hdcMemImag, hBitmapImag);

		if (width == -1) width = this->_w;
		if (height == -1) height = this->_h;

		if (r == -1 || g == -1 || b == -1)
		{
			if (!StretchBlt (this->_hdc, x, y, width, height,
							 hdcMemImag, 0, 0, bitmap.bmWidth, bitmap.bmHeight,
							 SRCCOPY))
			{
				DeleteDC (hdcMemImag);
				DeleteObject (hBitmapImag);
				return false;
			}
		}
		else
		{
			colorMask = RGB (max (0, min (255, r)),
							 max (0, min (255, g)),
							 max (0, min (255, b)));
			if (!TransparentBlt (this->_hdc, x, y, width, height,
								 hdcMemImag, 0, 0, bitmap.bmWidth, bitmap.bmHeight,
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

	void Egg::Clear ()
	{
		RECT rect;
		rect.left = rect.top = 0;
		rect.right = this->_w;
		rect.bottom = this->_h;

		auto hBrush = CreateSolidBrush (_colorMask);
		FillRect (this->_hdc, &rect, hBrush);
		DeleteObject (hBrush);
	}

	//======================Basic Function=====================

	Egg::Egg (unsigned int width = 60, unsigned int height = 60, int pos_x, int pos_y)
		: x (pos_x), y (pos_y), _DrawContext (width, height)
	{}

	void Egg::Move (int scale_x, int scale_y)
	{
		this->x = max (INT_MIN, min (INT_MAX, this->x + scale_x));
		this->y = max (INT_MIN, min (INT_MAX, this->y + scale_y));
	}

	void Egg::MoveTo (int pos_x, int pos_y)
	{
		this->x = pos_x;
		this->y = pos_y;
	}

	int Egg::GetX () const
	{
		return this->x;
	}

	int Egg::GetY () const
	{
		return this->y;
	}
}
