 /*----------------------------------------
   Note Pin on your screen.
				(c) 2014 LJN
  ----------------------------------------*/

#include <windows.h>
#include <stack>

#define ID_EDIT		1
#define IDN_TRAY	2

#define IDM_TRAY	3
#define IDM_SHOW	4
#define IDM_QUIT	5
#define IDM_AUTO	6

#define WM_TRAY		(WM_USER + 1)
#define WM_DISPLAY	(WM_USER + 2)  //get from another new instance

BOOL FileWrite (HWND hwndEdit, PTSTR pstrFileName);
BOOL FileRead (HWND hwndEdit, PTSTR pstrFileName);

class TextStack
{
private:
	std::stack <PTSTR> undo, redo;
	HWND hwndEdit;
	BOOL PushText (std::stack <PTSTR> &text);
	BOOL PopText (std::stack <PTSTR> &text);
	VOID ClearText (std::stack <PTSTR> &text);
public:
	TextStack (HWND h) : hwndEdit (h) {}
	BOOL Update ();
	BOOL Undo ();
	BOOL Redo ();
	VOID Clear ();
	~TextStack ();
};
