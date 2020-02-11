#ifndef MAINWND_H
#define MAINWND_H 1
/*--------------------------------------------------------------*/
/* Идентификатор комбинации клавиш для активации главного окна. */
/*--------------------------------------------------------------*/
#define HKACTIVATE 0
/*-------------------------------------------------------------*/
/* Объявления функций, относящихся к главному окну приложения. */
/*-------------------------------------------------------------*/
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM MainWndRegisterClass (HINSTANCE hInstance);
HWND MainWndCreate (HINSTANCE hInstance, ATOM wcx, LPCTSTR lpTitle);
#endif