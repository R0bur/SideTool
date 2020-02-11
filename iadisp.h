#ifndef IADISP_H
#define IADISP_H 1
#define IDDISPLAY 200
/* CAPACITY - разрядность дисплея в цифрах + позиция знака + позиция разделителя целой и дробной части + ограничитель */
/* Для 10-разрядного калькулятора значение CAPACITY должно быть равно 13. */
#define CAPACITY 13
/*------------------------------------------*/
/* Объявления функций дисплея калькулятора. */
/*------------------------------------------*/
LPCTSTR DisplayGetZero (void);
HWND DisplayCreate (HWND hWnd, LPRECT lpRc);
void DisplayClearAll (void);
void DisplayClearLastDigit (void);
void DisplayDecimalPoint (void);
void DisplayConcatDigit (TCHAR digit);
LPCTSTR DisplayGetValue (void);
void DisplaySetValue (LPCTSTR v);
#endif