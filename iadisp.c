/*=======================*/
/* Индикатор калькулятора. */
/*=======================*/
#include <windows.h>
#include "iadisp.h"
#include "iastring.h"
/*-------------------------*/
/* Сведения об индикаторе. */
/*-------------------------*/
static void DisplayUpdate (void);
#define ZERO "0,"
static struct {
	TCHAR value[CAPACITY];	/* строка, отображаемая на индикаторе */
	int nCap;	/* разрядность калькулятора (только цифры) */
	int n;		/* текущее заполнение (только цифры) */
	int m;		/* номер разряда, после которого стоит десятичный разделитель */
	HWND hWnd;	/* манипулятор элемента управления */
	HFONT hFont, hFontOld;	/* текущий шрифт и старый шрифт элемента управления */
} display;
/*==============================================================*/
/* Получение "индикаторного нуля".                              */
/* Возврат: указатель на строку с нулевым значением индикатора. */
/*==============================================================*/
LPCTSTR DisplayGetZero (void)
{
	return ZERO;
}
/*=============================================================*/
/* Создание индикатора.                                        */
/* Вызов: hWnd - родительское окно,                            */
/*        lpRc - указатель на область, отведённую для дисплея. */
/* Возврат: манипулятор окна дисплея.                          */
/*=============================================================*/
HWND DisplayCreate (HWND hWnd, LPRECT lpRc)
{
	BOOL lRes;
	HWND hWndTmp;
	LOGFONT lf;
	RECT rc;
	/* Создание рамки. */
	rc = *lpRc;
	hWndTmp = CreateWindow (TEXT ("STATIC"), NULL, WS_CHILD | WS_VISIBLE | SS_GRAYRECT,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		hWnd, NULL, (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE), NULL);
	/* Создание дисплея. */
	rc.left += 2;
	rc.top += 2;
	rc.right -= 2;
	rc.bottom -= 2;
	hWndTmp = CreateWindow (TEXT ("STATIC"), TEXT (ZERO), WS_CHILD | WS_VISIBLE | SS_RIGHT,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		hWnd, (HMENU) IDDISPLAY, (HINSTANCE) GetWindowLong (hWnd, GWL_HINSTANCE), NULL);
	/* Настройка шрифта дисплея калькулятора. */
	/* TODO: Реализовать вычисление размера шрифта в соответствии с размерами предоставленной области отображения. */
	GetObject (GetStockObject (ANSI_FIXED_FONT), sizeof (LOGFONT), &lf);
	lf.lfWeight = FW_BOLD;
	lf.lfHeight = 36;
	lf.lfWidth = 16;
	display.hFont = CreateFont (lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation,
		lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
		lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
	display.hFontOld = (HFONT) SendMessage (hWndTmp, WM_GETFONT, 0, 0);
	SendMessage (hWndTmp, WM_SETFONT, (WPARAM) display.hFont, MAKELONG (0, 0));
	/* Окончательная инициализация. */
	display.nCap = CAPACITY - 3;
	StrCpy (display.value, ZERO);
	display.n = 1;
	display.m = 0;
	display.hWnd = hWndTmp;
	return hWndTmp;
}
/*===========================================================*/
/* Обновление значения, отображаемого на индикаторе дисплея. */
/*===========================================================*/
static void DisplayUpdate (void)
{
	SendMessage (display.hWnd, WM_SETTEXT, 0, (LPARAM) TEXT (display.value));
}
/*==================================*/
/* Очистка индикатора калькулятора. */
/*==================================*/
void DisplayClearAll (void)
{
	/* MessageBox (NULL, TEXT ("DisplayClearAll ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	StrCpy (display.value, ZERO);
	display.n = 1;
	display.m = 0;
	DisplayUpdate ();
}
/*======================================================*/
/* Удаление последней цифры на индикаторе калькулятора. */
/*======================================================*/
void DisplayClearLastDigit (void)
{
	int p, q;
	/* MessageBox (NULL, TEXT ("DisplayClearLastDigit ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	if (display.m == display.n) {
		/* Сброс позиции десятичной точки. */
		display.m = 0;
	}
	else {
		q = display.value[0] == (TCHAR)'-'? 1: 0;	/* наличие разряда под знак */
		if (display.n > 1) {
			/* Удаление последней цифры на дисплее. */
			p = display.n + q;
			if (display.m > 0) {
				/* Удаление цифры из дробной части числа. */
				display.value[p] = display.value[p + 1];	/* сдвиг признака конца строки влево */
				display.n--;	/* учёт удалённой цифры */
			}
			else {
				/* Удаление цифры из целой части числа. */
				display.value[p - 1] = display.value[p];	/* сдвиг десятичного разделителя влево. */
				display.value[p] = display.value[p + 1];	/* сдвиг признака конца строки влево. */
				display.n--;	/* учёт удалённой цифры */
			}	
		}
		else
			/* Удаление единственной цифры на дисплее. */
			display.value[q] = (TCHAR)'0';
		/* Обновление показания индикатора. */
		DisplayUpdate ();
	}
}
/*=============================*/
/* Обработка десятичной точки. */
/*=============================*/
void DisplayDecimalPoint (void)
{
	/* MessageBox (NULL, TEXT ("DisplayDecimalPoint ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	if (display.m == 0)
		display.m = display.n;
}
/*========================*/
/* Изменение знака числа. */
/*========================*/
void DisplaySign (void)
{
	int i, n;
	n = display.n;
	if (display.value[0] == (TCHAR)'-') {
		/* число отрицательное */
		for (i = 0; i <= n; i++)
			display.value[i] = display.value[i + 1];	/* сдвиг цифр и десятичного разделителя влево */
		display.value[i] = display.value[i + 1];		/* сдвиг признака конца строки влево */
	}
	else {
		/* число не отрицательное */
		display.value[n + 2] = display.value[n + 1];		/* сдвиг признака конца строки вправо */
		for (i = n + 1; i > 0; i--)
			display.value[i] = display.value[i - 1];	/* сдвиг цифр и десятичного разделителя вправо */
		display.value[0] = (TCHAR)'-';
	}
	/* Обновление показания индикатора. */
	DisplayUpdate ();
}
/*=============================================*/
/* Добавление цифры на индикатор калькулятора. */
/* Вызов: digit - цифра.                       */
/*=============================================*/
void DisplayConcatDigit (TCHAR digit)
{
	/* MessageBox (NULL, TEXT ("DisplayConcatDigit ()"), TEXT ("Debug"), MB_OK | MB_ICONINFORMATION); */
	int p, q;
	if (display.n < display.nCap) {
		/* Есть свободные разряды для цифр. */
		q = display.value[0] == (TCHAR)'-'? 1: 0;	/* наличие разряда под знак */
		p = display.n + q;				/* количество разрядов с учётом знака */
		if (display.m == 0) {
			/* На дисплее - целое число. */
			if (display.n == 1 && display.value[q] == (TCHAR)'0')
				/* На дисплее единственный ноль. */
				display.value[q] = digit;
			else {
				display.value[p + 2] = display.value[p + 1];	/* сдвиг признака конца строки вправо */
				display.value[p + 1] = display.value[p];	/* сдвиг десятичного разделителя вправо */
				display.value[p] = digit;			/* вставка введённой цифры */
				display.n++;	/* учёт заполненного разряда */
			}
		}
		else {
			/* На дисплее - дробное число. */
			display.value[p + 2] = display.value[p + 1];	/* сдвиг признака конца строки вправо */
			display.value[p + 1] = digit;			/* вставка введённой цифры */
			display.n++;	/* учёт заполненного разряда */
		}
	}
	DisplayUpdate ();	
}
/*================================================================*/
/* Получение значения, отображаемого на индикаторе.               */
/* Возврат: константный указатель на строку, содержащую значение. */
/*================================================================*/
LPCTSTR DisplayGetValue (void)
{
	return (LPCTSTR)display.value;
}
/*=============================================================*/
/* Установка значения для отображения на индикаторе.           */
/* Вызов: v - константный указатель на строку для отображения. */
/*=============================================================*/
void DisplaySetValue (LPCTSTR v)
{
	int i, n, m, dp;
	/* Посчёт разрядов в устанавливаемом значении. */
	dp = 1;
	n = 0;
	m = 0;	/* номер разряда, после которого стоит десятичный разделитель, если есть дробная часть */
	for (i = 0; v[i] != (TCHAR)'\0'; i++)
		if ((TCHAR)'0' <= v[i] && v[i] <= (TCHAR)'9')
			n++;
		else if (v[i] == (TCHAR)'.' || v[i] == (TCHAR)',')
			m = n;
	display.n = n;
	display.m = n > m? m: 0;
	/* Отображение значения на индикаторе. */
	StrNCpy (display.value, v, CAPACITY);
	(display.value)[CAPACITY - 1] = (TCHAR) '\0';
	DisplayUpdate ();
}
/*=========================*/
/* Уничтожение индикатора. */
/*=========================*/
void DisplayDestroy (void)
{
}