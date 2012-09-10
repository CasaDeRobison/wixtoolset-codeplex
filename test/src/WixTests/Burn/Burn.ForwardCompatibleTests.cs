//-----------------------------------------------------------------------
// <copyright file="Burn.FailureTests.cs" company="Microsoft Corporation">
//   Copyright (c) 1999, Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Contains methods test Burn failure scenarios.
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
    public class ForwardCompatibleTests : BurnTests
    {
        const string V2 = "2.0.0.0";

        private PackageBuilder packageA;
        private PackageBuilder packageAv2;
        private PackageBuilder packageB;
        private BundleBuilder bundleA;
        private BundleBuilder bundleAv2;
        private BundleBuilder bundleB;

        [TestMethod]
        [Priority(2)]
        [Description("Installs v2 of a bundle then does a passthrough install and uninstall of v1 with parent.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_ForwardCompatibleInstallV1UninstallV1()
        {
            string providerId = String.Concat("~", this.TestContext.TestName, "_BundleA");
            string parent = "~BundleAv1";
            string parentSwitch = String.Concat("-parent ", parent);

            string packageA = this.GetPackageA().Output;
            string packageAv2 = this.GetPackageAv2().Output;

            string bundleA = this.GetBundleA().Output;
            string bundleAv2 = this.GetBundleAv2().Output;

            // Install the v2 bundle.
            BundleInstaller installerAv2 = new BundleInstaller(this, bundleAv2).Install();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            string actualProviderVersion;
            Assert.IsTrue(this.TryGetDependencyProviderValue(providerId, "Version", out actualProviderVersion));
            Assert.AreEqual(V2, actualProviderVersion);

            // Install the v1 bundle with a parent which should passthrough to v2.
            BundleInstaller installerAv1 = new BundleInstaller(this, bundleA).Install(arguments: parentSwitch);
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            Assert.IsTrue(this.DependencyDependentExists(providerId, parent));

            // Uninstall the v1 bundle with the same parent which should passthrough to v2 and remove parent.
            installerAv1.Uninstall(arguments: parentSwitch);
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            Assert.IsFalse(this.DependencyDependentExists(providerId, parent));

            // Uninstall the v2 bundle and all should be removed.
            installerAv2.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            Assert.IsFalse(this.TryGetDependencyProviderValue(providerId, "Version", out actualProviderVersion));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs v2 of a bundle then does a passthrough install v1 with parent then uninstall of v2.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_ForwardCompatibleInstallV1UninstallV2()
        {
            string providerId = String.Concat("~", this.TestContext.TestName, "_BundleA");
            string parent = "~BundleAv1";
            string parentSwitch = String.Concat("-parent ", parent);

            string packageA = this.GetPackageA().Output;
            string packageAv2 = this.GetPackageAv2().Output;

            string bundleA = this.GetBundleA().Output;
            string bundleAv2 = this.GetBundleAv2().Output;

            // Install the v2 bundle.
            BundleInstaller installerAv2 = new BundleInstaller(this, bundleAv2).Install();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            string actualProviderVersion;
            Assert.IsTrue(this.TryGetDependencyProviderValue(providerId, "Version", out actualProviderVersion));
            Assert.AreEqual(V2, actualProviderVersion);

            // Install the v1 bundle with a parent which should passthrough to v2.
            BundleInstaller installerAv1 = new BundleInstaller(this, bundleA).Install(arguments: parentSwitch);
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            Assert.IsTrue(this.DependencyDependentExists(providerId, parent));

            // Uninstall the v2 bundle.
            installerAv2.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageAv2));

            Assert.IsTrue(this.DependencyDependentExists(providerId, parent));

            // Uninstall the v1 bundle with passthrough and all should be removed.
            installerAv1.Uninstall(arguments: parentSwitch);
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageAv2));

            Assert.IsFalse(this.TryGetDependencyProviderValue(providerId, "Version", out actualProviderVersion));


            this.CleanTestArtifacts = true;
        }

        private PackageBuilder GetPackageA()
        {
            if (null == this.packageA)
            {
                this.packageA = new PackageBuilder(this, "A") { Extensions = Extensions }.Build();
            }

            return this.packageA;
        }

        private PackageBuilder GetPackageAv2()
        {
            if (null == this.packageAv2)
            {
                this.packageAv2 = new PackageBuilder(this, "A") { Extensions = Extensions, PreprocessorVariables = new Dictionary<string, string>() { { "Version", V2 } } }.Build();
            }

            return this.packageAv2;
        }

        private BundleBuilder GetBundleA(Dictionary<string, string> bindPaths = null)
        {
            if (null == bindPaths)
            {
                string packageAPath = this.GetPackageA().Output;
                bindPaths = new Dictionary<string, string>() { { "packageA", packageAPath } };
            }

            if (null == this.bundleA)
            {
                this.bundleA = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions }.Build();
            }

            return this.bundleA;
        }

        private BundleBuilder GetBundleAv2(Dictionary<string, string> bindPaths = null)
        {
            if (null == bindPaths)
            {
                string packageAPath = this.GetPackageAv2().Output;
                bindPaths = new Dictionary<string, string>() { { "packageA", packageAPath } };
            }

            if (null == this.bundleAv2)
            {
                this.bundleAv2 = new BundleBuilder(this, "BundleA") { BindPaths = bindPaths, Extensions = Extensions, PreprocessorVariables = new Dictionary<string, string>() { { "Version", V2 } } }.Build();
            }

            return this.bundleAv2;
        }

        private PackageBuilder GetPackageB()
        {
            if (null == this.packageB)
            {
                this.packageB = new PackageBuilder(this, "B") { Extensions = Extensions }.Build();
            }

            return this.packageB;
        }

        private BundleBuilder GetBundleB(Dictionary<string, string> bindPaths = null)
        {
            if (null == bindPaths)
            {
                string packageBPath = this.GetPackageB().Output;
                bindPaths = new Dictionary<string, string>() { { "packageB", packageBPath } };
            }

            if (null == this.bundleB)
            {
                this.bundleB = new BundleBuilder(this, "BundleB") { BindPaths = bindPaths, Extensions = Extensions }.Build();
            }

            return this.bundleB;
        }
    }
}
