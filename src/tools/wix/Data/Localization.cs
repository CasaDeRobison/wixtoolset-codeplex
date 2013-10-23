//-------------------------------------------------------------------------------------------------
// <copyright file="Localization.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// Object that represents a localization file.
// </summary>
//-------------------------------------------------------------------------------------------------
namespace WixToolset
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Xml;
    using System.Xml.Linq;
    using System.Xml.Schema;
    using WixToolset.Msi.Interop;

    /// <summary>
    /// Object that represents a localization file.
    /// </summary>
    public sealed class Localization
    {
        public static readonly XNamespace WxlNamespace = "http://wixtoolset.org/schemas/v4/wxl";
        private static string XmlElementName = "WixLocalization";

        private int codepage;
        private string culture;
        private Hashtable variables = new Hashtable();
        private TableDefinitionCollection tableDefinitions;
        private Dictionary<string, LocalizedControl> localizedControls = new Dictionary<string, LocalizedControl>();

        /// <summary>
        /// Protect the constructor.
        /// </summary>
        private Localization()
        {
        }

        /// <summary>
        /// Gets the codepage.
        /// </summary>
        /// <value>The codepage.</value>
        public int Codepage
        {
            get { return this.codepage; }
        }

        /// <summary>
        /// Gets the culture.
        /// </summary>
        /// <value>The culture.</value>
        public string Culture
        {
            get { return this.culture; }
        }

        /// <summary>
        /// Gets the variables.
        /// </summary>
        /// <value>The variables.</value>
        public ICollection Variables
        {
            get { return this.variables.Values; }
        }

        /// <summary>
        /// Gets the localized controls.
        /// </summary>
        /// <value>The localized controls.</value>
        public ICollection<KeyValuePair<string, LocalizedControl>> LocalizedControls
        {
            get { return this.localizedControls; }
        }

        /// <summary>
        /// Loads a localization file from a path on disk.
        /// </summary>
        /// <param name="path">Path to library file saved on disk.</param>
        /// <param name="tableDefinitions">Collection containing TableDefinitions to use when loading the localization file.</param>
        /// <param name="suppressSchema">Suppress xml schema validation while loading.</param>
        /// <returns>Returns the loaded localization file.</returns>
        public static Localization Load(string path, TableDefinitionCollection tableDefinitions, bool suppressSchema)
        {
            try
            {
                using (FileStream stream = File.OpenRead(path))
                {
                    return Load(stream, new Uri(Path.GetFullPath(path)), tableDefinitions, suppressSchema);
                }
            }
            catch (FileNotFoundException)
            {
                throw new WixException(WixErrors.FileNotFound(null, Path.GetFullPath(path)));
            }
        }

        /// <summary>
        /// Persists a localization file into an XML format.
        /// </summary>
        /// <param name="writer">XmlWriter where the localization file should persist itself as XML.</param>
        public void Persist(XmlWriter writer)
        {
            writer.WriteStartElement(Localization.XmlElementName, WxlNamespace.NamespaceName);

            if (-1 != this.codepage)
            {
                writer.WriteAttributeString("Codepage", this.codepage.ToString(CultureInfo.InvariantCulture));
            }

            if (!String.IsNullOrEmpty(this.culture))
            {
                writer.WriteAttributeString("Culture", this.culture);
            }

            foreach (WixVariableRow wixVariableRow in this.variables.Values)
            {
                writer.WriteStartElement("String", WxlNamespace.NamespaceName);

                writer.WriteAttributeString("Id", wixVariableRow.Id);

                if (wixVariableRow.Overridable)
                {
                    writer.WriteAttributeString("Overridable", "yes");
                }

                writer.WriteCData(wixVariableRow.Value);

                writer.WriteEndElement();
            }

            foreach (string controlKey in this.localizedControls.Keys)
            {
                writer.WriteStartElement("UI", WxlNamespace.NamespaceName);

                string[] controlKeys = controlKey.Split('/');
                string dialog = controlKeys[0];
                string control = controlKeys[1];

                if (!String.IsNullOrEmpty(dialog))
                {
                    writer.WriteAttributeString("Dialog", dialog);
                }

                if (!String.IsNullOrEmpty(control))
                {
                    writer.WriteAttributeString("Control", control);
                }

                LocalizedControl localizedControl = this.localizedControls[controlKey];

                if (CompilerCore.IntegerNotSet != localizedControl.X)
                {
                    writer.WriteAttributeString("X", localizedControl.X.ToString());
                }

                if (CompilerCore.IntegerNotSet != localizedControl.Y)
                {
                    writer.WriteAttributeString("Y", localizedControl.Y.ToString());
                }

                if (CompilerCore.IntegerNotSet != localizedControl.Width)
                {
                    writer.WriteAttributeString("Width", localizedControl.Width.ToString());
                }

                if (CompilerCore.IntegerNotSet != localizedControl.Height)
                {
                    writer.WriteAttributeString("Height", localizedControl.Height.ToString());
                }

                if (MsiInterop.MsidbControlAttributesRTLRO == (localizedControl.Attributes & MsiInterop.MsidbControlAttributesRTLRO))
                {
                    writer.WriteAttributeString("RightToLeft", "yes");
                }

                if (MsiInterop.MsidbControlAttributesRightAligned == (localizedControl.Attributes & MsiInterop.MsidbControlAttributesRightAligned))
                {
                    writer.WriteAttributeString("RightAligned", "yes");
                }

                if (MsiInterop.MsidbControlAttributesLeftScroll == (localizedControl.Attributes & MsiInterop.MsidbControlAttributesLeftScroll))
                {
                    writer.WriteAttributeString("LeftScroll", "yes");
                }

                if (!String.IsNullOrEmpty(localizedControl.Text))
                {
                    writer.WriteCData(localizedControl.Text);
                }

                writer.WriteEndElement();
            }

            writer.WriteEndElement();
        }

        /// <summary>
        /// Merge the information from another localization object into this one.
        /// </summary>
        /// <param name="localization">The localization object to be merged into this one.</param>
        public void Merge(Localization localization)
        {
            foreach (WixVariableRow wixVariableRow in localization.Variables)
            {
                WixVariableRow existingWixVariableRow = (WixVariableRow)variables[wixVariableRow.Id];

                if (null == existingWixVariableRow || (existingWixVariableRow.Overridable && !wixVariableRow.Overridable))
                {
                    variables[wixVariableRow.Id] = wixVariableRow;
                }
                else if (!wixVariableRow.Overridable)
                {
                    throw new WixException(WixErrors.DuplicateLocalizationIdentifier(wixVariableRow.SourceLineNumbers, wixVariableRow.Id));
                }
            }
        }

        /// <summary>
        /// Loads a localization file from a stream.
        /// </summary>
        /// <param name="stream">Stream containing the localization file.</param>
        /// <param name="uri">Uri for finding this stream.</param>
        /// <param name="tableDefinitions">Collection containing TableDefinitions to use when loading the localization file.</param>
        /// <param name="suppressSchema">Suppress xml schema validation while loading.</param>
        /// <returns>Returns the loaded localization file.</returns>
        internal static Localization Load(Stream stream, Uri uri, TableDefinitionCollection tableDefinitions, bool suppressSchema)
        {
            try
            {
                using (XmlReader reader = XmlReader.Create(stream, null, uri.AbsoluteUri))
                {
                    return Localization.Parse(reader, tableDefinitions, false);
                }
            }
            catch (XmlException xe)
            {
                throw new WixException(WixErrors.InvalidXml(SourceLineNumber.CreateFromUri(uri.AbsoluteUri), "localization", xe.Message));
            }
            catch (XmlSchemaException xse)
            {
                throw new WixException(WixErrors.SchemaValidationFailed(SourceLineNumber.CreateFromUri(uri.AbsoluteUri), xse.Message, xse.LineNumber, xse.LinePosition));
            }
        }

        /// <summary>
        /// Parse a localization file from an XML format.
        /// </summary>
        /// <param name="reader">XmlReader where the localization file is persisted.</param>
        /// <param name="tableDefinitions">Collection containing TableDefinitions to use when parsing the localization file.</param>
        /// <returns>The parsed localization.</returns>
        internal static Localization Parse(XmlReader reader, TableDefinitionCollection tableDefinitions, bool fragment)
        {
            XElement root =  fragment ? XElement.ReadFrom(reader) as XElement : XDocument.Load(reader).Root;

            Localization localization = new Localization();
            localization.tableDefinitions = tableDefinitions;
            localization.Parse(root);

            return localization;
        }

        /// <summary>
        /// Parse a localization file from an XML document.
        /// </summary>
        /// <param name="document">XmlDocument where the localization file is persisted.</param>
        internal void Parse(XElement root)
        {
            SourceLineNumber sourceLineNumbers = Preprocessor.GetSourceLineNumbers(root);
            if (Localization.XmlElementName == root.Name.LocalName)
            {
                if (WxlNamespace == root.Name.Namespace)
                {
                    this.ParseWixLocalizationElement(root);
                }
                else // invalid or missing namespace
                {
                    if (null == root.Name.Namespace)
                    {
                        throw new WixException(WixErrors.InvalidWixXmlNamespace(sourceLineNumbers, Localization.XmlElementName, Localization.WxlNamespace.NamespaceName));
                    }
                    else
                    {
                        throw new WixException(WixErrors.InvalidWixXmlNamespace(sourceLineNumbers, Localization.XmlElementName, root.Name.LocalName, Localization.WxlNamespace.NamespaceName));
                    }
                }
            }
            else
            {
                throw new WixException(WixErrors.InvalidDocumentElement(sourceLineNumbers, root.Name.LocalName, "localization", Localization.XmlElementName));
            }
        }

        /// <summary>
        /// Parses the WixLocalization element.
        /// </summary>
        /// <param name="node">Element to parse.</param>
        private void ParseWixLocalizationElement(XElement node)
        {
            int codepage = -1;
            string culture = null;
            SourceLineNumber sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);

            foreach (XAttribute attrib in node.Attributes())
            {
                if (String.IsNullOrEmpty(attrib.Name.NamespaceName) || Localization.WxlNamespace == attrib.Name.Namespace)
                {
                    switch (attrib.Name.LocalName)
                    {
                        case "Codepage":
                            codepage = Common.GetValidCodePage(attrib.Value, true);
                            break;
                        case "Culture":
                            culture = attrib.Value;
                            break;
                        case "Language":
                            // do nothing; @Language is used for locutil which can't convert Culture to lcid
                            break;
                        default:
                            Common.UnexpectedAttribute(sourceLineNumbers, attrib, Localization.OnMessage);
                            break;
                    }
                }
                else
                {
                    Common.UnsupportedExtensionAttribute(sourceLineNumbers, attrib, Localization.OnMessage);
                }
            }

            this.codepage = codepage;
            this.culture = String.IsNullOrEmpty(culture) ? String.Empty : culture.ToLower(CultureInfo.InvariantCulture);

            foreach (XElement child in node.Elements())
            {
                if (Localization.WxlNamespace == child.Name.Namespace)
                {
                    switch (child.Name.LocalName)
                    {
                        case "String":
                            this.ParseString(child);
                            break;
                        case "UI":
                            this.ParseUI(child);
                            break;
                        default:
                            throw new WixException(WixErrors.UnexpectedElement(sourceLineNumbers, node.Name.ToString(), child.Name.ToString()));
                    }
                }
                else
                {
                    throw new WixException(WixErrors.UnsupportedExtensionElement(sourceLineNumbers, node.Name.ToString(), child.Name.ToString()));
                }
            }
        }


        /// <summary>
        /// Parse a localization string.
        /// </summary>
        /// <param name="node">Element to parse.</param>
        private void ParseString(XElement node)
        {
            string id = null;
            bool overridable = false;
            SourceLineNumber sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);

            foreach (XAttribute attrib in node.Attributes())
            {
                if (String.IsNullOrEmpty(attrib.Name.NamespaceName) || Localization.WxlNamespace == attrib.Name.Namespace)
                {
                    switch (attrib.Name.LocalName)
                    {
                        case "Id":
                            id = Common.GetAttributeIdentifierValue(sourceLineNumbers, attrib, null);
                            break;
                        case "Overridable":
                            overridable = YesNoType.Yes == Common.GetAttributeYesNoValue(sourceLineNumbers, attrib, null);
                            break;
                        case "Localizable":
                            ; // do nothing
                            break;
                        default:
                            throw new WixException(WixErrors.UnexpectedAttribute(sourceLineNumbers, attrib.Parent.Name.ToString(), attrib.Name.ToString()));
                    }
                }
                else
                {
                    throw new WixException(WixErrors.UnsupportedExtensionAttribute(sourceLineNumbers, attrib.Parent.Name.ToString(), attrib.Name.ToString()));
                }
            }

            string value = Common.GetInnerText(node);

            if (null == id)
            {
                throw new WixException(WixErrors.ExpectedAttribute(sourceLineNumbers, "String", "Id"));
            }
            else if (0 == id.Length)
            {
                throw new WixException(WixErrors.IllegalIdentifier(sourceLineNumbers, "String", "Id", 0));
            }

            WixVariableRow wixVariableRow = new WixVariableRow(sourceLineNumbers, this.tableDefinitions["WixVariable"]);
            wixVariableRow.Id = id;
            wixVariableRow.Overridable = overridable;
            wixVariableRow.Value = value;

            WixVariableRow existingWixVariableRow = (WixVariableRow)this.variables[id];
            if (null == existingWixVariableRow || (existingWixVariableRow.Overridable && !overridable))
            {
                this.variables.Add(id, wixVariableRow);
            }
            else if (!overridable)
            {
                throw new WixException(WixErrors.DuplicateLocalizationIdentifier(sourceLineNumbers, id));
            }
        }

        /// <summary>
        /// Parse a localized control.
        /// </summary>
        /// <param name="node">Element to parse.</param>
        /// <param name="localization">The localization being parsed.</param>
        private void ParseUI(XElement node)
        {
            string dialog = null;
            string control = null;
            int x = CompilerCore.IntegerNotSet;
            int y = CompilerCore.IntegerNotSet;
            int width = CompilerCore.IntegerNotSet;
            int height = CompilerCore.IntegerNotSet;
            int attribs = 0;
            string text = null;
            SourceLineNumber sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);

            foreach (XAttribute attrib in node.Attributes())
            {
                if (String.IsNullOrEmpty(attrib.Name.NamespaceName) || Localization.WxlNamespace == attrib.Name.Namespace)
                {
                    switch (attrib.Name.LocalName)
                    {
                        case "Dialog":
                            dialog = Common.GetAttributeIdentifierValue(sourceLineNumbers, attrib, null);
                            break;
                        case "Control":
                            control = Common.GetAttributeIdentifierValue(sourceLineNumbers, attrib, null);
                            break;
                        case "X":
                            x = Common.GetAttributeIntegerValue(sourceLineNumbers, attrib, 0, short.MaxValue, null);
                            break;
                        case "Y":
                            y = Common.GetAttributeIntegerValue(sourceLineNumbers, attrib, 0, short.MaxValue, null);
                            break;
                        case "Width":
                            width = Common.GetAttributeIntegerValue(sourceLineNumbers, attrib, 0, short.MaxValue, null);
                            break;
                        case "Height":
                            height = Common.GetAttributeIntegerValue(sourceLineNumbers, attrib, 0, short.MaxValue, null);
                            break;
                        case "RightToLeft":
                            if (YesNoType.Yes == Common.GetAttributeYesNoValue(sourceLineNumbers, attrib, null))
                            {
                                attribs |= MsiInterop.MsidbControlAttributesRTLRO;
                            }
                            break;
                        case "RightAligned":
                            if (YesNoType.Yes == Common.GetAttributeYesNoValue(sourceLineNumbers, attrib, null))
                            {
                                attribs |= MsiInterop.MsidbControlAttributesRightAligned;
                            }
                            break;
                        case "LeftScroll":
                            if (YesNoType.Yes == Common.GetAttributeYesNoValue(sourceLineNumbers, attrib, null))
                            {
                                attribs |= MsiInterop.MsidbControlAttributesLeftScroll;
                            }
                            break;
                        default:
                            Common.UnexpectedAttribute(sourceLineNumbers, attrib, Localization.OnMessage);
                            break;
                    }
                }
                else
                {
                    Common.UnsupportedExtensionAttribute(sourceLineNumbers, attrib, Localization.OnMessage);
                }
            }

            text = Common.GetInnerText(node);

            if (String.IsNullOrEmpty(control) && 0 < attribs)
            {
                if (MsiInterop.MsidbControlAttributesRTLRO == (attribs & MsiInterop.MsidbControlAttributesRTLRO))
                {
                    throw new WixException(WixErrors.IllegalAttributeWithoutOtherAttributes(sourceLineNumbers, node.Name.ToString(), "RightToLeft", "Control"));
                }
                else if (MsiInterop.MsidbControlAttributesRightAligned == (attribs & MsiInterop.MsidbControlAttributesRightAligned))
                {
                    throw new WixException(WixErrors.IllegalAttributeWithoutOtherAttributes(sourceLineNumbers, node.Name.ToString(), "RightAligned", "Control"));
                }
                else if (MsiInterop.MsidbControlAttributesLeftScroll == (attribs & MsiInterop.MsidbControlAttributesLeftScroll))
                {
                    throw new WixException(WixErrors.IllegalAttributeWithoutOtherAttributes(sourceLineNumbers, node.Name.ToString(), "LeftScroll", "Control"));
                }
            }

            if (String.IsNullOrEmpty(control) && String.IsNullOrEmpty(dialog))
            {
                throw new WixException(WixErrors.ExpectedAttributesWithOtherAttribute(sourceLineNumbers, node.Name.ToString(), "Dialog", "Control"));
            }

            string key = LocalizedControl.GetKey(dialog, control);
            if (this.localizedControls.ContainsKey(key))
            {
                if (String.IsNullOrEmpty(control))
                {
                    throw new WixException(WixErrors.DuplicatedUiLocalization(sourceLineNumbers, dialog));
                }
                else
                {
                    throw new WixException(WixErrors.DuplicatedUiLocalization(sourceLineNumbers, dialog, control));
                }
            }

            this.localizedControls.Add(key, new LocalizedControl(x, y, width, height, attribs, text));
        }

        /// <summary>
        /// Throws an exception for an error message event.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public static void OnMessage(MessageEventArgs e)
        {
            WixErrorEventArgs errorEventArgs = e as WixErrorEventArgs;
            if (null != errorEventArgs)
            {
                throw new WixException(errorEventArgs);
            }
        }
    }

    public class LocalizedControl
    {
        public int X { get; private set; }
        public int Y { get; private set; }
        public int Width { get; private set; }
        public int Height { get; private set; }
        public int Attributes { get; private set; }
        public string Text { get; private set; }

        public LocalizedControl(int x, int y, int width, int height, int attribs, string text)
        {
            this.X = x;
            this.Y = y;
            this.Width = width;
            this.Height = height;
            this.Attributes = attribs;
            this.Text = text;
        }

        /// <summary>
        /// Get key for a localized control.
        /// </summary>
        /// <param name="dialog">The optional id of the control's dialog.</param>
        /// <param name="control">The id of the control.</param>
        /// <returns>The localized control id.</returns>
        public static string GetKey(string dialog, string control)
        {
            return String.Concat(String.IsNullOrEmpty(dialog) ? String.Empty : dialog, "/", String.IsNullOrEmpty(control) ? String.Empty : control);
        }
    }
}
