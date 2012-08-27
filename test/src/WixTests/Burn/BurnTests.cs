//-----------------------------------------------------------------------
// <copyright file="BurnTests.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Contains methods to help test Burn.
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Burn
{
    using System;
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    [TestClass]
    public class BurnTests : WixTests
    {
        public static string PayloadCacheFolder = "Package Cache";
        public static string PerMachinePayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\" + PayloadCacheFolder);
        public static string PerUserPayloadCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\" + PayloadCacheFolder);

        /// <summary>
        /// Slows the cache progress of a package.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        /// <param name="delay">Sets or removes the delay on a package being cached.</param>
        protected void SetPackageSlowCache(string packageId, int? delay)
        {
            this.SetPackageState(packageId, "SlowCache", delay.HasValue ? delay.ToString() : null);
        }

        /// <summary>
        /// Cancels the cache of a package at a particular progress point.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        /// <param name="cancelPoint">Sets or removes the cancel progress on a package being cached.</param>
        protected void SetPackageCancelCacheAtProgress(string packageId, int? cancelPoint)
        {
            this.SetPackageState(packageId, "CancelCacheAtProgress", cancelPoint.HasValue ? cancelPoint.ToString() : null);
        }

        /// <summary>
        /// Slows the execute progress of a package.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        /// <param name="delay">Sets or removes the delay on a package being executed.</param>
        protected void SetPackageSlowExecute(string packageId, int? delay)
        {
            this.SetPackageState(packageId, "SlowExecute", delay.HasValue ? delay.ToString() : null);
        }

        /// <summary>
        /// Cancels the execute of a package at a particular progress point.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        /// <param name="cancelPoint">Sets or removes the cancel progress on a package being executed.</param>
        protected void SetPackageCancelExecuteAtProgress(string packageId, int? cancelPoint)
        {
            this.SetPackageState(packageId, "CancelExecuteAtProgress", cancelPoint.HasValue ? cancelPoint.ToString() : null);
        }

        /// <summary>
        /// Sets the requested state for a package that the TestBA will return to the engine during plan.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        /// <param name="state">State to request.</param>
        protected void SetPackageRequestedState(string packageId, RequestState state)
        {
            this.SetPackageState(packageId, "Requested", state.ToString());
        }

        /// <summary>
        /// Sets the requested state for a package that the TestBA will return to the engine during plan.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        /// <param name="state">State to request.</param>
        protected void SetPackageFeatureState(string packageId, string featureId, FeatureState state)
        {
            this.SetPackageState(packageId, String.Concat(featureId, "Requested"), state.ToString());
        }

        /// <summary>
        /// Sets the number of times to re-run the Detect phase.
        /// </summary>
        /// <param name="state">Number of times to run Detect (after the first, normal, Detect).</param>
        protected void SetRedetectCount(int redetectCount)
        {
            this.SetPackageState(null, "RedetectCount", redetectCount.ToString());
        }

        /// <summary>
        /// Resets the state for a package that the TestBA will return to the engine during plan.
        /// </summary>
        /// <param name="packageId">Package identity.</param>
        protected void ResetPackageStates(string packageId)
        {
            string key = String.Format(@"Software\WiX\Tests\TestBAControl\{0}\{1}", this.TestContext.TestName, packageId ?? String.Empty);
            Registry.LocalMachine.DeleteSubKey(key);
        }

        private void SetPackageState(string packageId, string name, string value)
        {
            string key = String.Format(@"Software\WiX\Tests\TestBAControl\{0}\{1}", this.TestContext.TestName, packageId ?? String.Empty);
            using (RegistryKey packageKey = Registry.LocalMachine.CreateSubKey(key))
            {
                if (String.IsNullOrEmpty(value))
                {
                    packageKey.DeleteValue(name, false);
                }
                else
                {
                    packageKey.SetValue(name, value);
                }
            }
        }
    }
}
