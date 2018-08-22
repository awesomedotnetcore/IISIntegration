#include "ConfigurationSource.h"
#include "ahutil.h"
#include "exceptions.h"
#include "StringHelpers.h"

std::wstring ConfigurationSection::GetRequiredString(const std::wstring& name)
{
    auto result = GetString(name);
    if (!result.has_value() || result.value().empty())
    {
        ThrowRequiredException(name);
    }
    return result.value();
}

bool ConfigurationSection::GetRequiredBool(const std::wstring& name)
{
    auto result = GetBool(name);
    if (!result.has_value())
    {
        ThrowRequiredException(name);
    }
    return result.value();
}

DWORD ConfigurationSection::GetRequiredTimespan(const std::wstring& name)
{
    auto result = GetTimespan(name);
    if (!result.has_value())
    {
        ThrowRequiredException(name);
    }
    return result.value();
}

void ConfigurationSection::ThrowRequiredException(const std::wstring& name)
{
    throw ConfigurationLoadException(format(L"Attribute '%s' is required", name.c_str()));
}

std::optional<std::wstring> WebConfigConfigurationSection::GetString(const std::wstring& name)
{
    CComBSTR result;
    if (FAILED_LOG(GetElementStringProperty(m_element, name.c_str(), &result.m_str)))
    {
        return std::nullopt;
    }

    return std::make_optional(std::wstring(result));
}

std::optional<bool> WebConfigConfigurationSection::GetBool(const std::wstring& name)
{
    bool result;
    if (FAILED_LOG(GetElementBoolProperty(m_element, name.c_str(), &result)))
    {
        return std::nullopt;
    }

    return std::make_optional(result);
}

std::optional<DWORD> WebConfigConfigurationSection::GetTimespan(const std::wstring& name)
{
    ULONGLONG result;
    if (FAILED_LOG(GetElementRawTimeSpanProperty(m_element, name.c_str(), &result)))
    {
        return std::nullopt;
    }

    return std::make_optional(static_cast<DWORD>(result / 10000ull));
}

std::vector<std::pair<std::wstring, std::wstring>> WebConfigConfigurationSection::GetKeyValuePairs(const std::wstring& name)
{
    std::vector<std::pair<std::wstring, std::wstring>> pairs;
    HRESULT findElementResult;
    CComPtr<IAppHostElement>           element = nullptr;
    CComPtr<IAppHostElementCollection> elementCollection = nullptr;
    CComPtr<IAppHostElement>           collectionEntry = nullptr;
    ENUM_INDEX                         index{};


    if (FAILED_LOG(GetElementChildByName(m_element, name.c_str(), &element)))
    {
        return pairs;
    }

    THROW_IF_FAILED(element->get_Collection(&elementCollection));
    THROW_IF_FAILED(findElementResult = FindFirstElement(elementCollection, &index, &collectionEntry));

    while (findElementResult != S_FALSE)
    {
        CComBSTR strHandlerName;
        if (LOG_IF_FAILED(GetElementStringProperty(collectionEntry, L"name", &strHandlerName.m_str)))
        {
            ThrowRequiredException(L"name");
        }

        CComBSTR strHandlerValue;
        if (LOG_IF_FAILED(GetElementStringProperty(collectionEntry, L"value", &strHandlerName.m_str)))
        {
            ThrowRequiredException(L"value");
        }

        pairs.emplace_back(strHandlerName, strHandlerValue);

        collectionEntry.Release();

        THROW_IF_FAILED(findElementResult = FindNextElement(elementCollection, &index, &collectionEntry));
    }

    return pairs;
}

std::shared_ptr<ConfigurationSection> WebConfigConfigurationSource::GetSection(const std::wstring& name)
{
    const CComBSTR bstrAspNetCoreSection = name.c_str();
    const CComBSTR applicationConfigPath = m_application.GetAppConfigPath();

    IAppHostElement* sectionElement;
    THROW_IF_FAILED(m_manager->GetAdminSection(bstrAspNetCoreSection, applicationConfigPath, &sectionElement));
    return std::make_unique<WebConfigConfigurationSection>(sectionElement);
}

std::optional<std::wstring> find_element(std::vector<std::pair<std::wstring, std::wstring>>& pairs, const std::wstring& name)
{
    const auto iter = std::find_if(
        pairs.begin(),
        pairs.end(),
        [&](const std::pair<std::wstring, std::wstring>& pair) { return equals_ignore_case(pair.first, name); });

    if (iter == pairs.end())
    {
        return std::nullopt;
    }

    return std::make_optional(iter->second);
}
