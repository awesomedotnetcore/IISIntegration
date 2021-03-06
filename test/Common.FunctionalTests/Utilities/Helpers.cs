// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Threading.Tasks;
using System.Xml.Linq;
using Microsoft.AspNetCore.Server.IntegrationTesting;
using Microsoft.AspNetCore.Server.IntegrationTesting.IIS;
using Microsoft.AspNetCore.Testing;
using Microsoft.Extensions.Logging;
using Xunit;

namespace Microsoft.AspNetCore.Server.IISIntegration.FunctionalTests
{
    public static class Helpers
    {
        private static readonly TimeSpan RetryRequestDelay = TimeSpan.FromMilliseconds(100);
        private static readonly int RetryRequestCount = 5;

        public static string GetTestWebSitePath(string name)
        {
            return Path.Combine(TestPathUtilities.GetSolutionRootDirectory("IISIntegration"),"test", "WebSites", name);
        }

        public static string GetInProcessTestSitesPath() => GetTestWebSitePath("InProcessWebSite");

        public static string GetOutOfProcessTestSitesPath() => GetTestWebSitePath("OutOfProcessWebSite");

        public static async Task AssertStarts(this IISDeploymentResult deploymentResult, string path = "/HelloWorld")
        {
            var response = await deploymentResult.HttpClient.GetAsync(path);

            var responseText = await response.Content.ReadAsStringAsync();

            Assert.Equal("Hello World", responseText);
        }

        public static async Task StressLoad(HttpClient httpClient, string path, Action<HttpResponseMessage> action)
        {
            async Task RunRequests()
            {
                var connection = new HttpClient() { BaseAddress = httpClient.BaseAddress };

                for (int j = 0; j < 10; j++)
                {
                    var response = await connection.GetAsync(path);
                    action(response);
                }
            }

            List<Task> tasks = new List<Task>();
            for (int i = 0; i < 10; i++)
            {
                tasks.Add(Task.Run(RunRequests));
            }

            await Task.WhenAll(tasks);
        }

        public static void CopyFiles(DirectoryInfo source, DirectoryInfo target, ILogger logger)
        {
            foreach (DirectoryInfo directoryInfo in source.GetDirectories())
            {
                if (directoryInfo.FullName != target.FullName)
                {
                    CopyFiles(directoryInfo, target.CreateSubdirectory(directoryInfo.Name), logger);
                }
            }
            logger?.LogDebug($"Processing {target.FullName}");
            foreach (FileInfo fileInfo in source.GetFiles())
            {
                logger?.LogDebug($"  Copying {fileInfo.Name}");
                var destFileName = Path.Combine(target.FullName, fileInfo.Name);
                fileInfo.CopyTo(destFileName);
            }
        }

        public static void ModifyWebConfig(this DeploymentResult deploymentResult, Action<XElement> action)
        {
            var webConfigPath = Path.Combine(deploymentResult.ContentRoot, "web.config");
            var document = XDocument.Load(webConfigPath);
            action(document.Root);
            document.Save(webConfigPath);
        }

        public static Task<HttpResponseMessage> RetryRequestAsync(this HttpClient client, string uri, Func<HttpResponseMessage, bool> predicate)
        {
            return RetryRequestAsync(client, uri, message => Task.FromResult(predicate(message)));
        }

        public static async Task<HttpResponseMessage> RetryRequestAsync(this HttpClient client, string uri, Func<HttpResponseMessage, Task<bool>> predicate)
        {
            HttpResponseMessage response = await client.GetAsync(uri);

            for (var i = 0; i < RetryRequestCount && !await predicate(response); i++)
            {
                // Keep retrying until app_offline is present.
                response = await client.GetAsync(uri);
                await Task.Delay(RetryRequestDelay);
            }

            if (!await predicate(response))
            {
                throw new InvalidOperationException($"Didn't get response that satisfies predicate after {RetryRequestCount} retries");
            }

            return response;
        }

        public static void AssertWorkerProcessStop(this IISDeploymentResult deploymentResult)
        {
            var hostProcess = deploymentResult.HostProcess;
            Assert.True(hostProcess.WaitForExit((int)TimeoutExtensions.DefaultTimeout.TotalMilliseconds));

            if (deploymentResult.DeploymentParameters.ServerType == ServerType.IISExpress)
            {
                Assert.Equal(0, hostProcess.ExitCode);
            }
        }


        public static async Task AssertRecycledAsync(this IISDeploymentResult deploymentResult, Func<Task> verificationAction = null)
        {
            if (deploymentResult.DeploymentParameters.HostingModel != HostingModel.InProcess)
            {
                throw new NotSupportedException();
            }

            deploymentResult.AssertWorkerProcessStop();
            if (deploymentResult.DeploymentParameters.ServerType == ServerType.IIS)
            {
                verificationAction = verificationAction ?? (() => deploymentResult.AssertStarts());
                await verificationAction();
            }
        }

        public static IEnumerable<object[]> ToTheoryData<T>(this Dictionary<string, T> dictionary)
        {
            return dictionary.Keys.Select(k => new[] { k });
        }

        public static string GetExpectedLogName(IISDeploymentResult deploymentResult, string logFolderPath)
        {
            var startTime = deploymentResult.HostProcess.StartTime.ToUniversalTime();
            return Path.Combine(logFolderPath, $"std_{startTime.Year}{startTime.Month:D2}" +
                $"{startTime.Day:D2}{startTime.Hour:D2}" +
                $"{startTime.Minute:D2}{startTime.Second:D2}_" +
                $"{deploymentResult.HostProcess.Id}.log");
        }
    }
}
