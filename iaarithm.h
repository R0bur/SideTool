#ifndef IAARITHM_H
#define IAARITHM_H 1
/*===========================================================================================*/
/* ћодуль выполнени€ арифметических операций над числами с естественным размещением зап€той. */
/*===========================================================================================*/
enum EOpRes {
	OPERR,	/* ошибка при выполнении операции */
	OPOK,	/* операци€ выполнена успешно */
	OPOVR	/* возникло переполнение */
};
enum EOpRes OpAdd (LPTSTR x, LPCTSTR y);
enum EOpRes OpSub (LPTSTR x, LPCTSTR y);
enum EOpRes OpMul (LPTSTR x, LPCTSTR y);
enum EOpRes OpDiv (LPTSTR x, LPCTSTR y);
#endif