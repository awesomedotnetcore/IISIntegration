// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "IOutputManager.h"
#include "StdWrapper.h"
#include "stringu.h"

class PipeOutputManager : public IOutputManager
{
    // Timeout to be used if a thread never exits
    #define PIPE_OUTPUT_THREAD_TIMEOUT 2000

    // Max event log message is ~32KB, limit pipe size just below that.
    #define MAX_PIPE_READ_SIZE 30000
public:
    PipeOutputManager();
    PipeOutputManager(bool fEnableNativeLogging);
    ~PipeOutputManager();

    void Start() override;
    void Stop() override;
    std::wstring GetStdOutContent() override;

    // Thread functions
    void ReadStdErrHandleInternal();

    static void ReadStdErrHandle(LPVOID pContext);

private:

    HANDLE                          m_hErrReadPipe;
    HANDLE                          m_hErrWritePipe;
    STRU                            m_logFilePath;
    HANDLE                          m_hErrThread;
    CHAR                            m_pipeContents[MAX_PIPE_READ_SIZE] = { 0 };
    DWORD                           m_numBytesReadTotal;
    SRWLOCK                         m_srwLock;
    BOOL                            m_disposed;
    BOOL                            m_enableNativeRedirection;
    std::unique_ptr<StdWrapper>     stdoutWrapper;
    std::unique_ptr<StdWrapper>     stderrWrapper;
    std::wstring                    m_stdOutContent;
};
