//-------------------------------------------------------------------------------------------------
// <copyright file="IPreprocessorExtension.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Extensibility
{
    using System;

    /// <summary>
    /// Interface for extending the WiX toolset preprocessor.
    /// </summary>
    public interface IPreprocessorExtension
    {
        /// <summary>
        /// Sends a message with the given arguments.
        /// </summary>
        /// <param name="e">Message arguments.</param>
        void OnMessage(MessageEventArgs e);
    }
}
