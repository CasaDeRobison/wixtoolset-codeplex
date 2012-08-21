//-------------------------------------------------------------------------------------------------
// <copyright file="PreProcExtension.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// A simple preprocessor extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Collections;
    using System.IO;

    /// <summary>
    /// The example preprocessor extension.
    /// </summary>
    public sealed class PreProcExtension : PreprocessorExtension
    {
        /// <summary>
        /// Instantiate a new PreProcExtension.
        /// </summary>
        public PreProcExtension()
        {
        }

        /// <summary>
        /// Gets the variable prefixes for this extension.
        /// </summary>
        /// <value>The variable prefixes for this extension.</value>
        public override string[] Prefixes
        {
            get
            {
                string[] prefixes = new string[1];

                prefixes[0] = "abc";

                return prefixes;
            }
        }

        /// <summary>
        /// Preprocesses a parameter.
        /// </summary>
        /// <param name="name">Name of parameter that matches extension.</param>
        /// <returns>The value of the parameter after processing.</returns>
        /// <remarks>By default this method will cause an error if its called.</remarks>
        public override string PreprocessParameter(string name)
        {
            if ("SampleVariable" == name)
            {
                return "SampleValue";
            }

            return null;
        }
    }
}
