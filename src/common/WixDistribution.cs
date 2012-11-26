//-------------------------------------------------------------------------------------------------
// <copyright file="WixDistribution.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Common assembly information.
// </summary>
//-------------------------------------------------------------------------------------------------

using System.Reflection;
using System.Resources;

[assembly: AssemblyCompany("Microsoft Corporation")]
[assembly: AssemblyCopyright("Copyright © Microsoft Corporation. All rights reserved.")]
[assembly: AssemblyTrademark("")]
#if DEBUG
    [assembly: AssemblyConfiguration("DEBUG")]
#else
    [assembly: AssemblyConfiguration("")]
#endif
[assembly: NeutralResourcesLanguage("en-US")]
[assembly: AssemblyProduct("WiX Toolset")]
