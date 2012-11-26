//-----------------------------------------------------------------------
// <copyright file="Burn.BasicTests.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Contains methods test Burn.
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Burn
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using WixToolset.Dtf.WindowsInstaller;
    using WixTest.Utilities;
    using WixTest.Verifiers;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    [TestClass]
    public class BasicTests : BurnTests
    {
        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then bundle B then removes in same order.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_InstallUninstall()
        {
            // Build the packages.
            string packageA = new PackageBuilder(this, "A") { Extensions = Extensions }.Build().Output;
            string packageB = new PackageBuilder(this, "B") { Extensions = Extensions }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);
            bindPaths.Add("packageB", packageB);

            // Build the bundles.
            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;
            string bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;

            // Install the bundles.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install();
            BundleInstaller installerB = new BundleInstaller(this, bundleB).Install();

            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageB));

            // Attempt to uninstall bundleA.
            installerA.Uninstall();

            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageB));

            // Uninstall bundleB now.
            installerB.Uninstall();

            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageB));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then upgrades it to v2.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_MajorUpgrade()
        {
            string v2Version = "2.0.0.0";

            // Build the packages.
            string packageAv1 = new PackageBuilder(this, "A").Build().Output;
            string packageAv2 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", v2Version } } }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPathsv1 = new Dictionary<string, string>() { { "packageA", packageAv1 } };
            Dictionary<string, string> bindPathsv2 = new Dictionary<string, string>() { { "packageA", packageAv2 } };

            // Build the bundles.
            string bundleAv1 = new BundleBuilder(this, "BundleA") { BindPaths = bindPathsv1, Extensions = Extensions }.Build().Output;
            string bundleAv2 = new BundleBuilder(this, "BundleA") { BindPaths = bindPathsv2, Extensions = Extensions, PreprocessorVariables = new Dictionary<string, string>() { { "Version", v2Version } } }.Build().Output;

            // Initialize with first bundle.
            BundleInstaller installerAv1 = new BundleInstaller(this, bundleAv1).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv1));

            // Install second bundle which will major upgrade away v1.
            BundleInstaller installerAv2 = new BundleInstaller(this, bundleAv2).Install();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            // Uninstall the second bundle and everything should be gone.
            installerAv2.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then upgrades it to v2 using 'modify' as the action.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_MajorUpgradeUsingModify()
        {
            string v2Version = "2.0.0.0";

            // Build the packages.
            string packageAv1 = new PackageBuilder(this, "A").Build().Output;
            //string packageAv2 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", v2Version } } }.Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPathsv1 = new Dictionary<string, string>() { { "packageA", packageAv1 } };
            Dictionary<string, string> bindPathsv2 = new Dictionary<string, string>() { { "packageA", packageAv1 } };

            // Build the bundles.
            string bundleAv1 = new BundleBuilder(this, "BundleA") { BindPaths = bindPathsv1, Extensions = Extensions }.Build().Output;
            string bundleAv2 = new BundleBuilder(this, "BundleA") { BindPaths = bindPathsv2, Extensions = Extensions, PreprocessorVariables = new Dictionary<string, string>() { { "Version", v2Version } } }.Build().Output;

            // Initialize with first bundle.
            BundleInstaller installerAv1 = new BundleInstaller(this, bundleAv1).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv1));

            // Install second bundle which will major upgrade away v1.
            BundleInstaller installerAv2 = new BundleInstaller(this, bundleAv2).Modify();
            //Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv1));

            // Uninstall the second bundle and everything should be gone.
            installerAv2.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            //Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then upgrades it with same version bundle A.")]
        [TestProperty("IsRuntimeTest", "true")]
        [Ignore]
        public void Burn_MajorUpgradeSameVersion()
        {
            // Build the packages.
            string packageA1 = new PackageBuilder(this, "A").Build().Output;
            string packageA2 = new PackageBuilder(this, "A").Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths1 = new Dictionary<string, string>() { { "packageA", packageA1 } };
            Dictionary<string, string> bindPaths2 = new Dictionary<string, string>() { { "packageA", packageA2 } };

            // Build the bundles.
            string bundleA1 = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths1, Extensions = Extensions }.Build().Output;
            string bundleA2 = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths2, Extensions = Extensions }.Build().Output;

            // Initialize with first bundle.
            BundleInstaller installerA1 = new BundleInstaller(this, bundleA1).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA2));

            // Install second bundle which will major upgrade away A1 (since they have the same version).
            BundleInstaller installerA2 = new BundleInstaller(this, bundleA2).Install();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA2));

            // Uninstall the second bundle and everything should be gone.
            installerA2.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA2));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then bundle B with a minor upgrade of A.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_SharedMinorUpgrade()
        {
            string productCode = Guid.NewGuid().ToString("B").ToUpperInvariant();
            string originalVersion = "1.0.0.0";
            string v11Version = "1.0.1.0";

            // Build the packages.
            string packageAv1 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "ProductCode", productCode } } }.Build().Output;
            string packageAv11 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "ProductCode", productCode }, { "Version", v11Version } } }.Build().Output;
            string packageB = new PackageBuilder(this, "B").Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPathsA = new Dictionary<string, string>() { { "packageA", packageAv1 } };
            Dictionary<string, string> bindPathsB = new Dictionary<string, string>() { { "packageA", packageAv11 }, { "packageB", packageB } };

            // Build the bundles.
            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPathsA, Extensions = Extensions }.Build().Output;
            string bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPathsB, Extensions = Extensions }.Build().Output;

            // Initialize with first bundle.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install();
            Assert.IsTrue(MsiVerifier.IsProductInstalled(productCode));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(originalVersion, actualVersion);
            }

            // Install second bundle which will minor upgrade .
            BundleInstaller installerB = new BundleInstaller(this, bundleB).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageB));
            Assert.IsTrue(MsiVerifier.IsProductInstalled(productCode));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(v11Version, actualVersion);
            }

            // Uninstall the second bundle and only the minor upgrade MSI should be left.
            installerB.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageB));
            Assert.IsTrue(MsiVerifier.IsProductInstalled(productCode));
            using (RegistryKey root = this.GetTestRegistryRoot())
            {
                string actualVersion = root.GetValue("A") as string;
                Assert.AreEqual(v11Version, actualVersion);
            }

            // Now everything should be gone.
            installerA.Uninstall();
            Assert.IsFalse(MsiVerifier.IsProductInstalled(productCode));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then removes it.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_MajorUpgradeRemovesPackageFixedByRepair()
        {
            string v2Version = "2.0.0.0";

            // Build the packages.
            string packageAv1 = new PackageBuilder(this, "A").Build().Output;
            string packageAv2 = new PackageBuilder(this, "A") { PreprocessorVariables = new Dictionary<string, string>() { { "Version", v2Version } } }.Build().Output;
            string packageB = new PackageBuilder(this, "B").Build().Output;

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPathsv1 = new Dictionary<string, string>() { { "packageA", packageAv1 } };
            Dictionary<string, string> bindPathsv2 = new Dictionary<string, string>() { { "packageA", packageAv2 }, { "packageB", packageB } };

            // Build the bundles.
            string bundleAv1 = new BundleBuilder(this, "BundleA") { BindPaths = bindPathsv1, Extensions = Extensions }.Build().Output;
            string bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPathsv2, Extensions = Extensions }.Build().Output;

            // Initialize with first bundle.
            BundleInstaller installerA = new BundleInstaller(this, bundleAv1).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv1));

            // Install second bundle which will major upgrade away v1.
            BundleInstaller installerB = new BundleInstaller(this, bundleB).Install();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            // Uninstall second bundle which will remove all packages
            installerB.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            // Repair first bundle to get v1 back on the machine.
            installerA.Repair();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            // Uninstall first bundle and everything should be gone.
            installerA.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv1));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs a bundle and tests the register source list.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_ValidateMultipleSourcePaths()
        {
            // Build the package.
            string packageA = new PackageBuilder(this, "A") { Extensions = Extensions }.Build().Output;
            string packageA_Directory = Path.GetDirectoryName(packageA);
            string packageA_ProductCode = MsiUtils.GetMSIProductCode(packageA);

            // Create the named bind paths to the packages.
            Dictionary<string, string> bindPaths = new Dictionary<string, string>();
            bindPaths.Add("packageA", packageA);

            // Build the bundle.
            string bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build().Output;

            // Install the bundle.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));

            // Copy the package using the bundle package name.
            ProductInstallation product = new ProductInstallation(packageA_ProductCode, null, UserContexts.Machine);
            string packageA_Copy = Path.Combine(packageA_Directory, product.AdvertisedPackageName);
            File.Copy(packageA, packageA_Copy);
            this.TestArtifacts.Add(new FileInfo(packageA_Copy));

            // Repair and recache the MSI.
            MSIExec.InstallProduct(packageA_Copy, MSIExec.MSIExecReturnCode.SUCCESS, "REINSTALL=ALL REINSTALLMODE=vomus");
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));

            // Check that the source contains both the original and burn cached paths.
            SourceList sources = product.SourceList;
            Assert.AreEqual<int>(2, sources.Count);

            // Attempt to uninstall bundleA.
            installerA.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));

            this.CleanTestArtifacts = true;
        }
    }
}
