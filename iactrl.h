/*==============================*/
/* Блок управления приложением. */
/*==============================*/
#ifndef IACTRL_H
#define IACTRL_H 1
HBRUSH IACtrlHbrBackground (void);
LRESULT CALLBACK IACtrlWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif;