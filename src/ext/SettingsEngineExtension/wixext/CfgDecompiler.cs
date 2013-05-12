//-------------------------------------------------------------------------------------------------
// <copyright file="CfgDecompiler.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The decompiler for the Windows Installer XML Toolset Cfg Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Extensions
{
    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Globalization;

    using Cfg = WixToolset.Extensions.Serialize.Cfg;
    using Wix = WixToolset.Serialize;

    /// <summary>
    /// The decompiler for the Windows Installer XML Toolset Cfg Extension.
    /// </summary>
    public sealed class CfgDecompiler : DecompilerExtension
    {
        /// <summary>
        /// Decompiles an extension table.
        /// </summary>
        /// <param name="table">The table to decompile.</param>
        public override void DecompileTable(Table table)
        {
            switch (table.Name)
            {
                default:
                    base.DecompileTable(table);
                    break;
            }
        }
    }
}
