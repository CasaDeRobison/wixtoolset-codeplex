//-----------------------------------------------------------------------
// <copyright file="Burn.BasicTests.cs" company="Microsoft Corporation">
//   Copyright (c) 1999, Microsoft Corporation.  All rights reserved.
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
    using Microsoft.Deployment.WindowsInstaller;
    using WixTest.Utilities;
    using WixTest.Verifiers;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Win32;

    [TestClass]
    public class ParentTests : BurnTests
    {
        private PackageBuilder packageA;
        private PackageBuilder packageD;
        private BundleBuilder bundleA;
        private BundleBuilder bundleD;

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A with a parent then uninstalls without parent then uninstalls with parent.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_ParentInstallUninstallParentUninstall()
        {
            // Build.
            string packageA = this.GetPackageA().Output;
            string bundleA = this.GetBundleA().Output;

            // Install the bundle with a parent, and ensure it is installed.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install(arguments: "-parent Foo");
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));

            // Attempt to uninstall bundle without a parent and ensure it is still installed.
            installerA.Uninstall();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));

            // Uninstall bundle with the parent and ensure it is removed.
            installerA.Uninstall(arguments: "-parent Foo");
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));

            this.CleanTestArtifacts = true;
        }

        [TestMethod]
        [Priority(2)]
        [Description("Installs bundle A then installs bundle D with a parent as addon then uninstalls bundle A which leaves bundle D.")]
        [TestProperty("IsRuntimeTest", "true")]
        public void Burn_ParentInstallAddonUninstall()
        {
            // Build.
            string packageA = this.GetPackageA().Output;
            string bundleA = this.GetBundleA().Output;
            string packageD = this.GetPackageD().Output;
            string bundleD = this.GetBundleD().Output;

            // Install the base bundle, and ensure it is installed.
            BundleInstaller installerA = new BundleInstaller(this, bundleA).Install();
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageA));

            // Install the addon bundle, and ensure it is installed.
            BundleInstaller installerD = new BundleInstaller(this, bundleD).Install(arguments: "-parent Foo");
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageD));

            // Uninstall the base bundle and ensure it is removed but addon is still present.
            installerA.Uninstall();
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsTrue(MsiVerifier.IsPackageInstalled(packageD));

            // Uninstall addon bundle with the parent and ensure everything is removed.
            installerD.Uninstall(arguments: "-parent Foo");
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageA));
            Assert.IsFalse(MsiVerifier.IsPackageInstalled(packageD));

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

        private PackageBuilder GetPackageD()
        {
            if (null == this.packageD)
            {
                this.packageD = new PackageBuilder(this, "D") { Extensions = Extensions }.Build();
            }

            return this.packageD;
        }

        private BundleBuilder GetBundleD(Dictionary<string, string> bindPaths = null)
        {
            if (null == bindPaths)
            {
                string packageDPath = this.GetPackageD().Output;
                bindPaths = new Dictionary<string, string>() { { "packageD", packageDPath } };
            }

            if (null == this.bundleD)
            {
                this.bundleD = new BundleBuilder(this, "BundleD") { BindPaths = bindPaths, Extensions = Extensions }.Build();
            }

            return this.bundleD;
        }
    }
}
