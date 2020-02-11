/*====================================*/
/* Калькулятор для панели приложения. */
/*====================================*/
#include <windows.h>
#include <windowsx.h>
#include "iacalc.h"
#include "iadisp.h"
#include "iaarithm.h"
#include "iastring.h"
#include "debug.h"
/*----------------------------------*/
/* Объявления функций калькулятора. */
/*----------------------------------*/
static LRESULT WMCtlColorStatic (HWND hWnd, HDC hDC, HWND hCtl);
static LRESULT WMCreate (HWND hWnd, LPARAM lParam);
static LRESULT WMChar (HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT WMContextMenu (HWND hWnd, int x, int y);
static LRESULT WMCommand (HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT WMPaint (HWND hWnd);
static void CreateKeyboard (HINSTANCE hInstance, HWND hWnd, int cw, int ch);
static void InitKey (struct SKey* key, int row, int col, enum EKeySize size, LPCTSTR title);
static void InitKeyboard (void);
static void ClipboardCopy ();
static int ClipboardPasteTextToNumber (LPCTSTR txt, LPSTR buf, int sz);
static void ClipboardPaste ();
static void HandleKey (HWND hWnd, enum EKeys key);
static void InitCalculator (LPCTSTR zero, BOOL mc);
static enum EOpRes BiOperation (LPTSTR a, LPCTSTR b, enum EKeys op);
static enum EModes MemoryAction (enum EKeys key);
static void MemoryEnable (BOOL en);
static void CalculatorError (BOOL er);
static void DebugCalculatorState (void);
/*-------------------------------*/
/* Цвета элементов калькулятора. */
/*-------------------------------*/
static HBRUSH hbrBackground = NULL;
static HBRUSH hbrDisplay = NULL;
static DWORD dwDisplayBg = 0;
static DWORD dwDisplayFg = 0;
/*---------------------------------------*/
/* Перечень режимов работы калькулятора. */
/*---------------------------------------*/
static enum EModes {
	NUMB,	/* ввод числа */
	OPER,	/* ввод операции */
	ERR,	/* ошибка */
	CALC,	/* вычисление (по клавише "=") */
	UNOP	/* унарные операции */
};
static enum EModes SwitchCalculatorMode (enum EModes m1, enum EModes m2);
/*-------------------------------*/
/* Перечень клавиш калькулятора. */
/*-------------------------------*/
#define IDKEYS 100
static enum EKeys {
	KEY0, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7, KEY8, KEY9, KEYDP,
	KEYEQ, KEYAD, KEYSB, KEYML, KEYDV, /* KEYSQ, KEYPR, */ KEYSG,
	KEYON, KEYCE, KEYBS, KEYMC, KEYMR, KEYMS, KEYMA, 
	NKEYS,
	KEYST, KEYQT, KEYCC, KEYCV
};
/*------------------------------*/
/* Размеры клавиш калькулятора. */
/*------------------------------*/
static enum EKeySize {SZNORM, SZDBLW, SZDBLH, SZHALFH};
/*----------------------------------*/
/* Сведения о клавише калькулятора. */
/*----------------------------------*/
static struct SKey {
	int row, col;		/* расположение клавиши на клавиатуре калькулятора */
	enum EKeySize size;	/* размер клавиши */
	LPCTSTR title;		/* надпись на клавише */
};
static struct SKey keyboard[NKEYS];
static WNDPROC lpfnOldKeysWndProc;	/* оригинальная оконная функция клавиш */
/*-------------------------------------------------------------*/
/* Состояние калькулятора.                                     */
/* Величина CAPACITY определена в заголовочном файле iadisp.h. */
/*-------------------------------------------------------------*/
static struct {
	enum EModes mode;	/* режим работы */
	TCHAR regx[CAPACITY];	/* регистр X (индикатор) */
	TCHAR regy[CAPACITY];	/* регистр Y (второй операнд) */
	TCHAR regc[CAPACITY];	/* константа */
	TCHAR regm[CAPACITY];	/* память */
	enum EKeys op;		/* ожидающая операция */
	int cm;			/* режим вычислений с константой */
} calculator;
static HWND hWndCalc;		/* манипулятор окна калькулятора */
static HWND hWndDisp;		/* манипулятор окна индикатора */
static RECT rcDisp;		/* область окна индикатора в окне калькулятора */
/*========================================================*/
/* Получение кисти для заполнения фона окна калькулятора. */
/* Возврат: манипулятор кисти.                            */
/*========================================================*/
HBRUSH IACalcHbrBackground (void)
{
	if (!hbrBackground)
		hbrBackground = GetStockObject (WHITE_BRUSH);
	return hbrBackground;
}
/*======================================*/
/* Сообщение об активации калькулятора. */
/* Вызов: act - признак активности.     */
/*======================================*/
void IACalcActive (int act)
{
	dwDisplayFg = act? RGB (0, 0, 0): RGB (127, 127, 127);
	RedrawWindow (hWndDisp, NULL, NULL, RDW_INVALIDATE);
}
/*======================================*/
/* Оконная функция клавиш калькулятора. */
/*======================================*/
static LRESULT CALLBACK KeysWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lres;
	if (uMsg == WM_CHAR)
		lres = WMChar (hWnd, wParam, lParam);
	else
		lres = CallWindowProc (lpfnOldKeysWndProc, hWnd, uMsg, wParam, lParam);
	return lres;
}
/*==============================================*/
/* Оконная функция окна калькулятора.           */
/* Вызов: hWnd - манипулятор окна калькулятора, */
/*        uMsg - идентификатор сообщения,       */
/*        wParam - первый параметр,             */
/*        lParam - второй параметр.             */
/*==============================================*/
LRESULT CALLBACK IACalcWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	switch (uMsg) {
		case WM_CREATE:	/* Создание окна. */
			lRes = WMCreate (hWnd, lParam);
			break;
		case WM_CHAR:	/* Нажата клавиша на клавиатуре. */
			lRes = WMChar (hWnd, wParam, lParam);
			break;
		case WM_CONTEXTMENU: /* Запрошен вызов контекстного меню. */
			lRes = WMContextMenu (hWnd, GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			break;
		case WM_COMMAND: /* Обработка команды. */
			lRes = WMCommand (hWnd, wParam, lParam);
			break;
		case WM_CTLCOLORSTATIC:	/* Запрашивается цвет элемента управления. */
			lRes = WMCtlColorStatic (hWnd, (HDC)wParam, (HWND)lParam);
			break; 
		/*case WM_PAINT:*/ /* Отрисовка клиентской области окна. */
			/* lRes = WMPaint (hWnd);
			break; */
		default:
			lRes = DefWindowProc (hWnd, uMsg, wParam, lParam);
	}
	return lRes;
}
/*===================================================================*/
/* Обработка сообщения WM_CTLCOLORSTATIC.                            */
/* Вызов: hWnd - манипулятор окна калькулятора,                      */
/*        hDC - контекст устройства отображения элемента управления, */
/*        hCtl - манипулятор элемента управления.                    */
/* Возврат: код результата обработки сообщения (см. документацию).   */
/*===================================================================*/
static LRESULT WMCtlColorStatic (HWND hWnd, HDC hDC, HWND hCtl)
{
	/*MessageBox (hWnd, TEXT ("WMCtlColorStatic"), TEXT ("iacalc.c - WMCtlColorStatic"), MB_OK);*/
	LRESULT lRes;
	if (hCtl == hWndDisp) {
		SetTextColor (hDC, dwDisplayFg);
		SetBkColor (hDC, dwDisplayBg);
		lRes = (LRESULT)hbrDisplay;
	}
	else
		lRes = 0;
	return lRes;
}
/*==============================================*/
/* Обработка сообщения WM_PAINT.                */
/* Вызов: hWnd - манипулятор окна.              */
/* Возврат: код результата обработки сообщения. */
/*==============================================*/
static LRESULT WMPaint (HWND hWnd)
{
	RECT rc;
	PAINTSTRUCT ps;
	HDC hdc;
	LRESULT lRes = 0;
	if (GetUpdateRect (hWnd, &rc, TRUE)) {
		hdc = BeginPaint (hWnd, &ps);
		TextOut (hdc, 0, 0, TEXT ("Calculator"), 10);
		EndPaint (hWnd, &ps);
	}
	return lRes;
}
/*============================================================*/
/* Обработка сообщения WM_CHAR.                               */
/* Вызов: hWnd - манипулятор окна,                            */
/*        wParam - код символа, соответствующего клавише,     */
/*        lParam - дополнительные сведения о нажатой клавише. */
/* Возврат: код результата обработки сообщения.               */
/*============================================================*/
static LRESULT WMChar (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	static enum EKeys ekp = NKEYS;
	TCHAR buf[128];
	LRESULT lRes = 0;
	TCHAR ck;
	enum EKeys ek = NKEYS;	/* внутренний код нажатой клавиши */
	ck = (TCHAR) wParam;	/* код символа нажатой клавиши */
	if ((TCHAR) '0' <= ck && ck <= (TCHAR) '9')
		/* Нажата цифровая клавиша. */
		ek = KEY0 + (ck - (TCHAR) '0');
	else
		switch (ck) {
			case VK_ESCAPE:		/* очистка индикатора / инициализация калькулятора */
				ek = (ekp != KEYCE)? KEYCE: KEYON;
				break;
			case VK_BACK:		/* удаление последней набранной цифры */
				ek = KEYBS;
				break;
			case (TCHAR)',':	/* запятая */
			case (TCHAR)'.':	/* точка */
				ek = KEYDP;
				break;
			case (TCHAR)'-':	/* минус */
				ek = KEYSB;
				break;
			case (TCHAR)'+':	/* плюс */
				ek = KEYAD;
				break;
			case (TCHAR)'*':	/* умножить */
				ek = KEYML;
				break;
			case (TCHAR)'/':	/* разделить */
				ek = KEYDV;
				break;
			case (TCHAR)'`':		/* изменить знак числа */
			case (TCHAR)'~':
				ek = KEYSG;
				break;
			case (VK_RETURN):	/* ввод / равно */
			case (TCHAR)'=':
				ek = KEYEQ;
				break;
			case (TCHAR)4:		/* [CTRL]+[D] - отображение внутреннего состояния калькулятора */
				ek = KEYST;
				break;
			case (TCHAR)17:		/* [CTRL]+[Q] - завершение работы с калькулятором */
				ek = KEYQT;
				break;
			case (TCHAR)3:		/* [CTRL]+[C] - копирование содержимого индикатора */
				ek = KEYCC;
				break;
			case (TCHAR)22:		/* [CTRL]+[V] - вставка числа в индикатор */
				ek = KEYCV;
				break;
			default:
				ek = NKEYS;
/*				wsprintf (buf, TEXT ("WM_CHAR wParam = %u\nLOWORD(lParam) = %u\nHIWORD(lParam) = %u"), wParam, LOWORD (lParam), HIWORD (lParam));
				MessageBox (hWnd, buf, TEXT ("iacalc.c - WMChar()"), MB_OK);*/
		}
	/* Установка фокуса на кнопку, соответствующую нажатой клавише. */
	if (ek != NKEYS)
		SetFocus (GetDlgItem (hWndCalc, IDKEYS + ek));
	/* Обработка нажатой клавиши. */
	HandleKey (hWnd, ek);
	/* Запоминание обработанной клавиши. */
	ekp = ek;
	return lRes;
}
/*==============================================*/
/* Обработка сообщения WM_CONTEXTMENU.          */
/* Вызов: hWnd - манипулятор окна,              */
/*        x, y - местоположение указателя мыши. */
/* Возврат: код результата обработки сообщения. */
/*==============================================*/
static LRESULT WMContextMenu (HWND hWnd, int x, int y)
{
	HMENU hMenu;
	BOOL mRes;
	LANGID lang;
	int klang;
	LPCTSTR menuItems[] = {
		TEXT ("&Копировать \tCtrl+C"),
		TEXT ("&Copy \tCtrl+C"),
		TEXT ("Вст&авить \tCtrl+V"),
		TEXT ("P&aste \tCtrl+V")
	};
	RECT rc;
	POINT pt;
	LRESULT lRes = 0;
	/* Определение экранных координат для проверки щелчка по индикатору калькулятора. */
	GetWindowRect (hWndDisp, &rc);
	pt.x = x > 0? x: rc.left;
	pt.y = y > 0? y: rc.top;
	if (PtInRect (&rc, pt)) {
		/* Создание контекстного меню. */
		lang = GetUserDefaultUILanguage ();
		klang = lang == 0x0419 || lang == 0x0422 || lang == 0x0423? 0: 1;
		hMenu = CreatePopupMenu ();
		AppendMenu (hMenu, MF_STRING, IDM_CLIPBOARD_COPY, menuItems[0 + klang]);
		AppendMenu (hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu (hMenu, MF_STRING, IDM_CLIPBOARD_PASTE, menuItems[2 + klang]);
		/* Предложение меню к выбору. */
		mRes = TrackPopupMenuEx (hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, hWnd, NULL);
		/* Завершение работы с меню. */
		DestroyMenu (hMenu);
	}
	return lRes;
}
/*==================================================*/
/* Обработка сообщения WM_COMMAND.                  */
/* Вызов: hWnd - манипулятор окна,                  */
/*        wParam - первый параметр сообщения,       */
/*        lParam - манипулятор элемента управления. */
/* Возврат: код результата обработки сообщения.     */
/*==================================================*/
static LRESULT WMCommand (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	if (lParam == 0)
		/* Сообщение от контекстного меню. */
		switch (LOWORD (wParam)) {
			case IDM_CLIPBOARD_COPY:	/* копирование индикатора в буфер обмена */
				HandleKey (hWnd, KEYCC);
				break;
			case IDM_CLIPBOARD_PASTE:	/* вставка из буфера обмена в индикатор */
				HandleKey (hWnd, KEYCV);
				break;
			default:
				MessageBox (hWnd, TEXT ("Unknown context menu command."), TEXT ("iacalc.c: IACalcWindowProc"), MB_OK | MB_ICONEXCLAMATION);
		}
	else if (HIWORD (wParam) == BN_CLICKED)
		/* Сообщение от кнопки калькулятора. */
		HandleKey (hWnd, LOWORD (wParam) - IDKEYS);
	return lRes;
}
/*==============================================*/
/* Обработка сообщения WM_CREATE.               */
/* Вызов: hWnd - манипулятор окна,              */
/*        lParam - указатель на CREATESTRUCT.   */
/* Возврат: код результата обработки сообщения. */
/*==============================================*/
static LRESULT WMCreate (HWND hWnd, LPARAM lParam)
{
	LRESULT lRes = 0;
	HINSTANCE hInstance = (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE);
	hWndCalc = hWnd;
	InitKeyboard ();
	CreateKeyboard (hInstance, hWnd, 42, 42);
	InitCalculator (DisplayGetZero (), TRUE);
	return lRes;
}
/*==============================================*/
/* Создание клавиатуры калькулятора.            */
/* Вызов: hInstance - манипулятор приложения,   */
/*        hWnd - манипулятор окна калькулятора, */
/*        cw - ширина позиции для клавиши,      */
/*        ch - высота позиции для клавиши.      */
/*==============================================*/
static void CreateKeyboard (HINSTANCE hInstance, HWND hWnd, int cw, int ch)
{
	RECT rc;
	DWORD ws;
	int kw0, kh0, x0, y0, x, y, w, h;
	int rows, cols;
	enum EKeys key;
	HWND hWndTmp;
	/* Определение размеров сетки для клавиатуры. */
	rows = cols = 0;
	for (key = KEY0; key < NKEYS; key++) {
		if (keyboard[key].col > cols)
			cols = keyboard[key].col;
		if (keyboard[key].row > rows)
			rows = keyboard[key].row;
	}
	rows += 1;
	cols += 1;
	/* Определение местоположения клавиатуры на калькуляторе. */
	GetClientRect (hWnd, &rc);
	x0 = rc.left + (rc.right - rc.left - cols * cw) / 2;
	y0 = rc.bottom - 4 - ch;
	/* Стиль окна клавиш. */
	ws = WS_CHILD | BS_PUSHBUTTON | BS_FLAT | WS_VISIBLE;
	/* Определение значений коэффициентов для вычисления размеров клавиши. */
	kw0 = cw / 2 - 1;
	kh0 = ch / 2 - 1;
	/* Перебор всех клавиш клавиатуры калькулятора. */
	for (key = KEY0; key < NKEYS; key++) {
		/* Вычисление позиции клавиши на окне калькулятора. */
		x = x0 + keyboard[key].col * cw;
		y = y0 - keyboard[key].row * ch;
		/* Вычисление размера клавиши. */
		switch (keyboard[key].size) {
			case SZDBLW:	/* клавиша двойной ширины */
				w = cw + 2 * kw0;
				h = 2 * kh0;
				break;
			case SZDBLH:	/* клавиша двойной высоты */
				y -= ch;
				w = 2 * kw0;
				h = ch + 2 * kh0;
				break;
			case SZHALFH:	/* клавиша половинной высоты */
				y += ch / 3;
				w = 2 * kw0;
				h = kh0;
				break;
			default:	/* клавиша нормального размера */
				w = 2 * kw0;
				h = 2 * kh0;
		}
		/* Создание окна клавиши. */
		hWndTmp = CreateWindow ("BUTTON", keyboard[key].title, ws, x, y, w, h, hWnd, (HMENU)(int)(IDKEYS + key), hInstance, NULL);
		lpfnOldKeysWndProc = (WNDPROC)SetWindowLong (hWndTmp, GWL_WNDPROC, (DWORD)KeysWindowProc);
	}
	/* Размещение табло калькулятора. */
	/* TODO: уместно ли создавать табло в функции создания клавиатуры? */
	rcDisp.left = x0;
	rcDisp.top = 6;
	rcDisp.right = rcDisp.left + cols * cw;
	rcDisp.bottom = rcDisp.top + ch;
	hbrDisplay = GetStockObject (WHITE_BRUSH);
	dwDisplayBg = RGB (255, 255, 255);
	dwDisplayFg = RGB (0, 0, 0);
	hWndDisp = DisplayCreate (hWnd, &rcDisp);
}
/*===============================================*/
/* Формирование клавиши клавиатуры калькулятора. */
/* Вызов: key - указатель на сведения о клавише, */
/*        row - строка клавиши (снизу вверх),    */
/*        col - столбец клавиши (слева направо), */
/*        size - размер клавиши,                 */
/*        title - надпись на клавише.            */
/*===============================================*/
static void InitKey (struct SKey* key, int row, int col, enum EKeySize size, LPCTSTR title)
{
	key->row = row;
	key->col = col;
	key->size = size;
	key->title = title;
}
/*=======================================*/
/* Формирование клавиатуры калькулятора. */
/*=======================================*/
static void InitKeyboard (void)
{
	InitKey (keyboard + KEY0, 0, 0, SZDBLW, TEXT ("0"));
	InitKey (keyboard + KEYDP, 0, 2, SZNORM, TEXT ("."));
	InitKey (keyboard + KEYAD, 0, 3, SZNORM, TEXT ("+"));
	InitKey (keyboard + KEYEQ, 0, 4, SZDBLH, TEXT ("="));
	InitKey (keyboard + KEYON, 1, 0, SZNORM, TEXT ("ON/C"));
	InitKey (keyboard + KEY1, 1, 1, SZNORM, TEXT ("1"));
	InitKey (keyboard + KEY2, 1, 2, SZNORM, TEXT ("2"));
	InitKey (keyboard + KEY3, 1, 3, SZNORM, TEXT ("3"));
	InitKey (keyboard + KEYCE, 2, 0, SZNORM, TEXT ("CE"));
	InitKey (keyboard + KEY4, 2, 1, SZNORM, TEXT ("4"));
	InitKey (keyboard + KEY5, 2, 2, SZNORM, TEXT ("5"));
	InitKey (keyboard + KEY6, 2, 3, SZNORM, TEXT ("6"));
	InitKey (keyboard + KEYSB, 2, 4, SZNORM, TEXT ("-"));
	InitKey (keyboard + KEYBS, 3, 0, SZNORM, TEXT ("DEL"));
	InitKey (keyboard + KEY7, 3, 1, SZNORM, TEXT ("7"));
	InitKey (keyboard + KEY8, 3, 2, SZNORM, TEXT ("8"));
	InitKey (keyboard + KEY9, 3, 3, SZNORM, TEXT ("9"));
	InitKey (keyboard + KEYML, 3, 4, SZNORM, TEXT ("*"));
	InitKey (keyboard + KEYMC, 4, 0, SZNORM, TEXT ("MC"));
	InitKey (keyboard + KEYMR, 4, 1, SZNORM, TEXT ("MR"));
	InitKey (keyboard + KEYMS, 4, 2, SZNORM, TEXT ("M-"));
	InitKey (keyboard + KEYMA, 4, 3, SZNORM, TEXT ("M+"));
	InitKey (keyboard + KEYDV, 4, 4, SZNORM, TEXT ("/"));
/*	InitKey (keyboard + KEYSQ, 5, 2, SZHALFH, TEXT ("SQ"));
	InitKey (keyboard + KEYPR, 5, 3, SZHALFH, TEXT ("%"));*/
	InitKey (keyboard + KEYSG, 5, 4, SZHALFH, TEXT ("+/-"));
}
/*=======================================================================*/
/* Подготовка к работе вычислительного узла.                             */
/* Вызов: zero - нулевое значение, отображаемое на дисплее калькулятора, */
/*        mc - требование очистки регистра памяти.                       */
/*=======================================================================*/
static void InitCalculator (LPCTSTR zero, BOOL mc)
{
	StrCpy (calculator.regx, zero);
	StrCpy (calculator.regy, zero);
	StrCpy (calculator.regc, zero);
	if (mc) {
		StrCpy (calculator.regm, zero);
		MemoryEnable (FALSE);
	}
	calculator.op = KEYEQ;		/* ожидает фиктивная операция */
	calculator.mode = NUMB;		/* режим ввода числа */
	calculator.cm = 0;		/* константа не готова */
}
/*===================================================*/
/* Выполнение бинарной операции b <= a op b.         */
/* Вызов: b - второй операнд и место для результата, */
/*        a - первый операнд,                        */
/*        op - код операции.                         */
/* Возврат: результат выполнения операции.           */
/*===================================================*/
static enum EOpRes BiOperation (LPTSTR b, LPCTSTR a, enum EKeys op)
{
	enum EOpRes opr;
	switch (op) {
		case KEYAD:	/* сложение */
			opr = OpAdd (b, a);
			break;
		case KEYSB:	/* вычитание */
			opr = OpSub (b, a);
			break;
		case KEYML:	/* умножение */
			opr = OpMul (b, a);
			break;
		case KEYDV:	/* деление */
			opr = OpDiv (b, a);
			break;
		case KEYEQ:	/* фиктивная операция, не требующая вычислений */
			opr = OPOK;
			break;
		default:	/* неизвестная операция */
			MessageBox (NULL, TEXT ("BiOperation (...)\nop - unknown operation"), TEXT ("DEBUG: iacalc.c"), MB_ICONEXCLAMATION | MB_OK);
			opr = OPERR;
	}
	return opr;
}
/*==========================================*/
/* Переключение режима работы калькулятора. */
/* Вызов: m1 - текущий режим работы,        */
/*        m2 - запрашиваемый режим работы.  */
/* Возврат: новый режим работы.             */
/*==========================================*/
static enum EModes SwitchCalculatorMode (enum EModes m1, enum EModes m2)
{
	TCHAR msg[128];
	enum EModes res = m1;	/* по умолчанию переключение запрещено */
	enum EOpRes opr;	/* результат выполнения операции */
	opr = OPOK;	/* подразумевается корректное выполнение операции */
	if (m1 == NUMB && m2 == OPER) {
		/* Из режима ввода числа в режим выбора операции. */
		StrCpy (calculator.regx, DisplayGetValue ());
		if (!calculator.cm) {
			/* Вычисление Y [op] X с записью результата в X. */
			opr = BiOperation (calculator.regx, calculator.regy, calculator.op);
			/* Отображение результата на индикаторе. */
			DisplaySetValue (calculator.regx);
		}
		else
			/* Прекращение вычислений с константой. */
			calculator.cm = 0;
		res = m2;
	}
	else if (m1 == NUMB && m2 == UNOP) {
		/* Из режима ввода числа в режим выполнения унарной операции. */
		res = m2;
	}
	else if (m1 == UNOP && m2 == OPER) {
		/* Из режима унарных операций в режим выбора операции. */
		StrCpy (calculator.regx, DisplayGetValue ());
		if (!calculator.cm) {
			/* Вычисление Y [op] X с записью результата в X. */
			opr = BiOperation (calculator.regx, calculator.regy, calculator.op);
			/* Отображение результата на индикаторе. */
			DisplaySetValue (calculator.regx);
		}
		else
			/* Прекращение вычислений с константой. */
			calculator.cm = 0;
		res = m2;
	}
	else if (m1 == OPER && m2 == NUMB) {
		/* Из режима выбора операции в режим ввода числа. */
		/* DebugCalculatorState (); */
		/* Пересылка X в Y. */
		StrCpy (calculator.regy, calculator.regx);
		/* Очистка индикатора. */
		DisplayClearAll ();
		res = m2;
	}
	else if (m1 == OPER && m2 == UNOP) {
		/* Из режима выбора операции в режим унарной операции. */
		/* Пересылка X в Y. */
		StrCpy (calculator.regy, calculator.regx);
		res = m2;
	}
	else if (m1 == NUMB && m2 == CALC) {
		/* Из режима ввода числа в режим вычислений с константой. */
		if (!calculator.cm) {
			/* Установка константы в регистр X. */
			StrCpy (calculator.regx, DisplayGetValue ());
			/* Выполнение ожидающей бинарной операции. */
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* Отображение результата операции на индикаторе. */
			DisplaySetValue (calculator.regc);
			calculator.cm = 1;
		}
		else {
			/* Первое вычисление с константой, находящейся в регистре X. */
			StrCpy (calculator.regy, DisplayGetValue ());
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* Отображение результата операции на индикаторе. */
			DisplaySetValue (calculator.regc);
		}
		res = m2;
	}
	else if (m1 == CALC && m2 == CALC) {
		/* Повторное вычисление с константой, находящейся в регистре X. */
		StrCpy (calculator.regy, DisplayGetValue ());
		StrCpy (calculator.regc, calculator.regx);
		opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
		/* Отображение результата операции на индикаторе. */
		DisplaySetValue (calculator.regc);
		res = m2;
	}
	else if (m1 == CALC && m2 == NUMB) {
		/* Из режима вычислений с константой в режим ввода числа. */
		/* Просто очистка индикатора. */
		DisplayClearAll ();
		res = m2;		
	}
	else if (m1 == CALC && m2 == OPER) {
		/* Из режима вычислений с константой в режим выбора операции. */
		StrCpy (calculator.regx, DisplayGetValue ());
		/* Прекращение вычислений с константой. */
		calculator.cm = 0;
		res = m2;
	}
	else if (m1 == CALC && m2 == UNOP) {
		/* Из режима вычислений с константой в режим унарных операций. */
		res = m2;
	}
	else if (m1 == UNOP && m2 == CALC) {
		/* Из режима унарных операций в режим вычислений с константой. */
		if (calculator.cm) {
			/* Имеется готовая к вычислениям константа. */
			StrCpy (calculator.regy, DisplayGetValue ());
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* Отображение результата операции на индикаторе. */
			DisplaySetValue (calculator.regc);
			res = m2;
		}
		else {
			/* Установка константы в регистр X. */
			StrCpy (calculator.regx, DisplayGetValue ());
			/* Выполнение ожидающей бинарной операции. */
			StrCpy (calculator.regc, calculator.regx);
			opr = BiOperation (calculator.regc, calculator.regy, calculator.op);
			/* Отображение результата операции на индикаторе. */
			DisplaySetValue (calculator.regc);
			calculator.cm = 1;
		}
	}
	else if (m1 == UNOP && m2 == NUMB) {
		/* Из режима унарных операций в режим ввода числа. */
		/* Очистка индикатора. */
		DisplayClearAll ();
		res = m2;
	}
	else if (m1 == ERR) {
		/* Сброс состояния ошибки осуществляется клавишами [CE] и [ON/C]. */
		res = ERR;	/* подтверждается состояние ошибки */
	}
	else if (m1 != m2) {
		/* Переключение запрещено. */
		wsprintf (msg, TEXT ("Forbidden mode switch from %d to %d"), m1, m2);
		MessageBox (NULL, msg, TEXT ("iacalc.c - SwitchCalculatorMode ()"), MB_OK | MB_ICONEXCLAMATION);
	}
	/* Проверка отсутствия ошибок при выполнении вычислений. */
	if (opr != OPOK) {
		CalculatorError (TRUE);
		res = ERR;
	}
	return res;
}
/*================================================================*/
/* Выполнение действий с ячейкой памяти.                          */
/* Вызов: key - код клавиши, требующей действия с ячейкой памяти. */
/* Возврат: новый режим работы калькулятора (UNOP или ERR).       */
/*================================================================*/
static enum EModes MemoryAction (enum EKeys key)
{
	enum EModes res;
	enum EOpRes opr;
	opr = OPOK;	/* подразумевается корректное выполнение операции */
	switch (key) {
		case KEYMA:	/* Прибавление значения, отображаемого на индикаторе, к ячейке памяти. */
		case KEYMS:	/* Вычитание значения, отображаемого на индикаторе, из ячейки памяти. */
			/* Перенос значения, отображаемого на индикаторе, во вспомогательный регистр. */
			StrCpy (calculator.regc, DisplayGetValue ());
			/* Выполнение операции сложения или вычитания. */
			opr = BiOperation (calculator.regc, calculator.regm, key == KEYMA? KEYAD: key == KEYMS? KEYSB: NKEYS);
			/* Запись результата в ячейку памяти. */
			StrCpy (calculator.regm, calculator.regc);
			/* Включение клавиш управления ячейкой памяти. */
			MemoryEnable (TRUE);
			break;
		case KEYMR:	/* Извлечение значения из ячейки памяти на индикатор. */
			DisplaySetValue (calculator.regm);
			break;
		case KEYMC:	/* Очистка ячейки памяти. */
			StrCpy (calculator.regm, DisplayGetZero ());
			MemoryEnable (FALSE);
			break;
		default:
			MessageBox (NULL, "Unknown action in MemoryAction ()", "Debug: iacalc.c", MB_OK | MB_ICONEXCLAMATION);
	}
	/* Проверка отсутствия ошибок при выполнении вычислений. */
	if (opr != OPOK) {
		CalculatorError (TRUE);
		res = ERR;
	}
	else
		res = UNOP;
	return res;
}
/*==================================================================*/
/* Изменение состояния ячейки памяти калькулятора.                  */
/* Вызов: en - признак наличия полезной информации в ячейке памяти. */
/*==================================================================*/
static void MemoryEnable (BOOL en)
{
	EnableWindow (GetDlgItem (hWndCalc, IDKEYS + KEYMR), en);
	EnableWindow (GetDlgItem (hWndCalc, IDKEYS + KEYMC), en);
	if (!en)
		SetFocus (hWndCalc);
}
/*==========================================*/
/* Изменение состояния ошибки калькулятора. */
/* Вызов: er - признак наличия ошибки.      */
/*==========================================*/
static void CalculatorError (BOOL er)
{
	register int key;
	er = !er;
	for (key = KEY0; key < NKEYS; key++)
		if (key != KEYON && key != KEYCE && key != KEYMC && key != KEYMR)
			EnableWindow (GetDlgItem (hWndCalc, IDKEYS + key), er);
	if (!er)
		SetFocus (hWndCalc);
}
/*===============================================*/
/* Выполнение вычислений по нажатии клавиши "=". */
/*===============================================*/
static void HandleEqKey ()
{
	if (!calculator.cm) {
		/* Перенос значения из индикатора в регистр X калькулятора. */
		StrCpy (calculator.regx, DisplayGetValue ());
		/* Подготовка значения константы. */
		StrCpy (calculator.regc, calculator.regx);
		calculator.cm = 1;
		/* Выполнение операции с регистром Y. */
		BiOperation (calculator.regx, calculator.regy, calculator.op);
	}
	else {
		/* Выполнение операции с константой. */
		StrCpy (calculator.regy, DisplayGetValue ());
		StrCpy (calculator.regx, calculator.regc);
		BiOperation (calculator.regx, calculator.regy, calculator.op);
	}
	/* Отображение результата на индикаторе. */
	DisplaySetValue (calculator.regx);
}
/*====================================================*/
/* Копирование содержимого индикатора в буфер обмена. */
/*====================================================*/
static void ClipboardCopy ()
{
	TCHAR const* v;		/* указатель на содержимое индикатора */
	HGLOBAL hgValue;	/* манипулятор области глобальной памяти */
	LPTSTR lpValue;		/* указатель на глобальную область памяти */
	int n;
	/* Получение доступа к буферу обмена. */
	if (OpenClipboard (NULL)) {
		/* Очистка содержимого буфера обмена. */
		EmptyClipboard ();
		/* Получение сведений о содержимом индикатора */
		v = DisplayGetValue ();
		n = StrLen (v);
		hgValue = GlobalAlloc (GMEM_MOVEABLE, (n + 1) * sizeof (TCHAR));
		if (hgValue) {
			lpValue = GlobalLock (hgValue);
			memcpy (lpValue, v, n * sizeof (TCHAR));
			lpValue[n] = (TCHAR)'\0';
			GlobalUnlock (lpValue);
		}
		SetClipboardData (CF_TEXT, hgValue);
		/* Завершение работы с буфером обмена. */
		CloseClipboard ();
	}
}
/*==============================================================*/
/* Извлечение числа в формате калькулятора из текстовой строки. */
/* Вызов: txt - текстовая строка,                               */
/*        buf - буфер для записи числа,                         */
/*        sz  - размер буфера.                                  */
/* Возврат: количество символов числа в формате калькулятора,   */
/*          записанных в буфер buf, не считая символа '\0'.     */
/*==============================================================*/
static int ClipboardPasteTextToNumber (LPCTSTR txt, LPSTR buf, int sz)
{
	int i, j, dp;
	int nd, mnd;	/* максимальное количество цифр в числе */
	TCHAR s[CAPACITY];
	TCHAR c;
	int res;
	if (sz > 2) {
		mnd = sz - 3;	/* исключается знак, десятичный разделитель и ограничитель */
		dp = 0;	/* признак наличия дробной части */
		nd = 0;	/* текущее количество цифр в числе */
		i = 0;	/* индекс источника */
		j = 0;	/* индекс приёмника */
		/* Пропуск символов, не являющихся цифрами и знаками чисел. */
		c = txt[i];
		while (c && !((TCHAR)'0' <= c && c <= (TCHAR)'9') && c != (TCHAR)'-')
			c = txt[++i];
		/* Определение знака числа. */
		if (c == (TCHAR)'-') {
			s[j++] = (TCHAR)'-';
			i++;
		}
		/* Считывание целой части числа. */
		c = txt[i++];
		/* Пропуск незначащих нулей целой части. */
		while (c == (TCHAR)'0') 
			c = txt[i++];
		/* Считываниие остальных цифр целой части. */
		while ((TCHAR)'0' <= c && c <= (TCHAR)'9' && j < sz - 2 && nd < mnd) {
			s[j++] = c;
			c = txt[i++];
			nd++;
		}
		/* Считывание десятичного разделителя. */
		if ((c == (TCHAR)'.' || c == (TCHAR)',') && j < sz - 2) {
			/* При отсутствующей целой части вставка нуля. */
			if (!j || j == 1 && s[0] == '-') {
				s[j++] = (TCHAR)'0';
				nd++;
			}
			/* Вставка десятичного разделителя. */
			s[j++] = (TCHAR)',';
			dp = 1;
		}
		else
			i--;	/* возврат указателя к недопустимому символу. */
		/* Считывание цифр дробной части. */
		c = txt[i++];
		while ((TCHAR)'0' <= c && c <= (TCHAR)'9' && j < sz - 1 && nd < mnd) {
			s[j++] = c;
			c = txt[i++];
			nd++;
		}
		/* Отбрасывание незначащих нулей дробной части. */
		if (dp) {
			j--;
			while (s[j] == (TCHAR)'0')
				j--;
		}
		else
			s[j] = (TCHAR)',';
		s[++j] = (TCHAR)'\0';
		/* Проверка наличия числового значения в обработанной текстовой строке. */
		if (s[0] != (TCHAR)',' || s[1] != (TCHAR)'\0') {
			StrCpy (buf, s);
			res = j;
		}
		else
			res = 0;
	}
	else
		res = 0;
	return res;
}
/*================================================*/
/* Вставка содержимого буфера обмена в индикатор. */
/*================================================*/
static void ClipboardPaste ()
{
	TCHAR buf[CAPACITY];
	HGLOBAL hgValue;	/* манипулятор области глобальной памяти */
	LPTSTR lpValue;		/* указатель на глобальную область памяти */
	/* Получение доступа к буферу обмена. */
	if (IsClipboardFormatAvailable (CF_TEXT) && OpenClipboard (NULL)) {
		hgValue = GetClipboardData (CF_TEXT);
		if (hgValue) {
			lpValue = GlobalLock (hgValue);
			if (lpValue) {
				if (ClipboardPasteTextToNumber (lpValue, buf, CAPACITY))
					DisplaySetValue (buf);
				GlobalUnlock (hgValue);
			}
		}
		/* Завершение работы с буфером обмена. */
		CloseClipboard ();
		SetFocus (hWndCalc);
	}
}
/*==============================================*/
/* Обработка нажатия клавиши калькулятора.      */
/* Вызов: hWnd - манипулятор окна калькулятора, */
/*        key - внутренний код нажатой клавиши. */
/*==============================================*/
static void HandleKey (HWND hWnd, enum EKeys key)
{
	TCHAR msg[64];
	if (KEY0 <= key && key <= KEY9) {
		/* Цифровая клавиша. */
		/* Переход в режим ввода числа. */
		calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
		if (calculator.mode == NUMB)
			DisplayConcatDigit ((TCHAR) '0' + key);
	}
	else
		switch (key) {
			case KEYON:	/* включение калькулятора */
				InitCalculator (DisplayGetZero (), FALSE);
				DisplayClearAll ();
				CalculatorError (FALSE);
				break;
			case KEYCE:	/* сброс ошибки или очистка индикатора */
				if (calculator.mode == ERR) {
					/* Сброс ошибки. */
					calculator.mode = UNOP;
					CalculatorError (FALSE);
				}
				else {
					/* Очистка индикатора. */
					calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
					if (calculator.mode == NUMB)
						DisplayClearAll ();
				}
				break;
			case KEYBS:	/* удаление последней цифры */
				if (calculator.mode == NUMB)
					DisplayClearLastDigit ();
				break;
			case KEYDP:	/* десятичная точка */
				/* Переход в режим ввода числа. */
				calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
				if (calculator.mode == NUMB)
					DisplayDecimalPoint ();
				break;
			case KEYSG:	/* изменение знака числа */
				if (calculator.mode == NUMB)
					DisplaySign ();
				break;
			case KEYSB:	/* вычитание */
			case KEYAD:	/* сложение */
			case KEYML:	/* умножение */
			case KEYDV:	/* деление */
				/* Переход в режим выбора операции. */
				calculator.mode = SwitchCalculatorMode (calculator.mode, OPER);
				if (calculator.mode == OPER)
					calculator.op = key;	/* фиксация операции */
				break;
			case KEYEQ:	/* равно */
				/* Переход в режим вычислений с константой. */
				if (calculator.mode != OPER)	/* в режиме выбора операции клавиша "=" игнорируется */
					calculator.mode = SwitchCalculatorMode (calculator.mode, CALC);
					if (calculator.mode == CALC) {
					}
				break;
			case KEYMA:	/* прибавление к ячейке памяти */
			case KEYMS:	/* вычитание из ячейки памяти */
			case KEYMR:	/* получение из ячейки памяти */
			case KEYMC:	/* очистка ячейки памяти */
				calculator.mode = SwitchCalculatorMode (calculator.mode, UNOP);
				if (calculator.mode == UNOP)
					calculator.mode = MemoryAction (key);
				break;
			case KEYST:	/* отображение внутреннего состояния калькулятора */
				DebugCalculatorState ();
				break;
			case KEYQT:	/* завершение работы с калькулятором */
				SendMessage (GetParent (hWndCalc), WM_CLOSE, 0, 0);
				break;
			case KEYCC:	/* копирование содержимого индикатора в буфер обмена */
				ClipboardCopy ();
				/*MessageBox (hWnd, TEXT ("KEYCC"), TEXT ("iacalc.c - HandleKey ()"), MB_OK | MB_ICONINFORMATION);*/
				break;
			case KEYCV:	/* вставка содержимого буфера обмена в индикатор */
				calculator.mode = SwitchCalculatorMode (calculator.mode, NUMB);
				if (calculator.mode == NUMB)
					ClipboardPaste ();
				/* MessageBox (hWnd, TEXT ("KEYCV"), TEXT ("iacalc.c - HandleKey ()"), MB_OK | MB_ICONINFORMATION);*/
				break;
			default:	/* неизвестная клавиша */
				break;
				/*
				wsprintf (msg, TEXT ("The pressed key id code is %d"), key);
				MessageBox (hWnd, msg, TEXT ("iacalc.HandleKey ()"), MB_OK | MB_ICONINFORMATION);*/
		}
}
/*=================================================*/
/* Отображение состояния калькулятора для отладки. */
/*=================================================*/
static void DebugCalculatorState (void)
{
	TCHAR msg[512];
	enum EKeys op = calculator.op;
	wsprintf (msg, TEXT ("regx = [%s]\nregy = [%s]\nop = [%c]\nregm = [%s]\nregc = [%s]"),
		calculator.regx, calculator.regy,
		op == KEYAD? (TCHAR)'+': op == KEYSB? (TCHAR)'-': op == KEYML? (TCHAR)'*': op == KEYDV? (TCHAR)'/': op == KEYEQ? (TCHAR)'=' : (TCHAR)'?', calculator.regm, calculator.regc);
	MessageBox (NULL, msg, TEXT ("DebugCalculatorState ()"), MB_OK | MB_ICONINFORMATION);
}