//-------------------------------------------------------------------------------------------------
// <copyright file="attributes.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Designer.Interfaces;
using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Xml;
using System.Diagnostics;

[assembly: InternalsVisibleTo("votive2010, PublicKey=0024000004800000940000000602000000240000525341310004000001000100c5a1665fa12cf8cb33bfdfa8b4380c6141a5f87e71588a45b0ad3e66039dcc4c5af3afd146d72f7ba97c41beae5071115ec8264d5fabdeaa9bb42bb502e607e8bb035732c2aed5ae7d99ed8cd53287abe18b06d881c5cc5f0c6f82cf7d0fef9d4f8d55f22c54504ebf9da9c71b7636eeefb93a3407e3cff549146c62f36828e7")]
[assembly: CLSCompliant(true)]

namespace Microsoft.VisualStudio.Package
{ 
    /// <summary>
    /// Defines a type converter.
    /// </summary>
    /// <remarks>This is needed to get rid of the type TypeConverter type that could not give back the Type we were passing to him.
    /// We do not want to use reflection to get the type back from the  ConverterTypeName. Also the GetType methos does not spwan converters from other assemblies.</remarks>
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Enum | AttributeTargets.Property | AttributeTargets.Field)]
    public sealed class PropertyPageTypeConverterAttribute : Attribute
    {
        #region fields
        Type converterType;
        #endregion

        #region ctors
        public PropertyPageTypeConverterAttribute(Type type)
        {
            this.converterType = type;
        } 
        #endregion

        #region properties
        public Type ConverterType
        {
            get
            {
                return this.converterType;
            }
        } 
        #endregion
    }

    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Property | AttributeTargets.Field, Inherited = false, AllowMultiple = false)]
    internal sealed class LocDisplayNameAttribute : DisplayNameAttribute
    {
        #region fields
        string name;
        #endregion

        #region ctors
        public LocDisplayNameAttribute(string name)
        {
            this.name = name;
        } 
        #endregion

        #region properties
        public override string DisplayName
        {
            get
            {
                string result = SR.GetString(this.name, CultureInfo.CurrentUICulture);
                if (result == null)
                {
                    Debug.Assert(false, "String resource '" + this.name + "' is missing");
                    result = this.name;
                }
                return result;
            }
        } 
        #endregion
    }
}
