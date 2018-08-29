// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.IO;
using System.Security;
using System.Text;

namespace IISTestSite
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            var envVariable = Environment.GetEnvironmentVariable("ASPNETCORE_INPROCESS_STARTUP_VALUE");
            var randomNumber = Environment.GetEnvironmentVariable("ASPNETCORE_INPROCESS_RANDOM_VALUE");

            // Semicolons are appended to env variables; removing them.
            if (envVariable == "CheckLargeStdOutWrites")
            {
                Console.WriteLine(new string('a', 4096));
            }
            else if (envVariable == "CheckLargeStdErrWrites")
            {
                Console.Error.WriteLine(new string('a', 4096));
                Console.Error.Flush();
            }
            else if (envVariable == "CheckLogFile")
            {
                Console.OutputEncoding = Console.OutputEncoding;

                Console.WriteLine($"Random number: {randomNumber}");
            }
            else if (envVariable == "CheckErrLogFile")
            {
                Console.OutputEncoding = Console.OutputEncoding;

                Console.Error.WriteLine($"Random number: {randomNumber}");
                Console.Error.Flush();
            }
            else if (envVariable == "CheckOversizedStdErrWrites")
            {
                Console.WriteLine(new string('a', 5000));

            }
            else if (envVariable == "CheckOversizedStdOutWrites")
            {
                Console.Error.WriteLine(new string('a', 4096));
                Console.Error.Flush();
            }
            else if (envVariable == "CheckUTF8")
            {
                try
                {
                    Console.OutputEncoding = Encoding.UTF8;
                }
                catch (IOException ioe)
                {
                    Console.WriteLine(ioe.Message);
                }
                catch (SecurityException securityException)
                {
                    Console.WriteLine(securityException.Message);
                }
                Console.WriteLine("彡⾔");
            }
        }
    }
}
