#ifndef IAARITHM_H
#define IAARITHM_H 1
/*===========================================================================================*/
/* ������ ���������� �������������� �������� ��� ������� � ������������ ����������� �������. */
/*===========================================================================================*/
enum EOpRes {
	OPERR,	/* ������ ��� ���������� �������� */
	OPOK,	/* �������� ��������� ������� */
	OPOVR	/* �������� ������������ */
};
enum EOpRes OpAdd (LPTSTR x, LPCTSTR y);
enum EOpRes OpSub (LPTSTR x, LPCTSTR y);
enum EOpRes OpMul (LPTSTR x, LPCTSTR y);
enum EOpRes OpDiv (LPTSTR x, LPCTSTR y);
#endif