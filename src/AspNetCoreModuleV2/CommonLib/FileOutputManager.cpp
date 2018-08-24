// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "stdafx.h"
#include "FileOutputManager.h"
#include "sttimer.h"
#include "exceptions.h"
#include "debugutil.h"
#include "SRWExclusiveLock.h"
#include "file_utility.h"
#include "StdWrapper.h"
#include "StringHelpers.h"

extern HINSTANCE    g_hModule;

FileOutputManager::FileOutputManager(PCWSTR pwzStdOutLogFileName, PCWSTR pwzApplicationPath) :
    FileOutputManager(pwzStdOutLogFileName, pwzApplicationPath, /* fEnableNativeLogging */ true) { }

FileOutputManager::FileOutputManager(PCWSTR pwzStdOutLogFileName, PCWSTR pwzApplicationPath, bool fEnableNativeLogging) :
    m_hLogFileHandle(INVALID_HANDLE_VALUE),
    m_disposed(false),
    stdoutWrapper(nullptr),
    stderrWrapper(nullptr),
    m_enableNativeRedirection(fEnableNativeLogging),
    m_applicationPath(pwzApplicationPath),
    m_stdOutLogFileName(pwzStdOutLogFileName)
{
    InitializeSRWLock(&m_srwLock);
}

FileOutputManager::~FileOutputManager()
{
    FileOutputManager::Stop();
}

// Start redirecting stdout and stderr into the file handle.
// Uses sttimer to continuously flush output into the file.
HRESULT
FileOutputManager::Start()
{
    SYSTEMTIME systemTime;
    SECURITY_ATTRIBUTES saAttr = { 0 };
    FILETIME processCreationTime;
    FILETIME dummyFileTime;

    // Concatenate the log file name and application path
    auto logPath = m_applicationPath / m_stdOutLogFileName;
    create_directories(logPath.parent_path());

    RETURN_LAST_ERROR_IF(!GetProcessTimes(
        GetCurrentProcess(), 
        &processCreationTime, 
        &dummyFileTime, 
        &dummyFileTime, 
        &dummyFileTime));
    RETURN_LAST_ERROR_IF(!FileTimeToSystemTime(&processCreationTime, &systemTime));

    m_logFilePath = format(L"%s_%d%02d%02d%02d%02d%02d_%d_%s.log",
        logPath.c_str(),
        systemTime.wYear,
        systemTime.wMonth,
        systemTime.wDay,
        systemTime.wHour,
        systemTime.wMinute,
        systemTime.wSecond,
        GetCurrentProcessId(),
        fsPath.filename().stem().c_str());

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create the file with both READ and WRITE.
    m_hLogFileHandle = CreateFileW(m_logFilePath.c_str(),
        FILE_READ_DATA | FILE_WRITE_DATA,
        FILE_SHARE_READ,
        &saAttr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    RETURN_LAST_ERROR_IF(m_hLogFileHandle == INVALID_HANDLE_VALUE);

    stdoutWrapper = std::make_unique<StdWrapper>(stdout, STD_OUTPUT_HANDLE, m_hLogFileHandle, m_enableNativeRedirection);
    stderrWrapper = std::make_unique<StdWrapper>(stderr, STD_ERROR_HANDLE, m_hLogFileHandle, m_enableNativeRedirection);

    stdoutWrapper->StartRedirection();
    stderrWrapper->StartRedirection();

    return S_OK;
}


HRESULT
FileOutputManager::Stop()
{
    CHAR            pzFileContents[MAX_FILE_READ_SIZE] = { 0 };
    DWORD           dwNumBytesRead;
    LARGE_INTEGER   li = { 0 };
    DWORD           dwFilePointer = 0;
    HANDLE   handle = NULL;
    WIN32_FIND_DATA fileData;

    if (m_disposed)
    {
        return S_OK;
    }

    SRWExclusiveLock lock(m_srwLock);

    if (m_disposed)
    {
        return S_OK;
    }

    m_disposed = true;

    if (m_hLogFileHandle == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    FlushFileBuffers(m_hLogFileHandle);

    if (stdoutWrapper != nullptr)
    {
        RETURN_IF_FAILED(stdoutWrapper->StopRedirection());
    }

    if (stderrWrapper != nullptr)
    {
        RETURN_IF_FAILED(stderrWrapper->StopRedirection());
    }

    // delete empty log file
    handle = FindFirstFile(m_logFilePath.c_str(), &fileData);
    if (handle != INVALID_HANDLE_VALUE &&
        handle != NULL &&
        fileData.nFileSizeHigh == 0 &&
        fileData.nFileSizeLow == 0) // skip check of nFileSizeHigh
    {
        FindClose(handle);
        LOG_LAST_ERROR_IF(!DeleteFile(m_logFilePath.c_str()));
    }

    // Read the first 30Kb from the file and store it in a buffer.
    // By doing this, we can close the handle to the file and be done with it.
    RETURN_LAST_ERROR_IF(!GetFileSizeEx(m_hLogFileHandle, &li));

    if (li.LowPart == 0 || li.HighPart > 0)
    {
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(ERROR_FILE_INVALID));
    }

    dwFilePointer = SetFilePointer(m_hLogFileHandle, 0, NULL, FILE_BEGIN);

    RETURN_LAST_ERROR_IF(dwFilePointer == INVALID_SET_FILE_POINTER);

    RETURN_LAST_ERROR_IF(!ReadFile(m_hLogFileHandle, pzFileContents, MAX_FILE_READ_SIZE, &dwNumBytesRead, NULL));

    int nChars = MultiByteToWideChar(GetConsoleOutputCP(), MB_ERR_INVALID_CHARS, pzFileContents, static_cast<int>(dwNumBytesRead), NULL, 0);
    m_stdOutContent.resize(nChars);

    MultiByteToWideChar(GetConsoleOutputCP(), MB_ERR_INVALID_CHARS, pzFileContents, static_cast<int>(dwNumBytesRead), m_stdOutContent.data(), nChars);

    auto content = GetStdOutContent();
    if (!content.empty())
    {
        // printf will fail in in full IIS
        if (wprintf(content.c_str()) != -1)
        {
            // Need to flush contents for the new stdout and stderr
            _flushall();
        }
    }

    return S_OK;
}

std::wstring FileOutputManager::GetStdOutContent()
{
    return m_stdOutContent;
}
