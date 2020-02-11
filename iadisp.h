#ifndef IADISP_H
#define IADISP_H 1
#define IDDISPLAY 200
/* CAPACITY - ����������� ������� � ������ + ������� ����� + ������� ����������� ����� � ������� ����� + ������������ */
/* ��� 10-���������� ������������ �������� CAPACITY ������ ���� ����� 13. */
#define CAPACITY 13
/*------------------------------------------*/
/* ���������� ������� ������� ������������. */
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