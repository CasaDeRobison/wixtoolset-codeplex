//-------------------------------------------------------------------------------------------------
// <copyright file="Program.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace ExePkg
{
    using System;
    using Microsoft.Win32;

    class Program
    {
        static readonly string RegRootMachine = "HKEY_LOCAL_MACHINE";
        static readonly string RegRootUser = "HKEY_CURRENT_USER";
        static readonly string RegKey = "SOFTWARE\\Microsoft\\WiX_Burn\\ExePackages";

        static int Main(string[] args)
        {
            string root = RegRootMachine;
            int returnCode = 0;
            int wait = 0;

            if (0 == args.Length)
            {
                Console.WriteLine("usage: ExePkg.exe [-wait milliseconds] [-returnCode #] [-peruser|-permachine] {-install|-uninstall|-repair}");
                return 1;
            }

            for (int i = 0; i < args.Length; ++i)
            {
                string arg = args[i];
                Console.Write(arg);

                switch (arg.ToLower())
                {
                    case "-permachine":
                        root = RegRootMachine;
                        break;

                    case "-peruser":
                        root = RegRootUser;
                        break;

                    case "-install":
                        Registry.SetValue(root + "\\" + RegKey, "Exe1", "true", RegistryValueKind.String);
                        Console.WriteLine(": complete");
                        break;

                    case "-uninstall":
                        if (root == RegRootMachine)
                        {
                            Registry.LocalMachine.DeleteSubKey(RegKey, false);
                        }
                        else
                        {
                            Registry.CurrentUser.DeleteSubKey(RegKey, false);
                        }
                        Console.WriteLine(": complete");
                        break;

                    case "-repair":
                        Registry.SetValue(root + "\\" + RegKey, "Exe1", "true", RegistryValueKind.String);
                        Console.WriteLine(": complete");
                        break;

                    case "-returnCode":
                        returnCode = int.Parse(args[++i]);
                        break;

                    case "-wait":
                        wait = int.Parse(args[++i]);
                        break;

                    default:
                        Console.WriteLine(": invalid argument");
                        return 2;
                }
            }

            if (0 < wait)
            {
                System.Threading.Thread.Sleep(wait);
            }

            return returnCode;
        }
    }
}
