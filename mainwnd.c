#include <windows.h>
#include <windowsx.h>
#include "mainwnd.h"
#include "iacalc.h"
#include "iacalend.h"
#include "iactrl.h"
#include "debug.h"
#define WMU_APPBARNOTIFY WM_USER + 0
#define IDTOOLS 10
/*---------------------------*/
/* Сведения об инструментах. */
/*---------------------------*/
#define NABTOOLS 3
enum EToolPlace {TOOLPLACE_TOP, TOOLPLACE_BOTTOM};
static struct SABTool {
	int w, h;		/* ширина и высота окна инструмента */
	HWND hWnd;		/* DEBUG: манипулятор окна. */
	enum EToolPlace place;	/* размещение инструмента на панели */
	HBRUSH hbrBackground;	/* кисть для фона окна */
	WNDPROC lpfnWndProc;	/* оконная процедура */
} abTools[NABTOOLS];
/*====================================================*/
/* Заполнение структуры со сведениями об инструменте. */
/* Вызов: pABTool - указатель на структуру,           */
/*        w, h - ширина и высота окна инструмента,    */
/*        place - размещение инструмента на панели,   */
/*        hbrBackground - кисть для фона окна,        */
/*        lpfnWndProc - оконная функция.              */
/*====================================================*/
void SetABTool (struct SABTool* pABTool, int w, int h, enum EToolPlace place, HBRUSH hbrBackground, WNDPROC lpfnWndProc)
{
	pABTool->w = w;
	pABTool->h = h;
	pABTool->place = place;
	pABTool->hbrBackground = hbrBackground;
	pABTool->lpfnWndProc = lpfnWndProc;
}
BOOL CALLBACK PlaceToolWindow (HWND hWnd, LPARAM lParam);
/*---------------------------------------------------*/
/* Объявления функций управления панелью приложения. */
/*---------------------------------------------------*/
static BOOL AppBarRegister (HWND hWnd);
static BOOL AppBarUnregister (HWND hWnd);
static void AppBarPosChanged (HWND hWnd, UINT uEdge);
/*-------------------------------------------------------*/
/* Объявления функций обработки сообщений главного окна. */
/*-------------------------------------------------------*/
static LRESULT WMUAppBarNotify (HWND hWnd, UINT uNotifyMsg, LPARAM lParam);
static LRESULT WMCreate (HWND hWnd, LPCREATESTRUCT lpCs);
static LRESULT WMActivate (HWND hWnd, WORD uAct, HWND hWndCo);
static LRESULT WMWindowPosChanged (HWND hWnd, LPWINDOWPOS lpWp);
static LRESULT WMHotKey (HWND hWnd, WORD hkid, WORD modk);
static LRESULT WMDestroy (HWND hWnd);
/*-------------------------------------*/
/* Объявления вспомогательных функций. */
/*-------------------------------------*/
BOOL MainWndChildrenCreate (HINSTANCE hInstance, HWND hWndParent);
/*========================================*/
/* Функция главного окна приложения.      */
/* Вызов: hWnd - манипулятор окна,        */
/*        uMsg - идентификатор сообщения, */
/*        wParam - первый параметр,       */
/*        lParam - второй параметр.       */
/*========================================*/
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes;
	lRes = 0;
	switch (uMsg) {
		case WM_CREATE:		/* Создание главного окна приложения. */
			lRes = WMCreate (hWnd, (LPCREATESTRUCT)lParam);
			break;
		case WM_ACTIVATE:	/* Изменился флаг активности окна. */
			lRes = WMActivate (hWnd, (WORD)LOWORD (wParam), (HWND)lParam);
			break;
		case WM_WINDOWPOSCHANGED:	/* Изменилось местоположение окна. */
			lRes = WMWindowPosChanged (hWnd, (LPWINDOWPOS)lParam);
			break;
		case WM_HOTKEY:		/* Нажато зарегистрированное сочетание клавиш. */
			lRes = WMHotKey (hWnd, (WORD)wParam, (WORD)LOWORD (lParam));
			break;
		case WMU_APPBARNOTIFY:	/* Сообщение для панели приложений. */
			lRes = WMUAppBarNotify (hWnd, wParam, lParam);
			break;
		case WM_DESTROY:	/* Уничтожение главного окна приложения. */
			lRes = WMDestroy (hWnd);
			break;
		default:		/* Перехват не обработанного сообщения. */
			lRes = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	return lRes;
}
/*===================================================*/
/* Регистрация панели приложения.                    */
/* Вызов: hWnd - манипулятор окна панели приложения. */
/* Возврат: TRUE - панель зарегистрирована,          */
/*          FALSE - ошибка регистрации панели.       */
/*===================================================*/
static BOOL AppBarRegister (HWND hWnd)
{
	APPBARDATA abd;
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	abd.uCallbackMessage = WMU_APPBARNOTIFY;
	return SHAppBarMessage (ABM_NEW, &abd);
}
/*======================================================*/
/* Снятие с регистрации панели приложения.              */
/* Вызов: hWnd - манипулятор окна панели приложения.    */
/* Возврат: TRUE - снятие с регистрации прошло успешно, */
/*          FALSE - ошибка снятия с регистрации.        */
/*======================================================*/
static BOOL AppBarUnregister (HWND hWnd)
{
	APPBARDATA abd;
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	return SHAppBarMessage (ABM_REMOVE, &abd);
}
/*==================================================================*/
/* Изменение размеров и местоположения панели приложения.           */
/* Вызов: hWnd - манипулятор окна панели приложения,                */
/*        uEdge - код границы экрана, к которой прикреплена панель. */
/*==================================================================*/
static void AppBarPosChanged (HWND hWnd, UINT uEdge)
{
	APPBARDATA abd;
	RECT rc;
	int nWidth;
	/* Получение информации о размерах панели приложение. */
	GetWindowRect (hWnd, &rc);
	nWidth = rc.right - rc.left;
	/* Подготовка сведений о панели приложения. */
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	abd.uEdge = uEdge;
	/* Определение желаемой области экрана для панели приложения. */
	abd.rc.top = 0;
	abd.rc.bottom = GetSystemMetrics (SM_CYSCREEN);
	switch (uEdge) {
		case ABE_LEFT:
			abd.rc.left = 0;
			abd.rc.right = abd.rc.left + nWidth;
			break;
		case ABE_RIGHT:
			abd.rc.right = GetSystemMetrics (SM_CXSCREEN);
			abd.rc.left = abd.rc.right - nWidth;
	}
	/* Запрос на установку размеров панели приложения. */
	SHAppBarMessage (ABM_QUERYPOS, &abd);
	/* Корректировка предложенных размеров панели приложения. */
	switch (uEdge) {
		case ABE_LEFT:
			abd.rc.right = abd.rc.left + nWidth;
			break;
		case ABE_RIGHT:
			abd.rc.left = abd.rc.right - nWidth;
			break;
	}
	/* Установка новых размеров панели приложения. */
	SHAppBarMessage (ABM_SETPOS, &abd);
	MoveWindow (hWnd, abd.rc.left, abd.rc.top, abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, TRUE);
}
/*=========================================================*/
/* Обработка уведомления, отправленного панели приложений. */
/* Вызов: hWnd - идентификатор окна панели приложения,     */
/*        uNotifyMsg - код уведомления,                    */
/*        lParam - дополнительный параметр.                */
/* Возврат: код обработки уведомления.                     */
/*=========================================================*/
static LRESULT WMUAppBarNotify (HWND hWnd, UINT uNotifyMsg, LPARAM lParam)
{
	LRESULT lRes = 0;
	switch (uNotifyMsg) {
		case ABN_POSCHANGED:	/* возможно понадобится изменить местоположение или размеры панели */
			AppBarPosChanged (hWnd, ABE_RIGHT);
			break;
		case ABN_STATECHANGE:	/* изменился флаг "auto hide" или "always on top" */
			MessageBox (hWnd, TEXT ("ABN_STATECHANGE"), TEXT ("WMUAppBarNotify"), MB_OK);
			break;
		case ABN_FULLSCREENAPP:	/* запущено или закрыто полноэкранное приложение */
			SetWindowPos(hWnd, (BOOL)lParam? HWND_BOTTOM: HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); 
			break;
		case ABN_WINDOWARRANGE:	/* выбрана команда упорядочения окон */
			ShowWindow (hWnd, (BOOL)lParam? SW_HIDE: SW_SHOW);
			break;
		default:
			MessageBox (hWnd, TEXT ("Unknown notification message"), TEXT ("WMUAppBarNotify"), MB_OK);
	}
	return lRes;
}
/*==============================================*/
/* Обработка сообщения WM_CREATE.               */
/* Вызов: hWnd - манипулятор создаваемого окна. */
/* Возврат: код обработки сообщения.            */
/*==============================================*/
static LRESULT WMCreate (HWND hWnd, LPCREATESTRUCT lpCs)
{
	LRESULT lRes;
	lRes = 0;
	/* Создание дочерних окон - окон инструментов. */
	MainWndChildrenCreate (GetModuleHandle (NULL), hWnd);
	/* Регистрация панели приложения и позиционирование её в выделенную область экрана. */
	if (AppBarRegister (hWnd)) {
		AppBarPosChanged (hWnd, ABE_RIGHT);
		/* Регистрация сочетания клавиш для вызова приложения из любого состояния системы. */
		RegisterHotKey (hWnd, HKACTIVATE, MOD_ALT | MOD_CONTROL, 67);
	}
	else {
		MessageBox (hWnd, TEXT ("Cannot register Application bar."), TEXT ("Fatal error"), MB_OK | MB_ICONHAND);
		lRes = -1;
	}
	return lRes;
}
/*============================================*/
/* Обработка сообщения WM_ACTIVATE.           */
/* Вызов: hWnd - манипулятор окна приложения, */
/*        uAct - действие с окном приложения, */
/*        hWndCo - окно-корреспондент (теряю- */
/*               щее или обретающее фокус).   */
/* Возврат: код обработки сообщения.          */
/*============================================*/
static LRESULT WMActivate (HWND hWnd, WORD uAct, HWND hWndCo)
{
	APPBARDATA abd;
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	SHAppBarMessage (ABM_ACTIVATE, &abd); /* должно вызываться в ответ на WM_ACTIVATE по документации */
	/* Передача фокуса дочернему окну калькулятора. */
	/* TODO: реализовать передачу фокуса между окнами инструментов. */
	if (uAct != WA_INACTIVE) {
		/* Окно приложения активируется. */
		SetFocus (abTools[0].hWnd);
		IACalcActive (1);
	}
	else
		IACalcActive (0);
	return 0;
}
/*==============================================*/
/* Обработка сообщения WM_WINDOWPOSCHANGED.     */
/* Вызов: hWnd - манипулятор окна приложения.   */
/*        lpWp - указатель на сведения об окне. */
/* Возврат: код обработки сообщения.            */
/*==============================================*/
static LRESULT WMWindowPosChanged (HWND hWnd, LPWINDOWPOS lpWp)
{
	APPBARDATA abd;
	RECT rc;
	/* Корректировка размеров дочерних окон (окон инструментов). */
	GetClientRect (hWnd, &rc);
	EnumChildWindows (hWnd, PlaceToolWindow, (LPARAM) &rc);
	/* Уведомление операционной системы об изменении местоположения или размеров панели приложения. */
	abd.cbSize = sizeof (APPBARDATA);
	abd.hWnd = hWnd;
	SHAppBarMessage (ABM_WINDOWPOSCHANGED, &abd); /* должно вызываться в ответ на WM_WINDOWPOSCHANGED по документации */
	return 0;
}
/*===============================================*/
/* Обработка сообщения WM_HOTKEY.                */
/* Вызов: hWnd - манипулятор окна приложения,    */
/*        hkid - идентификатор сочетания клавиш, */
/*        modk - флаги клавиш-модификаторов.     */
/* Возврат: код обработки сообщения.             */
/*===============================================*/
static LRESULT WMHotKey (HWND hWnd, WORD hkid, WORD modk)
{
	static HWND hWndBack = NULL;
	HWND hWndCur;
	hWndCur = GetForegroundWindow ();
	if (hWndCur != hWnd) {
		hWndBack = hWndCur;
		SetForegroundWindow (hWnd);
	}
	else if (hWndBack != NULL)
		SetForegroundWindow (hWndBack);
	return 0;
}
/*=========================================================*/
/* Позиционирование окна инструмента на панели приложения. */
/* Вызов: hWnd - манипулятор окна инструмента,             */
/*        lParam - указатель на область RECT, доступную    */
/*                 для размещения окна инструмента.        */
/* Побочный эффект: из области RECT, на которую указывает  */
/*        lParam, исключается занятое окном инструмента    */
/*        пространство.                                    */
/*=========================================================*/
BOOL CALLBACK PlaceToolWindow (HWND hWnd, LPARAM lParam)
{
	LPRECT lprc;
	int i, x, y, w, h;
	/* Определение номера инструмента, для которого требуется подготовить окно. */
	i = GetWindowLong (hWnd, GWL_ID) - IDTOOLS;
	/* Фильтрация окон инструментов по идентификаторам. */
	if (i >= 0 && i < NABTOOLS) {
		lprc = (LPRECT) lParam;
		if (abTools[i].place == TOOLPLACE_TOP) {
			y = lprc->top;
			/* смещение вниз верхней границы свободной области */
			lprc->top += abTools[i].h;
		}
		else {
			y = lprc->bottom - abTools[i].h;
			/* смещение вверх нижней границы свободной области */
			lprc->bottom -= abTools[i].h;
		}
		x = (lprc->right - abTools[i].w) / 2; 
		MoveWindow (hWnd, x, y, abTools[i].w, abTools[i].h, TRUE);
	}
	return TRUE;
}
/*==============================================*/
/* Обработка сообщения WM_DESTROY.              */
/* Вызов: hWnd - манипулятор разрушаемого окна. */
/* Возврат: код обработки сообщения.            */
/*==============================================*/
static LRESULT WMDestroy (HWND hWnd)
{
	AppBarUnregister (hWnd);
	UnregisterHotKey (hWnd, HKACTIVATE);
	PostQuitMessage (0);
	return 0;
}
/*=======================================================*/
/* Регистрация класса главного окна приложения.          */
/* Вызов: hInstance - манипулятор экземпляра приложения. */
/* Возврат: атом класса главного окна приложения.        */
/*=======================================================*/
ATOM MainWndRegisterClass (HINSTANCE hInstance)
{
	WNDCLASSEX wcx;
	ATOM awcx;
	HBITMAP hBackground;
	hBackground = LoadBitmap (hInstance, "BACKGROUND");
	wcx.cbSize = sizeof (wcx);
	wcx.lpfnWndProc = MainWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor (NULL, IDC_ARROW);
	wcx.hbrBackground = CreatePatternBrush (hBackground);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = "MainWindowClass";
	wcx.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	awcx = RegisterClassEx (&wcx);
	DeleteObject (hBackground);
	return awcx;
}
/*=======================================================*/
/* Создание главного окна приложения.                    */
/* Вызов: hInstance - манипулятор экземпляра приложения, */
/*        wcx - атом класса главного окна,               */
/*        lpTitle - указатель на строку заголовка окна.  */
/* Возврат: манипулятор главного окна приложения.        */
/*=======================================================*/
HWND MainWndCreate (HINSTANCE hInstance, ATOM wcx, LPCTSTR lpTitle)
{
	HWND hWnd;
	RECT rc;
	int i;
	/* Формирование информации об инструментах. */
	/* SetABTool (abTools + 0, 240, 300, TOOLPLACE_BOTTOM, IACalcWindowProc); */
	SetABTool (abTools + 0, 240, 300, TOOLPLACE_BOTTOM, IACalcHbrBackground (), IACalcWindowProc);
	SetABTool (abTools + 1, 240, 32, TOOLPLACE_TOP, IACtrlHbrBackground (), IACtrlWindowProc);
	SetABTool (abTools + 2, 240, 240, TOOLPLACE_TOP, IACalendHbrBackground (), IACalendWindowProc);
	/* Вычисление размеров области, способной вместить окно любого инструмента. */
	rc.left = 0;
	rc.top = 0;
	rc.right = 0;
	rc.bottom = 0;
	for (i = 0; i < NABTOOLS; i++) {
		if (abTools[i].w > rc.right)
			rc.right = abTools[i].w;
		if (abTools[i].h > rc.bottom)
			rc.bottom = abTools[i].h;
	}
	/* Создание окна. */
	AdjustWindowRectEx (&rc, WS_POPUP, FALSE, WS_EX_TOOLWINDOW);
	/* Если не указать стиль WS_EX_TOOLWINDOW, то окно после создания будет перемещено в исходное положение. */
	hWnd = CreateWindowEx (WS_EX_TOPMOST | WS_EX_TOOLWINDOW, (LPCTSTR) MAKELONG (wcx, 0), lpTitle,
		WS_POPUP | WS_VISIBLE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL
	);
	return hWnd;
}
/*=====================================================*/
/* Создание дочерних окон - окон инструментов.         */
/* Вызов: hInstance - манипулятор приложения,          */
/*        hParentWnd - манипулятор родительского окна. */
/* Возврат: TRUE - дочерние окна созданы,              */
/*          FALSE - ошибка создания дочерних окон.     */
/*=====================================================*/
BOOL MainWndChildrenCreate (HINSTANCE hInstance, HWND hWndParent)
{
	BOOL bRes = FALSE;
	WNDCLASSEX wcx;
	ATOM awcx;
	HWND hWnd;
	TCHAR lpszClassName[32];
	int i;
	/* Перебор инструментов. */
	wcx.cbSize = sizeof (wcx);
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor (NULL, IDC_ARROW);
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = lpszClassName;
	wcx.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	bRes = TRUE;
	for (int i = 0; bRes && i < NABTOOLS; i++) {
		/* Установка значений, специфичных для инструмента. */
		wsprintf (lpszClassName, "ToolWindowClass%d", IDTOOLS + i);
		wcx.hbrBackground = abTools[i].hbrBackground;
		wcx.lpfnWndProc = abTools[i].lpfnWndProc;
		/* Регистрация класса окна инструмента. */
		awcx = RegisterClassEx (&wcx);
		/* Создание окна инструмента. */
		if (awcx) {
			hWnd = CreateWindowEx (0, (LPCTSTR) MAKELONG (awcx, 0), (LPCTSTR) NULL,
				WS_CHILD | WS_BORDER | WS_VISIBLE, 0, 0, abTools[i].w, abTools[i].h,
				hWndParent, (HMENU)(int)(IDTOOLS + i), hInstance, NULL
			);
			abTools[i].hWnd = hWnd;
			if (!hWnd) {
				bRes = FALSE;
				MessageBox (hWndParent, TEXT ("Child window create error"), TEXT ("Debug"), MB_OK);
			}
		}
		else
			bRes = FALSE;
	}
	return bRes;
}