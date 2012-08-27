//-------------------------------------------------------------------------------------------------
// <copyright file="PreProcExampleExtension.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The Windows Installer XML Toolset PreProcesses Example Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Reflection;

    /// <summary>
    /// The Windows Installer XML Toolset PreProcessor Example Extension.
    /// </summary>
    public sealed class PreProcExampleExtension : WixExtension
    {
        private PreProcExtension extension;

        /// <summary>
        /// Gets the optional preprocessor extension.
        /// </summary>
        /// <value>The optional preprocessor extension.</value>
        public override PreprocessorExtension PreprocessorExtension
        {
            get
            {
                if (null == this.extension)
                {
                    this.extension = new PreProcExtension();
                }

                return this.extension;
            }
        }
    }
}
