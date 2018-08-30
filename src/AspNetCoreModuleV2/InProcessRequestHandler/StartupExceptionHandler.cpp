// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "StartupExceptionApplication.h"
#include "StartupExceptionHandler.h"

std::string StartupExceptionHandler::s_html500Page = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"> \
            <html xmlns=\"http://www.w3.org/1999/xhtml\"> \
            <head> \
            <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" /> \
            <title> IIS 500.30 Error </title><style type=\"text/css\"></style></head> \
            <body> <div id = \"content\"> \
              <div class = \"content-container\"><h3> HTTP Error 500.30 - ANCM In-Process Start Failure </h3></div>  \
              <div class = \"content-container\"> \
               <fieldset> <h4> Common causes of this issue: </h4> \
                <ul><li> The application failed to start </li> \
                 <li> The application started but then stopped </li> \
                 <li> The application started but threw an exception during startup </li></ul></fieldset> \
              </div> \
              <div class = \"content-container\"> \
                <fieldset><h4> Troubleshooting steps: </h4> \
                 <ul><li> Check the system event log for error messages </li> \
                 <li> Enable logging the application process' stdout messages </li> \
                 <li> Attach a debugger to the application process and inspect </li></ul></fieldset> \
                 <fieldset><h4> For more information visit: \
                 <a href=\"https://go.microsoft.com/fwlink/?LinkID=808681\"> <cite> https://go.microsoft.com/fwlink/?LinkID=808681 </cite></a></h4> \
                 </fieldset> \
              </div> \
           </div></body></html>";

REQUEST_NOTIFICATION_STATUS StartupExceptionHandler::OnExecuteRequestHandler()
{
    if (!m_disableLogs)
    {
        HTTP_DATA_CHUNK   DataChunk;
        IHttpResponse* pResponse = m_pContext->GetResponse();
        pResponse->SetStatus(500, "Internal Server Error", 30, E_FAIL, NULL, TRUE);
        pResponse->SetHeader("Content-Type",
            "text/html",
            (USHORT)strlen("text/html"),
            FALSE
        );
        DataChunk.DataChunkType = HttpDataChunkFromMemory;
        DataChunk.FromMemory.pBuffer = (PVOID)s_html500Page.c_str();
        DataChunk.FromMemory.BufferLength = (ULONG)s_html500Page.size();
        pResponse->WriteEntityChunkByReference(&DataChunk);
    }
    else
    {
        m_pContext->GetResponse()->SetStatus(500, "Internal Server Error", 30, E_FAIL);
    }

    return REQUEST_NOTIFICATION_STATUS::RQ_NOTIFICATION_FINISH_REQUEST;
}

