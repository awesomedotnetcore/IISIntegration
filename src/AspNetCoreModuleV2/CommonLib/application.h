// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <string>
#include "iapplication.h"
#include "ntassert.h"
#include "SRWExclusiveLock.h"

class APPLICATION : public IAPPLICATION
{
public:
    // Non-copyable
    APPLICATION(const APPLICATION&) = delete;
    const APPLICATION& operator=(const APPLICATION&) = delete;

    APPLICATION_STATUS
    QueryStatus() override
    {
        return m_fStopCalled ? APPLICATION_STATUS::RECYCLED : APPLICATION_STATUS::RUNNING;
    }

    APPLICATION(const IHttpApplication& pHttpApplication)
        : m_fStopCalled(false),
          m_cRefs(1),
          m_applicationPhysicalPath(pHttpApplication.GetApplicationPhysicalPath()),
          m_applicationConfigPath(pHttpApplication.GetAppConfigPath()),
          m_applicationId(pHttpApplication.GetApplicationId())
    {
        InitializeSRWLock(&m_stateLock);
        m_applicationVirtualPath = ToVirtualPath(m_applicationConfigPath);
    }


    VOID
    Stop(bool fServerInitiated) override
    {
        SRWExclusiveLock stopLock(m_stateLock);

        if (m_fStopCalled)
        {
            return;
        }

        m_fStopCalled = true;

        StopInternal(fServerInitiated);
    }

    virtual
    VOID
    StopInternal(bool fServerInitiated)
    {
        UNREFERENCED_PARAMETER(fServerInitiated);
    }

    VOID
    ReferenceApplication() override
    {
        DBG_ASSERT(m_cRefs > 0);

        InterlockedIncrement(&m_cRefs);
    }

    VOID
    DereferenceApplication() override
    {
        DBG_ASSERT(m_cRefs > 0);

        if (InterlockedDecrement(&m_cRefs) == 0)
        {
            delete this;
        }
    }

    const std::wstring&
    QueryApplicationId() const
    {
        return m_applicationId;
    }

    const std::wstring&
    QueryApplicationPhysicalPath() const
    {
        return m_applicationPhysicalPath;
    }

    const std::wstring&
    QueryApplicationVirtualPath() const
    {
        return m_applicationVirtualPath;
    }

    const std::wstring&
    QueryConfigPath() const
    {
        return m_applicationConfigPath;
    }

protected:
    SRWLOCK m_stateLock {};
    bool m_fStopCalled;

private:
    mutable LONG           m_cRefs;

    std::wstring m_applicationPhysicalPath;
    std::wstring m_applicationVirtualPath;
    std::wstring m_applicationConfigPath;
    std::wstring m_applicationId;

    static std::wstring ToVirtualPath(const std::wstring& configurationPath)
    {
        auto segments = 0;
        auto position = configurationPath.find('/');
        // Skip first 4 segments of config path
        while (segments != 3 && position != std::wstring::npos)
        {
            segments++;
            position = configurationPath.find('/', position + 1);
        }

        if (position != std::wstring::npos)
        {
            return configurationPath.substr(position);
        }

        return L"/";
    }
};
