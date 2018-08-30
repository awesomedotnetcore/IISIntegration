// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once
#include "IOutputManager.h"
#include "StdWrapper.h"

class BaseOutputManager :
    public IOutputManager
{
public:
    BaseOutputManager() : BaseOutputManager(/* fEnableNativeLogging */ true) {}
    BaseOutputManager(bool enableNativeLogging) :
        m_disposed(false),
        stdoutWrapper(nullptr),
        stderrWrapper(nullptr),
        m_enableNativeRedirection(enableNativeLogging)
    {
        InitializeSRWLock(&m_srwLock);
    }
    ~BaseOutputManager() {}

protected:
    std::wstring m_stdOutContent;
    bool m_disposed;
    bool m_enableNativeRedirection;
    SRWLOCK m_srwLock{};
    std::unique_ptr<StdWrapper> stdoutWrapper;
    std::unique_ptr<StdWrapper> stderrWrapper;
};

