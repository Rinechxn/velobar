// include/windowsapi.hpp
#pragma once

#ifdef Q_OS_WIN
#include <windows.h>

// Function prototype for SetWindowCompositionAttribute
typedef struct _WINCOMPATTRDATA
{
    DWORD Attribute;
    PVOID Data;
    ULONG SizeOfData;
} WINCOMPATTRDATA;

typedef BOOL(WINAPI* SetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
#endif
