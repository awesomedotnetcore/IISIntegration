// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "aspnetcore_shim_config.h"

#include "config_utility.h"
#include "StringHelpers.h"

#define CS_ASPNETCORE_SECTION                            L"system.webServer/aspNetCore"
#define CS_ASPNETCORE_PROCESS_EXE_PATH                   L"processPath"
#define CS_ASPNETCORE_PROCESS_ARGUMENTS                  L"arguments"
#define CS_ASPNETCORE_HOSTING_MODEL                      L"hostingModel"
#define CS_ASPNETCORE_STDOUT_LOG_ENABLED                 L"stdoutLogEnabled"
#define CS_ASPNETCORE_STDOUT_LOG_FILE                    L"stdoutLogFile"
#define CS_ASPNETCORE_HANDLER_SETTINGS                   L"handlerSettings"
#define CS_ASPNETCORE_HANDLER_VERSION                    L"handlerVersion"

ASPNETCORE_SHIM_CONFIG::ASPNETCORE_SHIM_CONFIG(ConfigurationSource &configurationSource) :
        m_hostingModel(HOSTING_UNKNOWN),
        m_fStdoutLogEnabled(false)
{
    auto section = configurationSource.GetSection(CS_ASPNETCORE_SECTION);
    auto hostingModel = section->GetString(CS_ASPNETCORE_HOSTING_MODEL).value_or(L"");

    if (hostingModel.empty() || equals_ignore_case(hostingModel, L"outofprocess"))
    {
        m_hostingModel = HOSTING_OUT_PROCESS;
    }
    else if (equals_ignore_case(hostingModel, L"inprocess"))
    {
        m_hostingModel = HOSTING_IN_PROCESS;
    }
    else
    {
        throw ConfigurationLoadException(format(
            L"Unknown hosting model '%s'. Please specify either hostingModel=\"inprocess\" "
            "or hostingModel=\"outofprocess\" in the web.config file.", hostingModel.c_str()));
    }

    if (m_hostingModel == HOSTING_OUT_PROCESS)
    {
        auto handlerSettings = section->GetKeyValuePairs(CS_ASPNETCORE_HANDLER_SETTINGS);
        m_strHandlerVersion = find_element(handlerSettings, CS_ASPNETCORE_HANDLER_VERSION).value_or(std::wstring());
    }

    m_strArguments = section->GetRequiredString(CS_ASPNETCORE_PROCESS_ARGUMENTS);
    m_strProcessPath = section->GetRequiredString(CS_ASPNETCORE_PROCESS_EXE_PATH);
    m_fStdoutLogEnabled = section->GetRequiredBool(CS_ASPNETCORE_STDOUT_LOG_ENABLED);
    m_struStdoutLogFile = section->GetRequiredString(CS_ASPNETCORE_STDOUT_LOG_FILE);
}
