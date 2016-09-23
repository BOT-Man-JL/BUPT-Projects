//
// Interface of EggAche Graphics Library
// By BOT Man, 2016
//

#ifndef EGGACHE_GL
#define EGGACHE_GL

#include <list>
#include <exception>
#include "EggAche_impl.h"

namespace EggAche
{
	class Egg;

	typedef void (*ONCLICK)(int, int);
	typedef void (*ONPRESS)(char);

	//===================EggAche Window========================

	class Window : private _Window
	{
	public:
		Window (
			int width = 1000, int height = 750,				// Size at least 240 * 120
			const char *cap_string = "Hello EggAche",		// Caption String
			ONCLICK fnClick = nullptr,						// Callback void fnClick (int, int)
			ONPRESS fnPress = nullptr);						// Callback void fnPress (char)
		// Remarks:
		// 1. Create a Window of Logic Size width * height with Caption cap_string;
		// 2. If you click or press a key on Window, back-end will call fnClick or fnPress;
		//    Calling fnClick with (int x, int y) means point (x, y) is Clicked;
		//    Calling fnPress with (char ch) means character ch is Inputed;
		// 3. When an error occurs, throw std::runtime_error

		Window (const Window &);
		~Window ();
		
		void AddEgg (Egg& e);								// Add Egg to Window
		std::list<Egg *> &GetLayer ();						// Get Egg layer list

		bool Refresh () override;							// Refresh the Window
		bool IsClosed () const;								// Is Window closed
		void operator = (const Window &) = delete;			// Not allow to copy
	protected:
		std::list<Egg *> ly;								// Egg layer data
	};

	//===========================Egg===========================

	class Egg : protected _DrawContext
	{
	public:
		friend class Window;

		Egg (unsigned int width , unsigned int height ,		// Egg's size
			 int pos_x = 0, int pos_y = 0);					// Egg's initial postion
		// Remarks:
		// When an error occurs, throw std::runtime_error

		int GetX () const;									// Get Egg's coordinate x
		int GetY () const;									// Get Egg's coordinate y
		void MoveTo (int pos_x, int pos_y);					// Place Egg to (pos_x, pos_y)
		void Move (int scale_x, int scale_y);				// Move Egg
		// Remarks:
		// If scale_x > 0, Egg will be moved right scale_x units; else moved left -scale_x;
		// Similarly move scale_y;

		bool SetPen (unsigned int width,					// Pen width
					 unsigned int r = 0,					// Pen color
					 unsigned int g = 0,
					 unsigned int b = 0);
		bool SetBrush (unsigned int r,						// Brush color
					   unsigned int g,
					   unsigned int b);
		// Remarks:
		// If one of r/g/b = -1, the Pen/Brush will be set Transparent;

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
					  int xBeg, int yBeg, int xEnd, int yEnd);
		// Remarks:
		// The points (xLeft, yTop) and (xRight, yBottom) specify the bounding box;
		// An Ellipse formed by the bounding box defines the curve of the Arc;
		// The Arc extends in the current drawing direction from the point
		// where it intersects the radial from the center to (xBeg, yBeg);
		// The Arc ends where it intersects the radial to (xEnd, yEnd);

		bool DrawChord (int xLeft, int yTop, int xRight, int yBottom,
						int xBeg, int yBeg, int xEnd, int yEnd);
		// Remarks:
		// The points (xLeft, yTop) and (xRight, yBottom) specify the bounding box;
		// An Ellipse formed by the bounding box defines the curve of the Chord;
		// The curve begins at the point where the Ellipse intersects the first radial
		// and extends counterclockwise to the point where the second radial intersects;
		// The Chord is closed by drawing a line from the intersection of the first radial
		// and the curve to the intersection of the second radial and the curve;

		bool DrawPie (int xLeft, int yTop, int xRight, int yBottom,
					  int xBeg, int yBeg, int xEnd, int yEnd);
		// Remarks:
		// The points (xLeft, yTop) and (xRight, yBottom) specify the bounding box;
		// An Ellipse formed by the bounding box defines the curve of the Pie;
		// The curve begins at the point where the Ellipse intersects the first radial
		// and extends counterclockwise to the point where the second radial intersects;

		bool DrawTxt (int xBeg, int yBeg, const char *szText);
		// Remarks:
		// Draw the szText with a upper left point (xBeg, yBeg)

		bool DrawBmp (const char *szPath,					// Source: "path/name.bmp"
					  int x = 0, int y = 0,					// Position to paste in Egg
					  int width = -1, int height = -1,		// Size to paste (-1 as default)
					  int r = -1,							// Red color of mask (-1 is not used)
					  int g = -1,							// Green color of mask
					  int b = -1);							// Blue color of mask
		// Remarks:
		// 1. The bmp file will be stretched into width * height in Egg;
		// 2. The color of colorMask will be set to Transparent;
		//    If one of r/g/b is -1, the Egg will be set Opaque;

		void Clear ();										// Clear the Egg
		// Remarks:
		// Erase all the content in Egg

		void operator = (const Egg &) = delete;				// Not allow to copy
	private:
		int x, y;											// Postion data
	};

	//======================Message Box========================

	void MsgBox (
		const char *szTxt,				// Text String
		const char *szCap);				// Caption String
	// Remarks:
	// Pop up a Message Box;
}

#endif  //EGGACHE_GL
