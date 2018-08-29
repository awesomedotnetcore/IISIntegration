// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "stdafx.h"
#include "LoggingHelpers.h"
#include "IOutputManager.h"
#include "FileOutputManager.h"
#include "PipeOutputManager.h"
#include "NullOutputManager.h"
#include "debugutil.h"
#include <Windows.h>
#include <io.h>
#include "ntassert.h"
#include "exceptions.h"
#include "EventLog.h"

HRESULT
LoggingHelpers::CreateLoggingProvider(
    bool fIsLoggingEnabled,
    bool ,
    PCWSTR pwzStdOutFileName,
    PCWSTR pwzApplicationPath,
    std::unique_ptr<IOutputManager>& outputManager
)
{
    HRESULT hr = S_OK;

    DBG_ASSERT(outputManager != NULL);

    try
    {
        if (fIsLoggingEnabled)
        {
            auto manager = std::make_unique<FileOutputManager>(pwzStdOutFileName, pwzApplicationPath, false);
            outputManager = std::move(manager);
        }
        else if (!GetConsoleWindow())
        {
            outputManager = std::make_unique<PipeOutputManager>(false);
        }
        else
        {
            outputManager = std::make_unique<NullOutputManager>();
        }
    }
    catch (std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

void
LoggingHelpers::StartRedirection(
    std::unique_ptr<IOutputManager>& outputManager,
    std::wstring exceptionMessage)
{
    auto startLambda = [](std::unique_ptr<IOutputManager>& outputManager) { outputManager->Start(); };

    LoggingHelpers::TryOperation(startLambda, outputManager, exceptionMessage);
}

void
LoggingHelpers::StopRedirection(
    std::unique_ptr<IOutputManager>& outputManager,
    std::wstring exceptionMessage)
{
    auto stopLambda = [](std::unique_ptr<IOutputManager>& outputManager) { outputManager->Stop(); };

    LoggingHelpers::TryOperation(stopLambda, outputManager, exceptionMessage);
}

void
LoggingHelpers::TryOperation(void(*func)(std::unique_ptr<IOutputManager>& outputManager),
    std::unique_ptr<IOutputManager>& outputManager,
    std::wstring exceptionMessage)
{
    try
    {
        func(outputManager);
        // insert operation
    }
    catch (ResultException& exception)
    {
        EventLog::Warn(ASPNETCORE_EVENT_GENERAL_WARNING, exceptionMessage.c_str(), exception.GetResult());
    }
    catch (...)
    {
        OBSERVE_CAUGHT_EXCEPTION();
    }
}
