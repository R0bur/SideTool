/*==============================*/
/* Блок управления приложением. */
/*==============================*/
#include <windows.h>
#include <shellapi.h>
#include "iactrl.h"
#define CMD_QUIT 100
#define CMD_HELP 101
#define CMD_WEB 102
static HBRUSH hbrBackground = 0;
static LRESULT WMCreate (HWND hWnd);
static LRESULT WMCommand (HWND hWnd, WORD wCode, WORD wId, HWND hWndCtl);
static void CmdQuit (HWND hWnd);
static void CmdHelp (HWND hWnd);
static void CmdWeb (HWND hWnd);
/*===========================================*/
/* Получение кисти для заполнения фона окна. */
/* Возврат: манипулятор кисти.               */
/*===========================================*/
HBRUSH IACtrlHbrBackground (void)
{
	if (!hbrBackground)
		hbrBackground = GetStockObject (DKGRAY_BRUSH);
	return hbrBackground;
}
/*===========================================*/
/* Оконная функция блока управления.         */
/* Вызов: hWnd - манипулятор окна календаря, */
/*        uMsg - идентификатор сообщения,    */
/*        wParam - первый параметр,          */
/*        lParam - второй параметр.          */
/*===========================================*/
LRESULT CALLBACK IACtrlWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes;
	lRes = 0;
	switch (uMsg) {
		case WM_CREATE:	/* Создание окна. */
			lRes = WMCreate (hWnd);
			break;
		case WM_COMMAND: /* Обработка сообщений от кнопок. */
			lRes = WMCommand (hWnd, HIWORD (wParam), LOWORD (wParam), (HWND) lParam);
			break;
		default:	/* Не обработанные сообщения. */
			lRes = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	return lRes;
}
/*===================================*/
/* Обработка сообщения WM_CREATE.    */
/* Вызов: hWnd - манипулятор окна.   */
/* Возврат: код обработки сообщения. */
/*===================================*/
static LRESULT WMCreate (HWND hWnd)
{
	RECT rc;	/* координаты рабочей области окна */
	const int wb = 24;	/* ширина кнопки */
	const int hb = 24;	/* высота кнопки */
	int x, y;
	HINSTANCE hInstance = (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE);
	GetClientRect (hWnd, &rc);
	x = rc.right - wb - 1;
	y = rc.top + (rc.bottom - rc.top - hb) / 2;
	CreateWindow (TEXT ("BUTTON"), TEXT ("X"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, wb, hb, hWnd, (HMENU) CMD_QUIT, hInstance, NULL);
	x -= (wb + 1);
	CreateWindow (TEXT ("BUTTON"), TEXT ("?"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, wb, hb, hWnd, (HMENU) CMD_HELP, hInstance, NULL);
	x = 1;
	CreateWindow (TEXT ("BUTTON"), TEXT ("WWW"), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		x, y, 3*wb, hb, hWnd, (HMENU) CMD_WEB, hInstance, NULL);
	return 0;
}
/*=======================================*/
/* Обработка сообщения WM_COMMAND.       */
/* Вызов: hWnd - манипулятор окна,       */
/*        wCode - код сообщения,         */
/*        wId - идентификатор источника, */
/*        hWndCtl - манипулятор элемента */
/*              управления.              */
/* Возврат: код обработки сообщения.     */
/*=======================================*/
static LRESULT WMCommand (HWND hWnd, WORD wCode, WORD wId, HWND hWndCtl)
{
	LRESULT lRes = 0;
	TCHAR msg[256];
	if (!wCode)
		switch (wId) {
			case CMD_QUIT:
				CmdQuit (hWnd);
				break;
			case CMD_HELP:
				CmdHelp (hWnd);
				break;
			case CMD_WEB:
				CmdWeb (hWnd);
				break;
			default:
				wsprintf (msg, TEXT ("Code:\t%u\nId:\t%u\nHWND:\t%u"), wCode, wId, hWndCtl);
				MessageBox (hWnd, msg, TEXT ("iactl.c - WMCommand"), MB_OK | MB_ICONINFORMATION);
		}
	return lRes;
}
/*====================================================*/
/* Обработка запроса на завершение работы приложения. */
/* Вызов: hWnd - манипулятор окна блока управления.   */
/*====================================================*/
static void CmdQuit (HWND hWnd)
{
	HWND hWndParent;
	hWndParent = GetParent (hWnd);
	SendMessage (hWndParent, WM_CLOSE, 0, 0);
}
/*====================================================*/
/* Обработка запроса на выдачу справочной информации. */
/* Вызов: hWnd - манипулятор окна блока управления.   */
/*====================================================*/
static void CmdHelp (HWND hWnd)
{
	LANGID lang;
	static LPCTSTR s[] = {
		TEXT ("Подсказка"),
		TEXT ("Комбинации клавиш:\nCtrl+Q\t- выход из программы,\nCtrl+C\t- копирование значение в буфер обмена,\nCtrl+V\t- вставка значения из буфера обмена,\nCtrl+Alt+C\t- перекл\376чение от другой программы к калькулятору и обратно.\n\nКлавиши в режиме калькулятора:\nEnter\t- выполнение операции [=],\nEsc\t- первый раз [CE], второй - [ON/C],\n\"<-\" (забой)\t- удаление последней набранной цифры,\n\"~\" (тильда)\t- изменение знака набираемого числа.\n\nАвтор программы: Игорь Орещенков, 2019 г."),
		TEXT ("Help"),
		TEXT ("Keyboard shortcuts:\nCtrl+Q\t- close the Application,\nCtrl+C\t- copy the value on the clipboard,\nCtrl+V\t- paste the value from the clipboard,\nCtrl+Alt+C\t- switch from another application to the calculator and back.\n\nKeys in the calculator mode:\nEnter\t- execute equation [=],\nEsc\t- first time [CE], second time - [ON/C],\n\"<-\" (backspace)\t- remove the last entered digit,\n\"~\" (tilde)\t- invert the sign of the value.\n\nThe application developer: Ihar Areshchankau, 2019.")
	};
	LPCTSTR msg, cap;
	int klang;
	lang = GetUserDefaultUILanguage ();
	klang = lang == 0x0419 || lang == 0x0422 || lang == 0x0423? 0: 1;
	cap = s[2 * klang];
	msg = s[2 * klang + 1];
	MessageBox (hWnd, msg, cap, MB_OK | MB_ICONINFORMATION);
}
/*====================================================*/
/* Обработка запроса на открытие Web-сайта программы. */
/* Вызов: hWnd - манипулятор окна блока управления.   */
/*====================================================*/
static void CmdWeb (HWND hWnd)
{
	LANGID lang;
	TCHAR url[256];
	lang = GetUserDefaultUILanguage ();
	wsprintf (url, TEXT ("https://iharsw.login.by/sidetool/?lang=%x"), lang);
	ShellExecute (hWnd, NULL, url, NULL, NULL, SW_SHOWNORMAL);
}