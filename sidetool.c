#include <windows.h>
#include <commctrl.h>
#include "sidetool.h"
#include "mainwnd.h"
struct Application gApp;
HANDLE SingleAppStart (LPTSTR lpstrApplicationName);
void SingleAppFinish (HANDLE hSingleAppMutex);
WPARAM MessagesLoop (void);
static int CmpDates (int y1, int m1, int d1, int y2, int m2, int d2);
/*=======================================================================*/
/* Основная программа.                                                   */
/* Вызов: hInstance - манипулятор экземпляра приложения,                 */
/*        hPrevInstance - манипулятор предыдущего экземпляра приложения, */
/*        lpCmdLine - указатель на командную строку,                     */
/*        nCmdShow - режим запуска.                                      */
/* Возврат: код завершения.                                              */
/*=======================================================================*/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ATOM wcx;
	INITCOMMONCONTROLSEX iccex;
	gApp.lpName = TEXT ("Side Toolbar");
	HANDLE hSingleAppMutex = SingleAppStart (gApp.lpName);
	if (GetLastError () != ERROR_ALREADY_EXISTS) {
		/* Загрузка классов окон системных элементов управления. */
		iccex.dwSize = sizeof (iccex);
		iccex.dwICC = ICC_DATE_CLASSES;
		if (InitCommonControlsEx (&iccex)) {
			/* Регистрация класса главного окна приложения. */
			wcx = MainWndRegisterClass (hInstance);
			if (wcx) {
				/* Создание главного окна приложения. */
				gApp.hMainWindow = MainWndCreate (hInstance, wcx, gApp.lpName);
				if (gApp.hMainWindow)
					MessagesLoop ();
				else
					MessageBox (NULL, TEXT ("Can't create the Main Window."), gApp.lpName, MB_OK | MB_ICONHAND);
			}
			else 
				MessageBox (NULL, TEXT ("Can't register the Main Window Class."), gApp.lpName, MB_OK | MB_ICONHAND);
		}
		else
			MessageBox (NULL, TEXT ("Can't load Windows Common Controls Classes."), gApp.lpName, MB_OK | MB_ICONHAND); 
		SingleAppFinish (hSingleAppMutex);
	}
	else
		MessageBox (NULL, TEXT ("Application is running already."), gApp.lpName, MB_OK | MB_ICONEXCLAMATION);
	return 0;
}
/*========================================================================*/
/* Установка защиты от запусков дополнительных экземпляров приложения.    */
/* Вызов: lpApplicationName - указатель на строку с названием приложения. */
/* Возврат: манипулятор мьютекса, идентифицирующего приложение.           */
/*========================================================================*/
HANDLE SingleAppStart (LPTSTR lpApplicationName)
{
	TCHAR mName[MAX_PATH] = TEXT ("Local\\");
	StrCat (mName, lpApplicationName);
	StrCat (mName, TEXT (" Single Instance"));
	return CreateMutex (NULL, TRUE, mName);
}
/*==============================================================================*/
/* Снятие защиты от запусков дополнительных экземпляров приложения.             */
/* Вызов: hSingleAppMutex - манипулятор мьютекса, идентифицирующего приложение. */
/*==============================================================================*/
void SingleAppFinish (HANDLE hSingleAppMutex)
{
	CloseHandle (hSingleAppMutex);
	return;
}
/*===================================*/
/* Запуск цикла обработки сообщений. */
/*===================================*/
WPARAM MessagesLoop (void)
{
	BOOL bRet;
	MSG msg;
	do {
		bRet = GetMessage (&msg, NULL, 0, 0);
		if (bRet > 0) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	} while (bRet > 0);
	return msg.wParam;
}
/*=====================================================*/
/* Сравнение дат.                                      */
/* Вызов: y1, m1, d1 - год, месяц и число первой даты, */
/*        y2, m2, d2 - год, месяц и число второй даты. */
/* Возврат: -1 - первая дата меньше второй,            */
/*           0 - даты равны,                           */
/*           1 - первая дата больше второй.            */
/*=====================================================*/
static int CmpDates (int y1, int m1, int d1, int y2, int m2, int d2)
{
	int res;
	res = y1 < y2? -1: y1 > y2? 1: 0;
	if (res == 0)
		res = m1 < m2? -1: m1 > m2? 1: 0;
	if (res == 0)
		res = d1 < d2? -1: d1 > d2? 1: 0;
	return res;
}
