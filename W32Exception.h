#pragma once

#include <Windows.h>
#include <string>

class W32Exception {
public:
    W32Exception(wchar_t const* what);

    // Getter functions
    unsigned long code() const { return _code; }
    wchar_t const* what() const { return _what; }
    WCHAR const* message() const { return _message; }
private:
    unsigned long _code;
    wchar_t const* _what;
    WCHAR _message[1024];
};