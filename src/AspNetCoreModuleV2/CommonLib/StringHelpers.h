#pragma once

#include <string>

[[nodiscard]]
bool ends_with(const std::wstring &source, const std::wstring &suffix, bool ignoreCase = false);

template<typename ... Args>
[[nodiscard]]
std::wstring format(const std::wstring& format, Args ... args)
{
    const size_t size = _scwprintf(format.c_str(), args ...) + 1; // Extra char for '\0'
    std::unique_ptr<wchar_t[]> formattedBuffer(new wchar_t[size]);
    _snwprintf_s(formattedBuffer.get(), size, _TRUNCATE, format.c_str(), args ... );
    return std::wstring(formattedBuffer.get(), formattedBuffer.get() + size - 1);
}
