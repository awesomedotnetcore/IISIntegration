// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include "IOutputManager.h"

class LoggingHelpers
{
public:

    static
    HRESULT
    CreateLoggingProvider(
        bool fLoggingEnabled,
        bool fEnableNativeLogging,
        PCWSTR pwzStdOutFileName,
        PCWSTR pwzApplicationPath,
        std::unique_ptr<IOutputManager>& outputManager
    );

    static void TryOperation(void(*func)(std::unique_ptr<IOutputManager>& outputManager),
        std::unique_ptr<IOutputManager>& outputManager,
        std::wstring exceptionMessage);

    static void StartRedirection(
        std::unique_ptr<IOutputManager>& outputManager,
        std::wstring exceptionMessage);

    static void StopRedirection(
        std::unique_ptr<IOutputManager>& outputManager,
        std::wstring exceptionMessage);
};

