#ifndef SIDETOOL_H
#define SIDETOOL_H 1
/*------------------------*/
/* Сведения о приложении. */
/*------------------------*/
struct Application {
	LPTSTR lpName;		/* название приложения */
	HINSTANCE hInstance;	/* экземпляр приложения */
	HWND hMainWindow;	/* манипулятор главного окна приложения */
};
#endif