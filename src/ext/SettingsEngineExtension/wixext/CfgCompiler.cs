//-------------------------------------------------------------------------------------------------
// <copyright file="CfgCompiler.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The compiler for the Windows Installer XML Toolset Cfg Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Extensions
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Xml;
    using System.Xml.Schema;
    using Microsoft.Tools.WindowsInstallerXml;

    /// <summary>
    /// The compiler for the Windows Installer XML Toolset Cfg Extension.
    /// </summary>
    public sealed class CfgCompiler : CompilerExtension
    {
        private XmlSchema schema;
        internal const int MsidbRegistryRootLocalMachine = 2;
        internal const int MsidbComponentAttributesRegistryKeyPath = 4;

        /// <summary>
        /// Instantiate a new CfgCompiler.
        /// </summary>
        public CfgCompiler()
        {
            this.schema = LoadXmlSchemaHelper(Assembly.GetExecutingAssembly(), "Microsoft.Tools.WindowsInstallerXml.Extensions.Xsd.cfg.xsd");
        }

        /// <summary>
        /// Gets the schema for this extension.
        /// </summary>
        /// <value>Schema for this extension.</value>
        public override XmlSchema Schema
        {
            get { return this.schema; }
        }

        /// <summary>
        /// Processes an element for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        public override void ParseElement(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlElement element, params string[] contextValues)
        {
            switch (parentElement.LocalName)
            {
                case "Module":
                case "Product":
                case "Fragment":
                    switch (element.LocalName)
                    {
                        case "Product":
                            this.ParseCfgProductElement(element);
                            break;
                        default:
                            this.Core.UnexpectedElement(parentElement, element);
                            break;
                    }
                    break;
                default:
                    this.Core.UnexpectedElement(parentElement, element);
                    break;
            }
        }

        /// <summary>
        /// Parses a CfgException element.
        /// </summary>
        /// <param name="node">The element to parse.</param>
        private void ParseCfgProductElement(XmlNode node)
        {
            SourceLineNumberCollection sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);
            string name = null;
            string version = null;
            string publickey = null;
            string feature = null;

            foreach (XmlAttribute attrib in node.Attributes)
            {
                if (0 == attrib.NamespaceURI.Length || attrib.NamespaceURI == this.schema.TargetNamespace)
                {
                    switch (attrib.LocalName)
                    {
                        case "Name":
                            name = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            break;
                        case "Version":
                            version = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            break;
                        case "PublicKey":
                            publickey = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            break;
                        case "Feature":
                            feature = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            break;
                        default:
                            this.Core.UnexpectedAttribute(sourceLineNumbers, attrib);
                            break;
                    }
                }
                else
                {
                    this.Core.UnsupportedExtensionAttribute(sourceLineNumbers, attrib);
                }
            }

            // Id and Name are required
            if (null == name)
            {
                this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.Name, "Name"));
            }

            if (null == version)
            {
                this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.Name, "Version"));
            }

            if (null == publickey)
            {
                this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.Name, "PublicKey"));
            }

            if (null == feature)
            {
                this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.Name, "Feature"));
            }

            if (!this.Core.EncounteredError)
            {
                string componentGuid = CompilerCore.NewGuid(CfgConstants.wixCfgGuidNamespace, name + "_" + version + "_" + publickey);
                string componentId = "Cfg_" + componentGuid.Substring(1, componentGuid.Length - 2).Replace("-", "_"); // Cutoff the curly braces and switch dashes to underscrores to get the componentID
                string regId = "Reg_" + componentId;

                Row componentRow = this.Core.CreateRow(sourceLineNumbers, "Component");
                componentRow[0] = componentId;
                componentRow[1] = componentGuid;
                componentRow[2] = "TARGETDIR";
                componentRow[3] = MsidbComponentAttributesRegistryKeyPath;
                componentRow[4] = "";
                componentRow[5] = regId;

                Row featureComponentRow = this.Core.CreateRow(sourceLineNumbers, "FeatureComponents");
                featureComponentRow[0] = feature;
                featureComponentRow[1] = componentId;

                Row cfgRow = this.Core.CreateRow(sourceLineNumbers, "WixCfgProducts");
                cfgRow[0] = name;
                cfgRow[1] = version;
                cfgRow[2] = publickey;
                cfgRow[3] = componentId;

                Row regRow = this.Core.CreateRow(sourceLineNumbers, "Registry");
                regRow[0] = regId;
                regRow[1] = MsidbRegistryRootLocalMachine;
                regRow[2] = "SOFTWARE\\Wix\\SettingsStore\\Products";
                regRow[3] = name + ", " + version + ", " + publickey;
                regRow[4] = "#1";
                regRow[5] = componentId;

                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "Feature", feature);

                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "CustomAction", "SchedCfgProductsInstall");
                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "CustomAction", "SchedCfgProductsUninstall");
            }
        }
    }
}
