//
// Implementation Interface of EggAche Graphics Library
// By BOT Man, 2016
//

#ifndef EGGACHE_GL_IMPL
#define EGGACHE_GL_IMPL

#include <functional>
#include <utility>

namespace EggAche_Impl
{
	class GUIContext;

	class WindowImpl
	{
	public:
		virtual ~WindowImpl () {}

		virtual bool Draw (const GUIContext *context,
						   size_t x, size_t y) = 0;

		virtual std::pair<size_t, size_t> GetSize () = 0;
		virtual bool IsClosed () const = 0;

		virtual void OnClick (std::function<void (unsigned, unsigned)> fn) = 0;
		virtual void OnPress (std::function<void (char)> fn) = 0;
		virtual void OnResized (std::function<void (unsigned, unsigned)> fn) = 0;
		virtual void OnRefresh (std::function<void ()> fn) = 0;

		virtual void OnMouseMove (std::function<void (unsigned, unsigned)> fn) = 0;
		virtual void OnLButtonDown (std::function<void (unsigned, unsigned)> fn) = 0;
		virtual void OnRButtonDown (std::function<void (unsigned, unsigned)> fn) = 0;
		virtual void OnLButtonUp (std::function<void (unsigned, unsigned)> fn) = 0;
		virtual void OnRButtonUp (std::function<void (unsigned, unsigned)> fn) = 0;
		virtual void OnKeyDown (std::function<void (char)> fn) = 0;
		virtual void OnKeyUp (std::function<void (char)> fn) = 0;
	};

	class GUIContext
	{
	public:
		virtual ~GUIContext () {}

		virtual bool SetPen (unsigned width,
							 unsigned r = 0,
							 unsigned g = 0,
							 unsigned b = 0) = 0;

		virtual bool SetBrush (bool isTransparent,
							   unsigned r,
							   unsigned g,
							   unsigned b) = 0;

		virtual bool SetFont (unsigned size,
							  const char *family,
							  unsigned r,
							  unsigned g,
							  unsigned b) = 0;

		virtual bool DrawLine (int xBeg, int yBeg, int xEnd, int yEnd) = 0;

		virtual bool DrawRect (int xBeg, int yBeg, int xEnd, int yEnd) = 0;

		virtual bool DrawElps (int xBeg, int yBeg, int xEnd, int yEnd) = 0;

		virtual bool DrawRdRt (int xBeg, int yBeg,
							   int xEnd, int yEnd, int wElps, int hElps) = 0;

		virtual bool DrawArc (int xLeft, int yTop, int xRight, int yBottom,
							  double angleBeg, double cAngle) = 0;

		virtual bool DrawChord (int xLeft, int yTop, int xRight, int yBottom,
								double angleBeg, double cAngle) = 0;

		virtual bool DrawPie (int xLeft, int yTop, int xRight, int yBottom,
							  double angleBeg, double cAngle) = 0;

		virtual bool DrawTxt (int xBeg, int yBeg, const char *szText) = 0;
		virtual size_t GetTxtWidth (const char *szText) = 0;

		virtual bool DrawImg (const char *fileName,
							  int x = 0, int y = 0,
							  int width = -1, int height = -1,
							  int r = -1,
							  int g = -1,
							  int b = -1) = 0;

		virtual bool DrawImgMask (const char *srcFile,
								  const char *maskFile,
								  unsigned width, unsigned height,
								  int x_pos, int y_pos,
								  unsigned x_src, unsigned y_src,
								  unsigned x_msk, unsigned y_msk) = 0;

		virtual bool SaveAsJpg (const char *fileName) const = 0;
		virtual bool SaveAsPng (const char *fileName) const = 0;
		virtual bool SaveAsBmp (const char *fileName) const = 0;

		virtual void Clear () = 0;

		virtual void PaintOnContext (GUIContext *,
									 size_t x, size_t y) const = 0;
	};

	void MsgBox_Impl (const char *szTxt, const char *szCap);

	class GUIFactory
	{
	public:
		virtual WindowImpl *NewWindow (size_t width, size_t height,
									   const char *cap_string) = 0;
		virtual GUIContext *NewGUIContext (size_t width, size_t height) = 0;
	};

	class GUIFactory_Windows : public GUIFactory
	{
	public:
		WindowImpl *NewWindow (size_t width, size_t height,
							   const char *cap_string) override;
		GUIContext *NewGUIContext (size_t width, size_t height) override;
	};

	class GUIFactory_XWindow : public GUIFactory
	{
	public:
		WindowImpl *NewWindow (size_t width, size_t height,
							   const char *cap_string) override;
		GUIContext *NewGUIContext (size_t width, size_t height) override;
	};
}

#endif  //EGGACHE_GL_IMPL