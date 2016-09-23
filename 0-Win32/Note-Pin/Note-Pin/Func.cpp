 /*----------------------------------------
   Note Pin on your screen.
				(c) 2014 LJN
  ----------------------------------------*/

#include <windows.h>
#include <algorithm>
#include "notepin.h"
#include "resource.h"

BOOL FileWrite (HWND hwndEdit, PTSTR pstrFileName) //By Charles Petzold, 1998
{
	DWORD	dwBytesWritten;
	HANDLE	hFile;
	int		iLength;
	PTSTR	pstrBuffer;
	WORD	wByteOrderMark = 0xFEFF;

	if (INVALID_HANDLE_VALUE == 
		(hFile = CreateFile (pstrFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)))
	{
		MessageBox (NULL, TEXT ("NO Access to Create the File!!!"), NULL, MB_ICONWARNING);
		return FALSE;
	}
#ifdef UNICODE
	WriteFile (hFile, &wByteOrderMark, 2, &dwBytesWritten, NULL);
#endif
	iLength = GetWindowTextLength (hwndEdit);
	pstrBuffer = (PTSTR) malloc ((iLength + 1) * sizeof (TCHAR));
	GetWindowText (hwndEdit, pstrBuffer, iLength + 1);

	if (!pstrBuffer)
	{
		MessageBox (NULL, TEXT ("Can NOT Create the Buffer!!!"), NULL, MB_ICONWARNING);
		CloseHandle (hFile);
		return FALSE;
	}

	WriteFile (hFile, pstrBuffer, iLength * sizeof (TCHAR), &dwBytesWritten, NULL);

	if ((iLength * sizeof (TCHAR)) != (int) dwBytesWritten)
	{
		MessageBox (NULL, TEXT ("Too Many Characters!!!"), NULL, MB_ICONWARNING);
		CloseHandle (hFile);
		free (pstrBuffer);
		return FALSE;
	}

	free (pstrBuffer);
	CloseHandle (hFile);
	return TRUE;
}

BOOL FileRead (HWND hwndEdit, PTSTR pstrFileName) //By Charles Petzold, 1998
{
	BYTE	bySwap;
	DWORD	dwBytesRead;
	HANDLE	hFile;
	int		i, iFileLength, iUniTest;
	PBYTE	pBuffer, pText, pConv;

		// Open the file.

	if (INVALID_HANDLE_VALUE == 
			(hFile = CreateFile (pstrFileName, GENERIC_READ, FILE_SHARE_READ,
							NULL, OPEN_EXISTING, 0, NULL)))
		return FALSE;

		// Get file size in bytes and allocate memory for read.
		// Add an extra two bytes for zero termination.
				
	iFileLength = GetFileSize (hFile, NULL); 
	pBuffer = (BYTE *) malloc (iFileLength + 2);

		// Read file and put terminating zeros at end.
	
	ReadFile (hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
	CloseHandle (hFile);
	pBuffer[iFileLength] = '\0';
	pBuffer[iFileLength + 1] = '\0';

		// Test to see if the text is Unicode

	iUniTest = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;
	
	if (IsTextUnicode (pBuffer, iFileLength, &iUniTest))
	{
		pText = pBuffer + 2;
		iFileLength -= 2;

		if (iUniTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE)
		{
			for (i = 0; i < iFileLength / 2; i++)
			{
				bySwap = ((BYTE *) pText) [2 * i];
				((BYTE *) pText) [2 * i] = ((BYTE *) pText) [2 * i + 1];
				((BYTE *) pText) [2 * i + 1] = bySwap;
			}
		}

			// Allocate memory for possibly converted string

		pConv = (BYTE *) malloc (iFileLength + 2);

			// If the edit control is not Unicode, convert Unicode text to 
			// non-Unicode (ie, in general, wide character).

#ifndef UNICODE
		WideCharToMultiByte (CP_ACP, 0, (PWSTR) pText, -1, pConv, 
						iFileLength + 2, NULL, NULL);

			// If the edit control is Unicode, just copy the string
#else
		lstrcpy ((PTSTR) pConv, (PTSTR) pText);
#endif

	}
	else	// the file is not Unicode
	{
		pText = pBuffer;

			// Allocate memory for possibly converted string.

		pConv = (BYTE *) malloc (2 * iFileLength + 2);

			// If the edit control is Unicode, convert ASCII text.

#ifdef UNICODE
		MultiByteToWideChar (CP_ACP, 0, (PSTR) pText, -1, (PTSTR) pConv, 
						iFileLength + 1);

			// If not, just copy buffer
#else
		lstrcpy ((PTSTR) pConv, (PTSTR) pText);
#endif
	}
	
	SetWindowText (hwndEdit, (PTSTR) pConv);
	free (pBuffer);
	free (pConv);
   
	return TRUE;
}

//=============================================================================

BOOL TextStack::PushText (std::stack <PTSTR> &text)
{
	int		iLength;
	PTSTR	pBuffer;

	iLength = GetWindowTextLength (hwndEdit);
	pBuffer = (PTSTR) malloc ((iLength + 1) * sizeof (TCHAR));
	GetWindowText (hwndEdit, pBuffer, iLength + 1);

	text.push (pBuffer);

	if (text.empty ())
		return FALSE;
	else
		return TRUE;
}

BOOL TextStack::PopText (std::stack <PTSTR> &text)
{
	PTSTR pBuffer;

	if (text.size () > 1)
	{
		pBuffer = text.top ();
		free (pBuffer);
		text.pop ();
		pBuffer = text.top ();
		SetWindowText (hwndEdit, pBuffer);
		return TRUE;
	}
	else
		return FALSE;
}

VOID TextStack::ClearText (std::stack <PTSTR> &text)
{
	while (!text.empty ())
	{
		free (text.top ());
		text.pop ();
	}
}

BOOL TextStack::Update ()
{
	ClearText (redo);
	return PushText (undo);
}

BOOL TextStack::Undo ()
{
	if (redo.empty () && undo.size () > 1)
		PushText (redo);
	if (PopText (undo))
	{
		PushText (redo);
		return TRUE;
	}
	else return FALSE;
}

BOOL TextStack::Redo ()
{
	if (PopText (redo))
	{
		PushText (undo);
		return TRUE;
	}
	else return FALSE;
}

VOID TextStack::Clear ()
{
	ClearText (undo);
	ClearText (redo);
	PushText (undo);
}

TextStack::~TextStack ()
{
	ClearText (undo);
	ClearText (redo);
}
