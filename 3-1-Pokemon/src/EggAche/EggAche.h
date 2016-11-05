//
// Interface of EggAche Graphics Library
// By BOT Man, 2016
//

#ifndef EGGACHE_GL
#define EGGACHE_GL

#define EGGACHE_WINDOWS
//#define EGGACHE_XWINDOW

#include <functional>
#include <memory>
#include <list>
#include "EggAche_Impl.h"

namespace EggAche
{
	class Canvas;

	//===================EggAche Window========================

	class Window
	{
	public:
		Window (size_t width = 640, size_t height = 480,		// Size at least 240 * 120
				const char *cap_string = "Hello EggAche");		// Caption String
		// Remarks:
		// 1. Create a Window of Logic Size width * height with Caption cap_string;
		// 2. When an error occurs, throw std::runtime_error

		void SetBackground (Canvas *canvas);				// Set Background Canvas
		void ClearBackground ();							// Set BgCanvas to NULL
		bool Refresh ();									// Refresh the Window
		// Remarks:
		// 1. Call this function everytime you want to Render
		//    the Background Canvas and its Sub Canvases
		// 2. Why NO Auto Refresh?
		//    Auto Refresh will cost much more resource if there are too many
		//    drawing changes at a time...

		bool IsClosed () const;								// Is Window closed
		// Remarks:
		// If the Window has been Closed by User, it will return false

		void OnClick (std::function<void (Window *, unsigned, unsigned)> fn);
		void OnPress (std::function<void (Window *, char)> fn);
		void OnResized (std::function<void (Window *, unsigned, unsigned)> fn);
		// Remarks:
		// If you click or press a key on Window, back-end will callback fn;
		// 1. Calling onClick with (unsigned x, unsigned y) means point (x, y) is Clicked;
		// 2. Calling onPress with (char ch) means character 'ch' is Inputted;
		// 3. Calling onResized with (unsigned x, unsigned y) means Size is x * y;
		// 4. These functions will be called by Background Threads, so LOCK by yourself;

		void OnMouseMove (std::function<void (Window *, unsigned, unsigned)> fn);
		void OnLButtonDown (std::function<void (Window *, unsigned, unsigned)> fn);
		void OnRButtonDown (std::function<void (Window *, unsigned, unsigned)> fn);
		void OnLButtonUp (std::function<void (Window *, unsigned, unsigned)> fn);
		void OnRButtonUp (std::function<void (Window *, unsigned, unsigned)> fn);
		// Remarks:
		// Params are Similar to OnClick

		void OnKeyDown (std::function<void (Window *, char)> fn);
		void OnKeyUp (std::function<void (Window *, char)> fn);
		// Remarks:
		// Params are Similar to OnPress
		// Only Support 'A' - 'Z' in this Version (If needed, you can Add by yourself :-)

	private:
		std::unique_ptr<EggAche_Impl::WindowImpl> windowImpl;	// Window Impl Bridge
		Canvas *bgCanvas;										// Background Canvas

		Window (const Window &) = delete;					// Not allow to copy
		void operator= (const Window &) = delete;			// Not allow to copy
	};

	//===========================Canvas===========================

	class Canvas
	{
	public:
		Canvas (size_t width, size_t height,				// Canvas' size
				int pos_x = 0, int pos_y = 0);					// Canvas' initial postion
		   // Remarks:
		   // When an error occurs, throw std::runtime_error

		void Buffering ();									// Buffering Canvas and Sub Canvases
		// Remarks:
		// Buffer this Canvas and its Sub Canvases, for Window.Refresh ();
		// Window.Refresh () will check if the Background is Buffered before Rendering;

		void operator+= (Canvas *canvas);					// Add Sub Canvases
		void operator-= (Canvas *canvas);					// Remove Sub Canvases
		// Remarks:
		// Associated Canvases will be rendered after this Canvas

		void MoveTo (int pos_x, int pos_y);					// Place Canvas to (pos_x, pos_y)
		void Move (int scale_x, int scale_y);				// Move Canvas
		// Remarks:
		// If scale_x > 0, Canvas will be moved right scale_x units; else moved left -scale_x;
		// Similarly move scale_y;

		bool SetPen (unsigned width,						// Pen Width
					 unsigned r = 0,						// Pen Color
					 unsigned g = 0,
					 unsigned b = 0);
		bool SetBrush (bool isTransparent,					// Is Transparent
					   unsigned r,							// Brush Color
					   unsigned g,
					   unsigned b);
		bool SetFont (unsigned size = 18,					// Font Size
					  const char *family = "Consolas",		// Font Family
					  unsigned r = 0,						// Font Color
					  unsigned g = 0,
					  unsigned b = 0);
		// Remarks:
		// If SetBrush's isTransparent is set, the Color will be ignored

		void Clear ();										// Clear the Canvas
		// Remarks:
		// Erase the content in Canvas

		bool DrawTxt (int xBeg, int yBeg, const char *szText);
		// Remarks:
		// Draw the szText with a upper left point (xBeg, yBeg)

		size_t GetTxtWidth (const char *szText);			// Estimate Text Width
		// Remarks:
		// Return the Estimated Width of the String in Current Font;
		// Return 0 if an Error occurs;

		bool DrawImg (const char *fileName,					// Source: "path/*.bmp"
					  int x, int y);						// Position to paste in Canvas

		bool DrawImg (const char *fileName,					// Source: "path/*.bmp"
					  int x, int y,							// Position to paste in Canvas
					  unsigned width, unsigned height);		// Size to paste in Canvas
		// Remarks:
		// 1. The Bitmap file will be stretched into width * height in Canvas;
		// 2. Only Support Bitmap (.bmp) file... currently

		bool DrawImgMask (const char *srcFile,				// Source: "path/*.bmp"
						  const char *maskFile,				// Mask: "path/*.bmp"
						  unsigned width, unsigned height,	// Size of the part to Draw
						  int x_pos, int y_pos,				// Position to paste in Canvas
						  unsigned x_src, unsigned y_src,	// Position in srcFile
						  unsigned x_msk, unsigned y_msk);	// Position in maskFile
		// Remarks:
		// 1. The Bitmap file will NOT be stretched;
		// 2. The Mask is usually Black (foreground) and White (background)

		bool DrawLine (int xBeg, int yBeg, int xEnd, int yEnd);
		// Remarks:
		// Draw a Line from (xBeg, yBeg) to (xEnd, yEnd);

		bool DrawRect (int xBeg, int yBeg, int xEnd, int yEnd);
		// Remarks:
		// Draw a Rectangle
		//   (xBeg, yBeg)-----------------(xEnd, yBeg)
		//        |                            |
		//        |                            |
		//        |                            |
		//        |                            |
		//   (xBeg, yEnd)-----------------(xEnd, yEnd);

		bool DrawElps (int xBeg, int yBeg, int xEnd, int yEnd);
		// Remarks:
		// Draw the Ellipse in such an imaginary bounding box;

		bool DrawRdRt (int xBeg, int yBeg,
					   int xEnd, int yEnd, int wElps, int hElps);
		// Remarks:
		// Draw a round conner Rectangle;
		// wElps, hElps = the width/height of the rounded corners Ellipse;

		bool DrawArc (int xLeft, int yTop, int xRight, int yBottom,
					  double angleBeg, double cAngle);
		// Remarks:
		// The points (xLeft, yTop) and (xRight, yBottom) specify the bounding box;
		// An Ellipse formed by the bounding box defines the curve of the Arc;
		// 'angleBeg' specifies the start of the Arc relative to the 3-o'clock position;
		// The Arc extends from 'angleBeg' to 'angleBeg + cAngle';
		// Positive 'cAngle' indicates counterclockwise motion, and Negative clockwise;

		bool DrawChord (int xLeft, int yTop, int xRight, int yBottom,
						double angleBeg, double cAngle);
		// Remarks:
		// The points (xLeft, yTop) and (xRight, yBottom) specify the bounding box;
		// An Ellipse formed by the bounding box defines the curve of the Chord;
		// 'angleBeg' specifies the start of the Curve relative to the 3-o'clock position;
		// The Curve extends from 'angleBeg' to 'angleBeg + cAngle';
		// Positive 'cAngle' indicates counterclockwise motion, and Negative clockwise;
		// The Chord is closed by drawing a line from the intersection of the first radial
		// and the curve to the intersection of the second radial and the curve;

		bool DrawPie (int xLeft, int yTop, int xRight, int yBottom,
					  double angleBeg, double cAngle);
		// Remarks:
		// The points (xLeft, yTop) and (xRight, yBottom) specify the bounding box;
		// An Ellipse formed by the bounding box defines the curve of the Pie;
		// 'angleBeg' specifies the start of the Curve relative to the 3-o'clock position;
		// The Curve extends from 'angleBeg' to 'angleBeg + cAngle';
		// Positive 'cAngle' indicates counterclockwise motion, and Negative clockwise;
		// The Pie is closed by drawing 2 Radius of the Angles;

		bool SaveAsJpg (const char *fileName);				// "path/*.jpg"
		bool SaveAsPng (const char *fileName);				// "path/*.png"
		bool SaveAsBmp (const char *fileName);				// "path/*.bmp"
		// Remarks:
		// 1. Save Canvas' Content into a .jpg/.png/.bmp File;
		// 2. Performance: bmp = jpg >> png;
		// 3. Size: bmp >> jpg > png;
		// 4. Windows MinGW Version doesn't Support Jpg...

	private:
		int x, y; size_t w, h;								// Postion and Size
		std::unique_ptr<EggAche_Impl::GUIContext> context;	// GUI Impl Bridge

		std::list<Canvas *> subCanvases;				// Sub Canvases
		std::list<Canvas *> parCanvases;				// Parent Canvases

		bool isLatest;
		std::unique_ptr<EggAche_Impl::GUIContext> buffer;	// Buffer with White Bg

		void RecursiveDraw (EggAche_Impl::GUIContext *,		// Helper Function of
							size_t, size_t) const;			// Buffering
		void RecursiveInvalidate ();
		friend bool Window::Refresh ();

		Canvas (const Canvas &) = delete;					// Not allow to copy
		void operator= (const Canvas &) = delete;			// Not allow to copy
	};

	//======================Message Box========================

	void MsgBox (
		const char *szTxt,						// Text String
		const char *szCap = "Hello EggAche");	// Caption String
	// Remarks:
	// Pop up a Message Box;
}

#endif  //EGGACHE_GL
