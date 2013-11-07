//-------------------------------------------------------------------------------------------------
// <copyright file="GamingExtensionData.cs" company="Outercurve Foundation">
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

    public sealed class GamingExtensionData : ExtensionData
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
                if (null == GamingExtensionData.tableDefinitions)
                {
                    GamingExtensionData.tableDefinitions = ExtensionData.LoadTableDefinitionHelper(Assembly.GetExecutingAssembly(), "WixToolset.Extensions.Data.tables.xml");
                }

                return GamingExtensionData.tableDefinitions;
            }
        }

        /// <summary>
        /// Gets the library associated with this extension.
        /// </summary>
        /// <param name="tableDefinitions">The table definitions to use while loading the library.</param>
        /// <returns>The loaded library.</returns>
        public override Library GetLibrary(TableDefinitionCollection tableDefinitions)
        {
            if (null == GamingExtensionData.library)
            {
                GamingExtensionData.library = ExtensionData.LoadLibraryHelper(Assembly.GetExecutingAssembly(), "WixToolset.Extensions.Data.gaming.wixlib", tableDefinitions);
            }

            return GamingExtensionData.library;
        }
    }
}
