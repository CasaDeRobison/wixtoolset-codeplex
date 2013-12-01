//-------------------------------------------------------------------------------------------------
// <copyright file="UtilExtensionData.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Extensions
{
    using System;
    using System.Reflection;
    using WixToolset.Extensibility;

    /// <summary>
    /// The WiX Toolset Utility Extension Data.
    /// </summary>
    public sealed class UtilExtensionData : ExtensionData
    {
        private static Library library;
        private static TableDefinitionCollection tableDefinitions;

        /// <summary>
        /// Gets the default culture.
        /// </summary>
        /// <value>The default culture.</value>
        public override string DefaultCulture
        {
            get { return "en-us"; }
        }

        /// <summary>
        /// Gets the optional table definitions for this extension.
        /// </summary>
        /// <value>The optional table definitions for this extension.</value>
        public override TableDefinitionCollection TableDefinitions
        {
            get
            {
                return UtilExtensionData.GetExtensionTableDefinitions();
            }
        }

        /// <summary>
        /// Gets the library associated with this extension.
        /// </summary>
        /// <param name="tableDefinitions">The table definitions to use while loading the library.</param>
        /// <returns>The loaded library.</returns>
        public override Library GetLibrary(TableDefinitionCollection tableDefinitions)
        {
            return UtilExtensionData.GetExtensionLibrary(tableDefinitions);
        }

        /// <summary>
        /// Internal mechanism to access the extension's table definitions.
        /// </summary>
        /// <returns>Extension's table definitions.</returns>
        internal static TableDefinitionCollection GetExtensionTableDefinitions()
        {
            if (null == UtilExtensionData.tableDefinitions)
            {
                UtilExtensionData.tableDefinitions = ExtensionData.LoadTableDefinitionHelper(Assembly.GetExecutingAssembly(), "WixToolset.Extensions.Data.tables.xml");
            }

            return UtilExtensionData.tableDefinitions;
        }

        /// <summary>
        /// Internal mechanism to access the extension's library.
        /// </summary>
        /// <returns>Extension's library.</returns>
        internal static Library GetExtensionLibrary(TableDefinitionCollection tableDefinitions)
        {
            if (null == UtilExtensionData.library)
            {
                UtilExtensionData.library = ExtensionData.LoadLibraryHelper(Assembly.GetExecutingAssembly(), "WixToolset.Extensions.Data.util.wixlib", tableDefinitions);
            }

            return UtilExtensionData.library;
        }
    }
}