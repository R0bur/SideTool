/*==============================*/
/* Средства отладки приложения. */
/*==============================*/
#include <windows.h>
#include "debug.h"
void DebugMessageBoxXYWH (HWND hWnd, LPCTSTR lpTitle, int x, int y, int w, int h)
{
	TCHAR buf[1024];
	TCHAR* tpl = TEXT ("x = %d\ny = %d\nw = %d\nh = %d");
	wsprintf (buf, tpl, x, y, w, h);
	MessageBox (hWnd, buf, lpTitle, MB_OK | MB_ICONINFORMATION);
}