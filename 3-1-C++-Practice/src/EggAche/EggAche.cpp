//
// Common Implementation of EggAche Graphics Library
// By BOT Man, 2016
//

#include <memory>

#include "EggAche.h"
#include "EggAche_Impl.h"

namespace EggAche
{
	EggAche_Impl::GUIFactory *NewGUIFactory ()
	{
#ifdef EGGACHE_WINDOWS
		return new EggAche_Impl::GUIFactory_Windows ();
#endif
#ifdef EGGACHE_XWINDOW
		return new EggAche_Impl::GUIFactory_XWindow ();
#endif
	}

	Window::Window (size_t width,
					size_t height,
					const char *cap_string)
		: bgCanvas (nullptr), windowImpl (nullptr)
	{
		std::unique_ptr<EggAche_Impl::GUIFactory> guiFactory (
			NewGUIFactory ());

		windowImpl = std::unique_ptr<EggAche_Impl::WindowImpl> (
			guiFactory->NewWindow (width, height, cap_string));
		windowImpl->OnRefresh (std::bind (&Window::Refresh, this));
	}

	void Window::SetBackground (Canvas *canvas)
	{
		bgCanvas = canvas;
	}

	void Window::ClearBackground ()
	{
		bgCanvas = nullptr;
	}

	bool Window::Refresh ()
	{
		if (IsClosed () || !this->bgCanvas)
			return false;

		if (!this->bgCanvas->isLatest)
			this->bgCanvas->Buffering ();
		return windowImpl->Draw (this->bgCanvas->buffer.get (), 0, 0);
	}

	bool Window::IsClosed () const
	{
		return windowImpl->IsClosed ();
	}

	// Basic Events
	void Window::OnClick (std::function<void (Window *, unsigned, unsigned)> fn)
	{
		windowImpl->OnClick ([=] (unsigned x, unsigned y)
		{ fn (this, x, y); });
	}
	void Window::OnPress (std::function<void (Window *, char)> fn)
	{
		windowImpl->OnPress ([=] (char ch)
		{ fn (this, ch); });
	}
	void Window::OnResized (std::function<void (Window *, unsigned, unsigned)> fn)
	{
		windowImpl->OnResized ([=] (unsigned x, unsigned y)
		{ fn (this, x, y); });
	}

	// New Events
	void Window::OnMouseMove (std::function<void (Window *, unsigned, unsigned)> fn)
	{
		windowImpl->OnMouseMove ([=] (unsigned x, unsigned y)
		{ fn (this, x, y); });
	}
	void Window::OnLButtonDown (std::function<void (Window *, unsigned, unsigned)> fn)
	{
		windowImpl->OnLButtonDown ([=] (unsigned x, unsigned y)
		{ fn (this, x, y); });
	}
	void Window::OnRButtonDown (std::function<void (Window *, unsigned, unsigned)> fn)
	{
		windowImpl->OnRButtonDown ([=] (unsigned x, unsigned y)
		{ fn (this, x, y); });
	}
	void Window::OnLButtonUp (std::function<void (Window *, unsigned, unsigned)> fn)
	{
		windowImpl->OnLButtonUp ([=] (unsigned x, unsigned y)
		{ fn (this, x, y); });
	}
	void Window::OnRButtonUp (std::function<void (Window *, unsigned, unsigned)> fn)
	{
		windowImpl->OnRButtonUp ([=] (unsigned x, unsigned y)
		{ fn (this, x, y); });
	}
	void Window::OnKeyDown (std::function<void (Window *, char)> fn)
	{
		windowImpl->OnKeyDown ([=] (char ch)
		{ fn (this, ch); });
	}
	void Window::OnKeyUp (std::function<void (Window *, char)> fn)
	{
		windowImpl->OnKeyUp ([=] (char ch)
		{ fn (this, ch); });
	}

	Canvas::Canvas (size_t width, size_t height,
					int pos_x, int pos_y)
		: isLatest (false),
		x (pos_x), y (pos_y), w (width), h (height)
	{
		std::unique_ptr<EggAche_Impl::GUIFactory> guiFactory (
			NewGUIFactory ());
		context = std::unique_ptr<EggAche_Impl::GUIContext>(
			guiFactory->NewGUIContext (width, height));
	}

	void Canvas::RecursiveDraw (EggAche_Impl::GUIContext *parentContext,
								size_t x, size_t y) const
	{
		// Actual Position of this Canvas
		this->context->PaintOnContext (parentContext, x, y);

		for (auto subCanvas : this->subCanvases)
			subCanvas->RecursiveDraw (parentContext,
									  x + subCanvas->x, y + subCanvas->y);
	}

	void Canvas::RecursiveInvalidate ()
	{
		this->isLatest = false;
		for (auto& pa : this->parCanvases)
			pa->RecursiveInvalidate ();
	}

	void Canvas::Buffering ()
	{
		std::unique_ptr<EggAche_Impl::GUIFactory> guiFactory (
			NewGUIFactory ());
		buffer = std::unique_ptr<EggAche_Impl::GUIContext> (
			guiFactory->NewGUIContext (this->w, this->h));

		buffer->SetBrush (false, 255, 255, 255);
		buffer->DrawRect (-10, -10, (int) (this->w + 10), (int) (this->h + 10));

		this->RecursiveDraw (buffer.get (), 0, 0);
		isLatest = true;
	}

	void Canvas::operator+= (Canvas *canvas)
	{
		if (canvas != this)
		{
			this->RecursiveInvalidate ();

			subCanvases.push_back (canvas);
			subCanvases.unique ();

			canvas->parCanvases.push_back (this);
			canvas->parCanvases.unique ();
		}
	}

	void Canvas::operator-= (Canvas *canvas)
	{
		this->RecursiveInvalidate ();
		subCanvases.remove (canvas);
		canvas->parCanvases.remove (this);
	}

	void Canvas::Move (int scale_x, int scale_y)
	{
		this->RecursiveInvalidate ();
		x = x + scale_x;
		y = y + scale_y;
	}

	void Canvas::MoveTo (int pos_x, int pos_y)
	{
		this->RecursiveInvalidate ();
		x = pos_x;
		y = pos_y;
	}

	bool Canvas::SetPen (unsigned width,
						 unsigned r, unsigned g, unsigned b)
	{
		return context->SetPen (width, r, g, b);
	}

	bool Canvas::SetBrush (bool isTransparent,
						   unsigned r, unsigned g, unsigned b)
	{
		return context->SetBrush (isTransparent, r, g, b);
	}

	bool Canvas::SetFont (unsigned size, const char *family,
						  unsigned r, unsigned g, unsigned b)
	{
		return context->SetFont (size, family, r, g, b);
	}

	void Canvas::Clear ()
	{
		this->RecursiveInvalidate ();
		context->Clear ();
	}

	bool Canvas::DrawLine (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		this->RecursiveInvalidate ();
		return context->DrawLine (xBeg, yBeg, xEnd, yEnd);
	}

	bool Canvas::DrawRect (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		this->RecursiveInvalidate ();
		return context->DrawRect (xBeg, yBeg, xEnd, yEnd);
	}

	bool Canvas::DrawElps (int xBeg, int yBeg, int xEnd, int yEnd)
	{
		this->RecursiveInvalidate ();
		return context->DrawElps (xBeg, yBeg, xEnd, yEnd);
	}

	bool Canvas::DrawRdRt (int xBeg, int yBeg, int xEnd, int yEnd,
						   int wElps, int hElps)
	{
		this->RecursiveInvalidate ();
		return context->DrawRdRt (xBeg, yBeg, xEnd, yEnd,
								  wElps, hElps);
	}

	bool Canvas::DrawArc (int xLeft, int yTop, int xRight, int yBottom,
						  double angleBeg, double cAngle)
	{
		this->RecursiveInvalidate ();
		return context->DrawArc (xLeft, yTop, xRight, yBottom,
								 angleBeg, cAngle);
	}

	bool Canvas::DrawChord (int xLeft, int yTop, int xRight, int yBottom,
							double angleBeg, double cAngle)
	{
		this->RecursiveInvalidate ();
		return context->DrawChord (xLeft, yTop, xRight, yBottom,
								   angleBeg, cAngle);
	}

	bool Canvas::DrawPie (int xLeft, int yTop, int xRight, int yBottom,
						  double angleBeg, double cAngle)
	{
		this->RecursiveInvalidate ();
		return context->DrawPie (xLeft, yTop, xRight, yBottom,
								 angleBeg, cAngle);
	}

	bool Canvas::DrawTxt (int xBeg, int yBeg, const char *szText)
	{
		this->RecursiveInvalidate ();
		return context->DrawTxt (xBeg, yBeg, szText);
	}

	size_t Canvas::GetTxtWidth (const char *szText)
	{
		return context->GetTxtWidth (szText);
	}

	bool Canvas::DrawImg (const char *fileName, int x, int y)
	{
		this->RecursiveInvalidate ();
		return context->DrawImg (fileName, x, y);
	}

	bool Canvas::DrawImg (const char *fileName, int x, int y,
						  unsigned width, unsigned height)
	{
		this->RecursiveInvalidate ();
		return context->DrawImg (fileName, x, y, width, height);
	}

	bool Canvas::DrawImgMask (const char *srcFile,
							  const char *maskFile,
							  unsigned width, unsigned height,
							  int x_pos, int y_pos,
							  unsigned x_src, unsigned y_src,
							  unsigned x_msk, unsigned y_msk)
	{
		this->RecursiveInvalidate ();
		return context->DrawImgMask (srcFile, maskFile,
									 width, height,
									 x_pos, y_pos,
									 x_src, y_src,
									 x_msk, y_msk);
	}

	bool Canvas::SaveAsJpg (const char * fileName)
	{
		if (!isLatest)
			this->Buffering ();
		return buffer->SaveAsJpg (fileName);
	}

	bool Canvas::SaveAsPng (const char * fileName)
	{
		if (!isLatest)
			this->Buffering ();
		return buffer->SaveAsPng (fileName);
	}

	bool Canvas::SaveAsBmp (const char *fileName)
	{
		if (!isLatest)
			this->Buffering ();
		return buffer->SaveAsBmp (fileName);
	}

	void MsgBox (const char *szTxt, const char *szCap)
	{
		EggAche_Impl::MsgBox_Impl (szTxt, szCap);
	}
}