//-------------------------------------------------------------------------------------------------
// <copyright file="CompilerCore.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// The base compiler extension.  Any of these methods can be overridden to change
// the behavior of the compiler.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Extensibility
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Security.Cryptography;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Xml;
    using System.Xml.Schema;
    using Wix = WixToolset.Serialize;

    /// <summary>
    /// A set of rules describing the whitespace rules for an attribute.
    /// </summary>
    public enum EmptyRule
    {
        /// <summary>
        /// The trimmed value cannot be empty.
        /// </summary>
        MustHaveNonWhitespaceCharacters,

        /// <summary>
        /// The trimmed value can be empty, but the value itself cannot be empty.
        /// </summary>
        CanBeWhitespaceOnly,

        /// <summary>
        /// The value can be empty.
        /// </summary>
        CanBeEmpty
    }

    public enum ValueListKind
    {
        /// <summary>
        /// A list of values with nothing before the final value.
        /// </summary>
        None,

        /// <summary>
        /// A list of values with 'and' before the final value.
        /// </summary>
        And,

        /// <summary>
        /// A list of values with 'or' before the final value.
        /// </summary>
        Or
    }

    /// <summary>
    /// Core interface provided by the compiler.
    /// </summary>
    public interface ICompilerCore : IMessageHandler
    {
        /// <summary>
        /// Gets the section the compiler is currently emitting symbols into.
        /// </summary>
        /// <value>The section the compiler is currently emitting symbols into.</value>
        Section ActiveSection { get; }

        /// <summary>
        /// Gets or sets the platform which the compiler will use when defaulting 64-bit attributes and elements.
        /// </summary>
        /// <value>The platform which the compiler will use when defaulting 64-bit attributes and elements.</value>
        Platform CurrentPlatform { get; }

        /// <summary>
        /// Gets whether the compiler core encountered an error while processing.
        /// </summary>
        /// <value>Flag if core encountered an error during processing.</value>
        bool EncounteredError { get; }

        /// <summary>
        /// Gets or sets the option to show pedantic messages.
        /// </summary>
        /// <value>The option to show pedantic messages.</value>
        bool ShowPedanticMessages { get; }

        /// <summary>
        /// Gets the table definitions used by the compiler core.
        /// </summary>
        /// <value>Table definition collection.</value>
        TableDefinitionCollection TableDefinitions { get; }

        /// <summary>
        /// Checks if the string contains a property (i.e. "foo[Property]bar")
        /// </summary>
        /// <param name="possibleProperty">String to evaluate for properties.</param>
        /// <returns>True if a property is found in the string.</returns>
        bool ContainsProperty(string possibleProperty);

        /// <summary>
        /// Generate an identifier by hashing data from the row.
        /// </summary>
        /// <param name="prefix">Three letter or less prefix for generated row identifier.</param>
        /// <param name="args">Information to hash.</param>
        /// <returns>The generated identifier.</returns>
        string GenerateIdentifier(string prefix, params string[] args);

        /// <summary>
        /// Creates WixComplexReference and WixGroup rows in the active section.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line information.</param>
        /// <param name="parentType">The parent type.</param>
        /// <param name="parentId">The parent id.</param>
        /// <param name="parentLanguage">The parent language.</param>
        /// <param name="childType">The child type.</param>
        /// <param name="childId">The child id.</param>
        /// <param name="isPrimary">Whether the child is primary.</param>
        void CreateComplexReference(SourceLineNumber sourceLineNumbers, ComplexReferenceParentType parentType, string parentId, string parentLanguage, ComplexReferenceChildType childType, string childId, bool isPrimary);

        /// <summary>
        /// Creates a version 3 name-based UUID.
        /// </summary>
        /// <param name="namespaceGuid">The namespace UUID.</param>
        /// <param name="value">The value.</param>
        /// <returns>The generated GUID for the given namespace and value.</returns>
        string NewGuid(Guid namespaceGuid, string value);
    }
}
