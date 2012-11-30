//-------------------------------------------------------------------------------------------------
// <copyright file="AssemblyInfo.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Markup;

[assembly: AssemblyTitle("Simplified WiX Toolset Core")]
[assembly: AssemblyDescription("")]
[assembly: AssemblyCulture("")]
[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]

// Map XML namespaces to prefixes to CLR namespaces.
[assembly: XmlnsPrefix("http://wixtoolset.org/schemas/2010/wix/simplified", "")]
[assembly: XmlnsPrefix("http://wixtoolset.org/schemas/2010/wix/simplified/appx", "appx")]
[assembly: XmlnsPrefix("http://wixtoolset.org/schemas/2010/wix/simplified/msi", "msi")]
[assembly: XmlnsPrefix("http://wixtoolset.org/schemas/2010/wix/simplified/vsix", "vsix")]

[assembly: XmlnsDefinition("http://wixtoolset.org/schemas/2010/wix/simplified", "WixToolset.Simplified.Lexicon")]
[assembly: XmlnsDefinition("http://wixtoolset.org/schemas/2010/wix/simplified/appx", "WixToolset.Simplified.Lexicon.Appx")]
[assembly: XmlnsDefinition("http://wixtoolset.org/schemas/2010/wix/simplified/msi", "WixToolset.Simplified.Lexicon.Msi")]
[assembly: XmlnsDefinition("http://wixtoolset.org/schemas/2010/wix/simplified/vsix", "WixToolset.Simplified.Lexicon.Vsix")]

// Expose our internal types to the unit test assembly.
//[assembly: InternalsVisibleTo("WixToolset.Simplified.UnitTest.Swcore")]
