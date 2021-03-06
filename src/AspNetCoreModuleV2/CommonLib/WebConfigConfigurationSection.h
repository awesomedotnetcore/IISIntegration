// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <atlcomcli.h>
#include <optional>
#include "ConfigurationSection.h"

class WebConfigConfigurationSection: public ConfigurationSection
{
public:
    WebConfigConfigurationSection(IAppHostElement* pElement)
        : m_element(pElement)
    {
    }

    std::optional<std::wstring> GetString(const std::wstring& name) const override;
    std::optional<bool> GetBool(const std::wstring& name) const override;
    std::optional<DWORD> GetTimespan(const std::wstring& name) const override;
    std::vector<std::pair<std::wstring, std::wstring>> GetKeyValuePairs(const std::wstring& name) const override;

private:
    CComPtr<IAppHostElement> m_element;
};
