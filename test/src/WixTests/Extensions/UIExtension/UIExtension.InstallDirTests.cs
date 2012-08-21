//-----------------------------------------------------------------------
// <copyright file="UIExtension.InstallDirTests.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>UI Extension InstallDir tests</summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Extensions.UIExtension
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using WixTest;
    using WixTest.Verifiers;

    /// <summary>
    /// NetFX extension InstallDir element tests
    /// </summary>
    [TestClass]
    public class InstallDirTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Extensions\UIExtension\InstallDirTests");

        [TestMethod]
        [Description("Verify that the CustomAction Table is created in the MSI and has the expected data.")]
        [Priority(1)]
        public void InstallDir_VerifyMSITableData()
        {
            string sourceFile = Path.Combine(InstallDirTests.TestDataDirectory, @"product.wxs");
            string msiFile = Builder.BuildPackage(sourceFile, "test.msi", "WixUIExtension");

            Verifier.VerifyCustomActionTableData(msiFile,
                new CustomActionTableData("WixUIValidatePath", 65, "WixUIWixca", "ValidatePath"),
                new CustomActionTableData("WixUIPrintEula", 65, "WixUIWixca", "PrintEula"));
        }

        [TestMethod]
        [Description("Verify using the msilog that the correct actions was executed.")]
        [Priority(2)]
        [TestProperty("IsRuntimeTest", "false")]
        [Ignore]
        public void InstallDir_ChangeInstallDir()
        {
        }
    }
}
