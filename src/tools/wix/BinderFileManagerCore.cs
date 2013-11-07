﻿//-------------------------------------------------------------------------------------------------
// <copyright file="BinderFileManagerCore.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using WixToolset.Extensibility;

    public class BinderFileManagerCore : IBinderFileManagerCore
    {
        private Dictionary<BindStage, Dictionary<string, List<string>>> bindPaths;

        /// <summary>
        /// Instantiate a new BinderFileManager.
        /// </summary>
        public BinderFileManagerCore()
        {
            this.bindPaths = new Dictionary<BindStage, Dictionary<string, List<string>>>();
            this.bindPaths.Add(BindStage.Normal, new Dictionary<string, List<string>>());
            this.bindPaths.Add(BindStage.Target, new Dictionary<string, List<string>>());
            this.bindPaths.Add(BindStage.Updated, new Dictionary<string, List<string>>());
        }

        /// <summary>
        /// Gets or sets the path to cabinet cache.
        /// </summary>
        /// <value>The path to cabinet cache.</value>
        public string CabCachePath { get; set; }

        /// <summary>
        /// Gets or sets the option to reuse cabinets in the cache.
        /// </summary>
        /// <value>The option to reuse cabinets in the cache.</value>
        public bool ReuseCabinets { get; set; }

        /// <summary>
        /// Gets or sets the active subStorage used for binding.
        /// </summary>
        /// <value>The subStorage object.</value>
        public SubStorage ActiveSubStorage { get; set; }

        /// <summary>
        /// Gets or sets the option to enable building binary delta patches.
        /// </summary>
        /// <value>The option to enable building binary delta patches.</value>
        public bool DeltaBinaryPatch { get; set; }

        /// <summary>
        /// Gets or sets the output object used for binding.
        /// </summary>
        /// <value>The output object.</value>
        public Output Output { get; set; }

        /// <summary>
        /// Gets or sets the path to the temp files location.
        /// </summary>
        /// <value>The path to the temp files location.</value>
        public string TempFilesLocation { get; set; }

        /// <summary>
        /// Gets the property if re-basing target is true or false
        /// </summary>
        /// <value>It returns true if target bind path is to be replaced, otherwise false.</value>
        public bool RebaseTarget
        {
            get { return this.bindPaths[BindStage.Target].Any(); }
        }

        /// <summary>
        /// Gets the property if re-basing updated build is true or false
        /// </summary>
        /// <value>It returns true if updated bind path is to be replaced, otherwise false.</value>
        public bool RebaseUpdated
        {
            get { return this.bindPaths[BindStage.Updated].Any(); }
        }

        public void AddBindPaths(IEnumerable<BindPath> paths, BindStage stage)
        {
            Dictionary<string, List<string>> dict;
            if (!this.bindPaths.TryGetValue(stage, out dict))
            {
                throw new ArgumentException("stage");
            }

            foreach (BindPath bindPath in paths)
            {
                List<string> values;
                if (!dict.TryGetValue(bindPath.Name, out values))
                {
                    values = new List<string>();
                    dict.Add(bindPath.Name, values);
                }

                if (!values.Contains(bindPath.Path))
                {
                    values.Add(bindPath.Path);
                }
            }
        }

        public IEnumerable<string> GetBindPaths(BindStage stage = BindStage.Normal, string name = null)
        {
            List<string> paths;
            if (this.bindPaths[stage].TryGetValue(name ?? String.Empty, out paths))
            {
                return paths;
            }

            return new string[0];
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="e">Message event arguments.</param>
        public void OnMessage(MessageEventArgs e)
        {
            Messaging.Instance.OnMessage(e);
        }
    }
}
