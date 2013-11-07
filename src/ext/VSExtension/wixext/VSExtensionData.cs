//-------------------------------------------------------------------------------------------------
// <copyright file="VSExtensionData.cs" company="Outercurve Foundation">
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
    /// The WiX Toolset Visual Studio Extension.
    /// </summary>
    public sealed class VSExtensionData : ExtensionData
    {
        private static Library library;
        private static TableDefinitionCollection tableDefinitions;

        /// <summary>
        /// Gets the optional table definitions for this extension.
        /// </summary>
        /// <value>The optional table definitions for this extension.</value>
        public override TableDefinitionCollection TableDefinitions
        {
            get
            {
                return VSExtensionData.GetExtensionTableDefinitions();
            }
        }

        /// <summary>
        /// Gets the library associated with this extension.
        /// </summary>
        /// <param name="tableDefinitions">The table definitions to use while loading the library.</param>
        /// <returns>The loaded library.</returns>
        public override Library GetLibrary(TableDefinitionCollection tableDefinitions)
        {
            return VSExtensionData.GetExtensionLibrary(tableDefinitions);
        }

        /// <summary>
        /// Internal mechanism to access the extension's table definitions.
        /// </summary>
        /// <returns>Extension's table definitions.</returns>
        internal static TableDefinitionCollection GetExtensionTableDefinitions()
        {
            if (null == VSExtensionData.tableDefinitions)
            {
                VSExtensionData.tableDefinitions = ExtensionData.LoadTableDefinitionHelper(Assembly.GetExecutingAssembly(), "WixToolset.Extensions.Data.tables.xml");
            }

            return VSExtensionData.tableDefinitions;
        }

        /// <summary>
        /// Internal mechanism to access the extension's library.
        /// </summary>
        /// <returns>Extension's library.</returns>
        internal static Library GetExtensionLibrary(TableDefinitionCollection tableDefinitions)
        {
            if (null == VSExtensionData.library)
            {
                VSExtensionData.library = ExtensionData.LoadLibraryHelper(Assembly.GetExecutingAssembly(), "WixToolset.Extensions.Data.vs.wixlib", tableDefinitions);
            }

            return VSExtensionData.library;
        }
    }
}
