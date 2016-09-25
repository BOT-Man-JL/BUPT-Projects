

#include <windows.h>
#define UNICODE
#define _UNICODE
 
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("Monkey") ;
     HWND         hwnd ;
     MSG          msg ;
     WNDCLASS     wndclass ;
     
     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
     wndclass.lpszMenuName  = NULL ;
     wndclass.lpszClassName = szAppName ;
     
     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("Program requires Windows NT!"), 
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
     
     hwnd = CreateWindow (szAppName, TEXT ("Monkey"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, NULL, hInstance, NULL) ;
     
     ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ;
     
     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
     }
     return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     static int  cxChar, cxCaps, cyChar, cxClient, cyClient, x ;
     HDC         hdc ;
     PAINTSTRUCT ps ;
	 static RECT rect;
     TEXTMETRIC  tm ;
	 static POINT apt [13], bpt[13] ;
	 TCHAR       szBuffer[100] ;
	 
	 switch (message)
     {
	 case WM_CREATE:
          hdc = GetDC (hwnd) ;
          
          GetTextMetrics (hdc, &tm) ;
          cxChar = tm.tmAveCharWidth ;
          cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2 ;
          cyChar = tm.tmHeight + tm.tmExternalLeading ;
          
          ReleaseDC (hwnd, hdc) ;
          return 0 ;

     case WM_SIZE:
          cxClient = LOWORD (lParam) ;
          cyClient = HIWORD (lParam) ;

          apt[0].x =     cxClient / 4 ;
          apt[0].y = 2 * cyClient / 3 ;
          
          apt[1].x = 3 * cxClient / 64 ;
          apt[1].y = cyClient ;
          
          apt[2].x = 61 * cxClient / 64 ;
          apt[2].y = cyClient ;
          
          apt[3].x = 3 * cxClient / 4 ;
          apt[3].y = 2 * cyClient / 3 ;

		  apt[4].x = cxClient ;
		  apt[4].y = cyClient ;

		  apt[5].x = cxClient ;
		  apt[5].y = 0 ;

		  apt[6].x = 3 * cxClient / 4 ;
		  apt[6].y =     cyClient / 3 ;

		  apt[7].x = 7 * cxClient / 8 ;
		  apt[7].y = - cyClient / 12 ;

		  apt[8].x = cxClient / 8 ;
		  apt[8].y = - cyClient / 12 ;

		  apt[9].x = cxClient / 4 ;
          apt[9].y = cyClient / 3 ;

		  apt[10].x = 0 ;
          apt[10].y = 0 ;

		  apt[11].x = 0 ;
          apt[11].y = cyClient ;

          apt[12].x =     cxClient / 4 ;
          apt[12].y = 2 * cyClient / 3 ;

		  bpt[0].x = cxClient / 2 ;
		  bpt[0].y = cyClient / 4 ;

		  bpt[1].x = cxClient / 3 ;
		  bpt[1].y = cyClient / 18 ;

		  bpt[2].x = cxClient / 5 ;
		  bpt[2].y = cyClient / 3 ;

		  bpt[3].x = cxClient / 2 - 19 * cxClient / 100 ;
		  bpt[3].y = cyClient / 2;

		  bpt[4].x = cxClient / 4 ;
		  bpt[4].y = 23 * cyClient / 32;

		  bpt[5].x = cxClient / 6 ;
		  bpt[5].y = 57 * cyClient / 64;

		  bpt[6].x = cxClient / 2 ;
		  bpt[6].y = 57 * cyClient / 64;

		  bpt[7].x = 5 * cxClient / 6 ;
		  bpt[7].y = 57 * cyClient / 64;

		  bpt[8].x = 3 * cxClient / 4 ;
		  bpt[8].y = 23 * cyClient / 32;

		  bpt[9].x = cxClient / 2 + 19 * cxClient / 100 ;
		  bpt[9].y = cyClient / 2;

		  bpt[10].x = 4 * cxClient / 5 ;
		  bpt[10].y = cyClient / 3 ;

		  bpt[11].x = 2 * cxClient / 3 ;
		  bpt[11].y = cyClient / 18 ;

		  bpt[12].x = cxClient / 2 ;
		  bpt[12].y = cyClient / 4 ;
          return 0 ;
          
     case WM_PAINT:
          hdc = BeginPaint (hwnd, &ps) ;

		  //TextOut (hdc, 10, 12 + cyChar, szBuffer, wsprintf (szBuffer, TEXT ("%4d   %4d"), cxClient, cyClient)) ;
          
          MoveToEx  (hdc, 2 * cxClient / 3, 3 * cyClient / 4, NULL) ;
          LineTo    (hdc, cxClient / 3, 3 * cyClient / 4) ;

          //MoveToEx  (hdc, cxClient / 2, 9 * cyClient / 16, NULL) ;
          //LineTo    (hdc, 31 * cxClient / 64, 78 * cyClient / 128) ;
          //LineTo    (hdc, 16 * cxClient / 32, 79 * cyClient / 128) ;

          //MoveToEx  (hdc, 23 * cxClient / 64, cyClient / 3, NULL) ;
          //LineTo    (hdc, 27 * cxClient / 64, 44 * cyClient / 128) ;

          //MoveToEx  (hdc, 37 * cxClient / 64, 44 * cyClient / 128, NULL) ;
          //LineTo    (hdc, 41 * cxClient / 64, cyClient / 3) ;

		  //SelectObject (hdc, (HBRUSH) GetStockObject (BLACK_BRUSH)) ;

		  Ellipse	(hdc, cxClient / 2 - 159 * cxClient / 1000, cyClient / 2 - 117 * cyClient / 1000,
						  cxClient / 2 - 53 * cxClient / 1000, cyClient / 2 + 29 * cyClient / 1000) ;
		  Ellipse	(hdc, cxClient / 2 + 53 * cxClient / 1000, cyClient / 2 - 117 * cyClient / 1000,
						  cxClient / 2 + 159 * cxClient / 1000, cyClient / 2 + 29 * cyClient / 1000) ;

		  //SelectObject (hdc, (HBRUSH) GetStockObject (WHITE_BRUSH)) ;

		  //Ellipse	(hdc, cxClient / 2 - 105, cyClient / 2 - 40, cxClient / 2 - 95, cyClient / 2 - 20) ;
		  //Ellipse	(hdc, cxClient / 2 + 95, cyClient / 2 - 40, cxClient / 2 + 105, cyClient / 2 - 20) ;

		  PolyBezier (hdc, apt, 13) ;
		  PolyBezier (hdc, bpt, 13) ;
          
          EndPaint (hwnd, &ps) ;

		  //InvalidateRect (hwnd, NULL, TRUE) ;
          return 0 ;

	 case WM_SYSKEYDOWN :

	 case WM_SYSKEYUP :

	 case WM_SYSCHAR :
		 return 0 ;

          
     case WM_DESTROY:
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}
