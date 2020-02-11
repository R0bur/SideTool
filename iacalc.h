/*===================================================*/
/* Заголовочный файл калькулятора панели приложения. */
/*===================================================*/
#ifndef IACALC_H
#define IACALC_H 1
#define IDM_CLIPBOARD_COPY 100
#define IDM_CLIPBOARD_PASTE 101
LRESULT CALLBACK IACalcWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HBRUSH IACalcHbrBackground (void);
void IACalcActive (int act);
#endif