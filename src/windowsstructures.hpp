// include/windowsstructures.hpp
#pragma once

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>

namespace CustomAccent {
// Accent constants
static const DWORD ACCENT_ENABLE_BLURBEHIND = 3;
static const DWORD WCA_ACCENT_POLICY = 19;
}

struct ACCENTPOLICY {
    DWORD AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
};

// We don't need to redefine these as they're already in shellapi.h
// struct APPBARDATA and SHAppBarMessage are already defined

#endif
