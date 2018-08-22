#pragma once

#include <string>
#include <optional>
#include <atlcomcli.h>
#include "NonCopyable.h"

class ConfigurationSection: NonCopyable
{
public:
    ConfigurationSection() = default;
    virtual ~ConfigurationSection() = default;
    virtual std::optional<std::wstring> GetString(const std::wstring& name) = 0;
    virtual std::optional<bool> GetBool(const std::wstring& name) = 0;
    virtual std::optional<DWORD> GetTimespan(const std::wstring& name) = 0;

    std::wstring GetRequiredString(const std::wstring& name);
    bool GetRequiredBool(const std::wstring& name);
    DWORD GetRequiredTimespan(const std::wstring& name);

    virtual std::vector<std::pair<std::wstring, std::wstring>> GetKeyValuePairs(const std::wstring& name) = 0;

private:
    static void ThrowRequiredException(const std::wstring& name);
};

class ConfigurationSource: NonCopyable
{
public:
    ConfigurationSource() = default;
    virtual ~ConfigurationSource() = default;
    virtual std::shared_ptr<ConfigurationSection> GetSection(const std::wstring& name) = 0;
};

class WebConfigConfigurationSection: public ConfigurationSection
{
public:
    WebConfigConfigurationSection(IAppHostElement* pElement)
        : m_element(pElement)
    {
    }

    std::optional<std::wstring> GetString(const std::wstring& name) override;
    std::optional<bool> GetBool(const std::wstring& name) override;
    std::optional<DWORD> GetTimespan(const std::wstring& name) override;
    std::vector<std::pair<std::wstring, std::wstring>> GetKeyValuePairs(const std::wstring& name) override;

private:
    CComPtr<IAppHostElement> m_element;
};


class WebConfigConfigurationSource: public ConfigurationSource
{
public:
    WebConfigConfigurationSource(IAppHostAdminManager *pAdminManager, IHttpApplication *pHttpApplication)
        : m_manager(pAdminManager),
          m_application(pHttpApplication)
    {
    }

    std::shared_ptr<ConfigurationSection> GetSection(const std::wstring& name) override;

private:
    CComPtr<IAppHostAdminManager> m_manager;
    CComPtr<IHttpApplication> m_application;
};

std::optional<std::wstring> find_element(std::vector<std::pair<std::wstring, std::wstring>>& pairs, const std::wstring& name);

class ConfigurationLoadException: public std::runtime_error
{
    public:
        ConfigurationLoadException(std::wstring msg)
            : runtime_error("Configuration load exception has occured"), message(std::move(msg))
        {
        }

        std::wstring get_message() const { return message; }

    private:
        std::wstring message;
};
