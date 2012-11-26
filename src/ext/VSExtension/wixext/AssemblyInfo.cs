//-------------------------------------------------------------------------------------------------
// <copyright file="AssemblyInfo.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The assembly information for the WiX Toolset Visual Studio Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using WixToolset;
using WixToolset.Extensions;
using WixToolset.Tools;

[assembly: AssemblyTitle("WiX Toolset VS Extension")]
[assembly: AssemblyDescription("WiX Toolset Visual Studio Extension")]
[assembly: AssemblyCulture("")]
[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]
[assembly: AssemblyDefaultWixExtension(typeof(VSExtension))]
[assembly: AssemblyDefaultHeatExtension(typeof(VSHeatExtension))]
