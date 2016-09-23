//
// Implementation Interface of EggAche Graphics Library
// By BOT Man, 2016
//

#ifndef EGGACHE_GL_IMPL
#define EGGACHE_GL_IMPL

#include <windows.h>
#include <string>

namespace EggAche
{
	typedef void (*ONCLICK)(int, int);
	typedef void (*ONPRESS)(char);

	double _dRatio ();
	
	class _Window
	{
	public:
		_Window (ONCLICK fnClick, ONPRESS fnPress,
				 const char * cap_string, int width, int height);
		_Window (const _Window &origin);
		~_Window ();

		HWND	_hwnd;
		HANDLE	_hEvent;
		bool	_fFailed;

		int		_cxClient, _cyClient;
		int		_cxCanvas, _cyCanvas;

		std::string		_szCap;
		ONCLICK			_fnOnClick;
		ONPRESS			_fnOnPress;
		bool			_fClosed;
		virtual bool	Refresh ();

		static void WINAPI _NewWindow_Thread (_Window *pew);
		static LRESULT CALLBACK _WndProc (HWND, UINT, WPARAM, LPARAM);
	};

	class _DrawContext
	{
	public:
		HDC _hdc;
		HBITMAP _hBitmap;
		unsigned int _w, _h;

		_DrawContext (unsigned int width, unsigned int height);
		_DrawContext (const _DrawContext &origin);
		~_DrawContext ();

		const bool _PaintOn (HDC hdcWnd, int x, int y);

		static const COLORREF _colorMask;
		static const COLORREF _GetColor (int r, int g, int b);
	};
}

#endif  //EGGACHE_GL_IMPL