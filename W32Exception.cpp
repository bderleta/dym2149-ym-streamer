#include "W32Exception.h"

W32Exception::W32Exception(wchar_t const* what)
    : _code(::GetLastError()),
    _what(what), _message(L"Unknown error") {
    DWORD messageLength = ::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, _code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&_message, sizeof(_message), NULL);
    if (!messageLength && GetLastError() == ERROR_MUI_FILE_NOT_FOUND) {
        messageLength = ::FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, _code, MAKELANGID(LANG_SYSTEM_DEFAULT, SUBLANG_DEFAULT), (LPWSTR)&_message, sizeof(_message), NULL);
    }
}