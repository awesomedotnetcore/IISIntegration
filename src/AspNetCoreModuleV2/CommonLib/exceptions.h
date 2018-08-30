// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <exception>
#include <system_error>

#include "debugutil.h"
#include "StringHelpers.h"
#include "InvalidOperationException.h"

#define LOCATION_INFO_ENABLED TRUE

#if LOCATION_INFO_ENABLED
#define LOCATION_FORMAT                                         "%s:%d "
#define LOCATION_ARGUMENTS_ONLY                                 _In_opt_ PCSTR fileName, unsigned int lineNumber
#define LOCATION_ARGUMENTS                                      LOCATION_ARGUMENTS_ONLY,
#define LOCATION_CALL_ONLY                                      fileName, lineNumber
#define LOCATION_CALL                                           LOCATION_CALL_ONLY,
#define LOCATION_INFO                                           __FILE__, __LINE__
#else
#define LOCATION_FORMAT
#define LOCATION_ARGUMENTS_ONLY
#define LOCATION_ARGUMENTS
#define LOCATION_CALL_ONLY
#define LOCATION_CALL
#define LOCATION_INFO
#endif

#define OBSERVE_CAUGHT_EXCEPTION()                              CaughtExceptionHResult(LOCATION_INFO);
#define RETURN_CAUGHT_EXCEPTION()                               return CaughtExceptionHResult(LOCATION_INFO);

#define RETURN_HR(hr)                                           do { HRESULT __hrRet = hr; if (FAILED(__hrRet)) { LogHResultFailed(LOCATION_INFO, __hrRet); } return __hrRet; } while (0, 0)
#define RETURN_LAST_ERROR()                                     do { return LogLastError(LOCATION_INFO); } while (0, 0)
#define RETURN_IF_FAILED(hr)                                    do { HRESULT __hrRet = hr; if (FAILED(__hrRet)) { LogHResultFailed(LOCATION_INFO, __hrRet); return __hrRet; }} while (0, 0)
#define RETURN_LAST_ERROR_IF(condition)                         do { if (condition) { return LogLastError(LOCATION_INFO); }} while (0, 0)
#define RETURN_LAST_ERROR_IF_NULL(ptr)                          do { if ((ptr) == nullptr) { return LogLastError(LOCATION_INFO); }} while (0, 0)

#define FINISHED(hrr)                                           do { HRESULT __hrRet = hrr; LogHResultFailed(LOCATION_INFO, __hrRet); hr = __hrRet; goto Finished; } while (0, 0)
#define FINISHED_IF_FAILED(hrr)                                 do { HRESULT __hrRet = hrr; if (FAILED(__hrRet)) { LogHResultFailed(LOCATION_INFO, __hrRet); hr = __hrRet; goto Finished; }} while (0, 0)
#define FINISHED_IF_NULL_ALLOC(ptr)                             do { if ((ptr) == nullptr) { hr = LogHResultFailed(LOCATION_INFO, E_OUTOFMEMORY); goto Finished; }} while (0, 0)
#define FINISHED_LAST_ERROR_IF(condition)                       do { if (condition) { hr = LogLastError(LOCATION_INFO); goto Finished; }} while (0, 0)
#define FINISHED_LAST_ERROR_IF_NULL(ptr)                        do { if ((ptr) == nullptr) { hr = LogLastError(LOCATION_INFO); goto Finished; }} while (0, 0)

#define THROW_LAST_ERROR()                                      do { ThrowResultException(LOCATION_INFO, LogLastError(LOCATION_INFO)); } while (0, 0)
#define THROW_IF_FAILED(hr)                                     do { HRESULT __hrRet = hr; if (FAILED(__hrRet)) { ThrowResultException(LOCATION_INFO, __hrRet); }} while (0, 0)
#define THROW_LAST_ERROR_IF(condition)                          do { if (condition) { ThrowResultException(LOCATION_INFO, LogLastError(LOCATION_INFO)); }} while (0, 0)
#define THROW_LAST_ERROR_IF_NULL(ptr)                           do { if ((ptr) == nullptr) { ThrowResultException(LOCATION_INFO, LogLastError(LOCATION_INFO)); }} while (0, 0)

#define THROW_IF_NULL_ALLOC(ptr)                                Throw_IfNullAlloc(ptr)

#define CATCH_RETURN()                                          catch (...) { RETURN_CAUGHT_EXCEPTION(); }
#define LOG_IF_FAILED(hr)                                       LogHResultFailed(LOCATION_INFO, hr)
#define LOG_LAST_ERROR()                                        LogLastErrorIf(LOCATION_INFO, true)
#define LOG_LAST_ERROR_IF(condition)                            LogLastErrorIf(LOCATION_INFO, condition)
#define SUCCEEDED_LOG(hr)                                       SUCCEEDED(LOG_IF_FAILED(hr))
#define FAILED_LOG(hr)                                          FAILED(LOG_IF_FAILED(hr))


class ResultException: public std::runtime_error
{
public:
    explicit ResultException(HRESULT hr, LOCATION_ARGUMENTS_ONLY) :
        runtime_error(format("HRESULT 0x%x returned at " LOCATION_FORMAT, hr, LOCATION_CALL_ONLY)),
        m_hr(hr)
    {
    }

    HRESULT GetResult() const { return m_hr; }

private:
    HRESULT m_hr;
};

 __declspec(noinline) inline VOID ReportUntypedException(LOCATION_ARGUMENTS_ONLY)
{
    DebugPrintf(ASPNETCORE_DEBUG_FLAG_ERROR, LOCATION_FORMAT "Unhandled non-standard exception", LOCATION_CALL_ONLY);
}

 __declspec(noinline) inline HRESULT LogLastError(LOCATION_ARGUMENTS_ONLY)
{
    const auto lastError = GetLastError();
    const auto hr = HRESULT_FROM_WIN32(lastError);

    DebugPrintf(ASPNETCORE_DEBUG_FLAG_ERROR, LOCATION_FORMAT "Operation failed with LastError: %d HR: 0x%x", LOCATION_CALL lastError, hr);

    return hr;
}

 __declspec(noinline) inline bool LogLastErrorIf(LOCATION_ARGUMENTS_ONLY, bool condition)
{
    if (condition)
    {
        LogLastError(LOCATION_CALL_ONLY);
    }

    return condition;
}

 __declspec(noinline) inline VOID ReportException(LOCATION_ARGUMENTS std::exception& exception)
{
    DebugPrintf(ASPNETCORE_DEBUG_FLAG_ERROR, "Exception '%s' caught at " LOCATION_FORMAT, exception.what(), LOCATION_CALL_ONLY);
}

 __declspec(noinline) inline HRESULT LogHResultFailed(LOCATION_ARGUMENTS HRESULT hr)
{
    if (FAILED(hr))
    {
        DebugPrintf(ASPNETCORE_DEBUG_FLAG_ERROR,  "Failed HRESULT returned: 0x%x at " LOCATION_FORMAT, hr, LOCATION_CALL_ONLY);
    }
    return hr;
}

__declspec(noinline) inline HRESULT CaughtExceptionHResult(LOCATION_ARGUMENTS_ONLY)
{
    try
    {
        throw;
    }
    catch (const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    catch (ResultException& exception)
    {
        ReportException(LOCATION_CALL exception);
        return exception.GetResult();
    }
    catch (std::exception& exception)
    {
        ReportException(LOCATION_CALL exception);
        return HRESULT_FROM_WIN32(ERROR_UNHANDLED_EXCEPTION);
    }
    catch (...)
    {
        ReportUntypedException(LOCATION_CALL_ONLY);
        return HRESULT_FROM_WIN32(ERROR_UNHANDLED_EXCEPTION);
    }
}

[[noreturn]]
 __declspec(noinline) inline void ThrowResultException(LOCATION_ARGUMENTS HRESULT hr)
{
    DebugPrintf(ASPNETCORE_DEBUG_FLAG_ERROR,  "Throwing ResultException for HRESULT 0x%x at " LOCATION_FORMAT, hr, LOCATION_CALL_ONLY);
    throw ResultException(hr, LOCATION_CALL_ONLY);
}

template <typename PointerT> auto Throw_IfNullAlloc(PointerT pointer)
{
    if (pointer == nullptr)
    {
        throw std::bad_alloc();
    }
    return pointer;
}
__declspec(noinline) inline std::wstring GetUnexpectedExceptionMessage(std::runtime_error& ex)
{
    return format(L"Unexpected exception: %S", ex.what());
}
