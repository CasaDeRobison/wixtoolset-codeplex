//-------------------------------------------------------------------------------------------------
// <copyright file="ICompilerExtension.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Extensibility
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.Composition;
    using System.Linq;
    using System.Text;
    using System.Xml.Linq;

    public interface ICompilerExtension
    {
        /// <summary>
        /// Gets or sets the compiler core for the extension.
        /// </summary>
        /// <value>Compiler core for the extension.</value>
        [Import]
        ICompilerCore Core { get; set; }

        /// <summary>
        /// Called at the beginning of the compilation of a source file.
        /// </summary>
        void InitializeCompile();

        /// <summary>
        /// Processes an attribute for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of attribute.</param>
        /// <param name="attribute">Attribute to process.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        void ParseAttribute(XElement parentElement, XAttribute attribute, IDictionary<string, string> contextValues);

        /// <summary>
        /// Processes an element for the Compiler.
        /// </summary>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="context">Extra information about the context in which this element is being parsed.</param>
        void ParseElement(XElement parentElement, XElement element, IDictionary<string, string> context);

        /// <summary>
        /// Processes an element for the Compiler, with the ability to supply a component keypath.
        /// </summary>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        ComponentKeyPath ParsePossibleKeyPathElement(XElement parentElement, XElement element, IDictionary<string, string> context);

        /// <summary>
        /// Called at the end of the compilation of a source file.
        /// </summary>
        void FinalizeCompile();
    }
}
