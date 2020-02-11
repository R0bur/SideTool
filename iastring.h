/*---------------------------------*/
/* Функции для работы со строками. */
/*---------------------------------*/
#ifndef UNICODE
#define StrLen strlen
#define StrCpy strcpy
#define StrNCpy strncpy
#define StrCmp strcmp
#define StrNCmp strncmp
#else
#define StrLen wcslen
#define StrCpy wcscpy
#define StrNCpy wcsncpy
#define StrCmp wcscmp
#define StrNCmp wcsncmp
#endif
