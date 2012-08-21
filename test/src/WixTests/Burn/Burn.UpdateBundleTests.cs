//-----------------------------------------------------------------------
// <copyright file="Burn.UpdateBundleTests.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Contains methods to test update bundles in Burn.
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Burn
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using Microsoft.Deployment.WindowsInstaller;
    using WixTest.Utilities;
    using WixTest.Verifiers;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    [TestClass]
    public class UpdateBundleTests : BurnTests
    {
        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle Av1.0 that is updated bundle Av2.0.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_InstallUpdatedBundle()
        {
            const string updatedVersion = "2.0.0.0";

            // Build the packages.
            string packageA1 = new PackageBuilder(this, "A").Build().Output;
            string packageA2 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", updatedVersion } } }.Build().Output;

            // Build the bundles.
            string bundleA1 = new BundleBuilder(this, "BundleA") { BindPaths = new Dictionary<string, string>() { { "packageA", packageA1 } }, Extensions = Extensions }.Build().Output;
            string bundleA2 = new BundleBuilder(this, "BundleA") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", updatedVersion } }, BindPaths = new Dictionary<string, string>() { { "packageA", packageA2 } }, Extensions = Extensions }.Build().Output;

            // Install the bundles.
            BundleInstaller installerA1 = new BundleInstaller(this, bundleA1).Install(arguments: String.Concat("\"", "-updatebundle:", bundleA2, "\""));
            BundleInstaller installerA2 = new BundleInstaller(this, bundleA2);

            // Test that only the newest packages is installed.
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA2));

            // Attempt to uninstall bundleA2.
            installerA2.Uninstall();

            // Test all packages are uninstalled.
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA2));
            Assert.IsNull(this.GetTestRegistryRoot());

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle Av1.0 then does an update to bundle Av2.0 during modify.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_UpdateInstalledBundle()
        {
            const string updatedVersion = "2.0.0.0";

            // Build the packages.
            string packageA1 = new PackageBuilder(this, "A").Build().Output;
            string packageA2 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", updatedVersion } } }.Build().Output;

            // Build the bundles.
            string bundleA1 = new BundleBuilder(this, "BundleA") { BindPaths = new Dictionary<string, string>() { { "packageA", packageA1 } }, Extensions = Extensions }.Build().Output;
            string bundleA2 = new BundleBuilder(this, "BundleA") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", updatedVersion } }, BindPaths = new Dictionary<string, string>() { { "packageA", packageA2 } }, Extensions = Extensions }.Build().Output;

            // Install the v1 bundle.
            BundleInstaller installerA1 = new BundleInstaller(this, bundleA1).Install();

            // Test that v1 was correctly installed.
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA2));

            // Run the v1 bundle providing an update bundle.
            installerA1.Modify(arguments: String.Concat("\"", "-updatebundle:", bundleA2, "\""));

            // Test that only v2 packages is installed.
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA2));

            // Attempt to uninstall v2.
            BundleInstaller installerA2 = new BundleInstaller(this, bundleA2).Uninstall();

            // Test all packages are uninstalled.
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA2));
            Assert.IsNull(this.GetTestRegistryRoot());

            this.CleanTestArtifacts = true;
        }
    }
}
