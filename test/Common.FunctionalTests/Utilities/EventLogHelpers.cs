// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text.RegularExpressions;
using Microsoft.AspNetCore.Server.IntegrationTesting;
using Microsoft.AspNetCore.Server.IntegrationTesting.IIS;
using Xunit;

namespace Microsoft.AspNetCore.Server.IISIntegration.FunctionalTests
{
    public class EventLogHelpers
    {
        public static void VerifyEventLogEvent(IISDeploymentResult deploymentResult, string expectedRegexMatchString)
        {
            Assert.True(deploymentResult.HostProcess.HasExited);

            var entries = GetEntries(deploymentResult);
            AssertSingleEntry(expectedRegexMatchString, entries);
        }
        
        public static void VerifyEventLogEvents(IISDeploymentResult deploymentResult, params string[] expectedRegexMatchString)
        {
            Assert.True(deploymentResult.HostProcess.HasExited);

            var entries = GetEntries(deploymentResult).ToList();
            foreach (var regexString in expectedRegexMatchString)
            {
                var matchedEntries = AssertSingleEntry(regexString, entries);

                foreach (var matchedEntry in matchedEntries)
                {
                    entries.Remove(matchedEntry);
                }
            }
            
            Assert.True(0 == entries.Count, $"Some entries were not matched by any regex {FormatEntries(entries)}");
        }

        private static EventLogEntry[] AssertSingleEntry(string regexString, IEnumerable<EventLogEntry> entries)
        {
            var expectedRegex = new Regex(regexString, RegexOptions.Singleline);
            var matchedEntries = entries.Where(entry => expectedRegex.IsMatch(entry.Message)).ToArray();
            Assert.True(matchedEntries.Length > 0, $"No entries matched by '{regexString}'");
            Assert.True(matchedEntries.Length < 2, $"Multiple entries matched by '{regexString}': {FormatEntries(matchedEntries)}");
            return matchedEntries;
        }

        private static string FormatEntries(IEnumerable<EventLogEntry> entries)
        {
            return string.Join(",", entries.Select(e => e.Message));
        }

        private static IEnumerable<EventLogEntry> GetEntries(IISDeploymentResult deploymentResult)
        {
            var eventLog = new EventLog("Application");

            // Eventlog is already sorted based on time of event in ascending time.
            // Check results in reverse order.
            var processIdString = $"Process Id: {deploymentResult.HostProcess.Id}.";

            // Event log messages round down to the nearest second, so subtract a second
            var processStartTime = deploymentResult.HostProcess.StartTime.AddSeconds(-1);
            for (var i = eventLog.Entries.Count - 1; i >= 0; i--)
            {
                var eventLogEntry = eventLog.Entries[i];
                if (eventLogEntry.TimeGenerated < processStartTime)
                {
                    // If event logs is older than the process start time, we didn't find a match.
                    break;
                }

                if (eventLogEntry.ReplacementStrings == null ||
                    eventLogEntry.ReplacementStrings.Length < 3)
                {
                    continue;
                }
                
                // ReplacementStings == EventData collection in EventLog
                // This is unaffected if event providers are not registered correctly
                if (eventLogEntry.Source == AncmVersionToMatch(deploymentResult) &&
                    processIdString == eventLogEntry.ReplacementStrings[1])
                {
                    yield return eventLogEntry;
                }
            }
        }

        private static string AncmVersionToMatch(IISDeploymentResult deploymentResult)
        {
            return "IIS " +
                (deploymentResult.DeploymentParameters.ServerType == ServerType.IISExpress ? "Express " : "") +
                "AspNetCore Module" +
                (deploymentResult.DeploymentParameters.AncmVersion == AncmVersion.AspNetCoreModuleV2 ? " V2" : "");
        }

        
        public static string InProcessStarted(IISDeploymentResult deploymentResult)
        {
            return $"Application '{EscapedContentRoot(deploymentResult)}' started the coreclr in-process successfully";
        }

        public static string InProcessFailedToStart(IISDeploymentResult deploymentResult, string reason)
        {
            return $"Application '/LM/W3SVC/1/ROOT' with physical root '{EscapedContentRoot(deploymentResult)}' failed to load clr and managed application. {reason}";
        }

        public static string InProcessFailedToStop(IISDeploymentResult deploymentResult, string reason)
        {
            return "Failed to gracefully shutdown application 'MACHINE/WEBROOT/APPHOST/HTTPTESTSITE'.";
        }

        public static string InProcessThreadException(IISDeploymentResult deploymentResult, string reason)
        {
            return $"Application '/LM/W3SVC/1/ROOT' with physical root '{EscapedContentRoot(deploymentResult)}' hit unexpected managed exception{reason}";
        }

        public static string InProcessThreadExit(IISDeploymentResult deploymentResult, string code)
        {
            return $"Application '/LM/W3SVC/1/ROOT' with physical root '{EscapedContentRoot(deploymentResult)}' hit unexpected managed background thread exit, exit code = '{code}'.";
        }
        
        public static string FailedToStartApplication(IISDeploymentResult deploymentResult, string code)
        {
            return $"Failed to start application '/LM/W3SVC/1/ROOT', ErrorCode '{code}'.";
        }

        public static string ConfigurationLoadError(IISDeploymentResult deploymentResult, string reason)
        {
            return $"Configuration load error. {reason}";
        }

        private static string EscapedContentRoot(IISDeploymentResult deploymentResult)
        {
            var contentRoot = deploymentResult.ContentRoot;
            if (!contentRoot.EndsWith('\\'))
            {
                contentRoot += '\\';
            }
            return Regex.Escape(contentRoot);
        }
    }
}
