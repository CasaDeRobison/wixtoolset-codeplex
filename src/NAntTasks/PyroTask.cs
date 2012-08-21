//--------------------------------------------------------------------------------------------------
// <copyright file="PyroTask.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// NAnt task for the pyro tool.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.NAntTasks
{
    using System;
    using System.IO;

    using NAnt.Core;
    using NAnt.Core.Attributes;
    using NAnt.Core.Types;

    /// <summary>
    /// Represents the NAnt task for the &lt;pyro&gt; element in a NAnt script.
    /// </summary>
    [TaskName("pyro")]
    public class PyroTask : SingleOutputWixTask
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private Transform[] transforms;

        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="PyroTask"/> class.
        /// </summary>
        public PyroTask()
            : base("pyro.exe")
        {
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets or sets the list of transforms.
        /// </summary>
        [BuildElementCollection("transforms", "transform", ElementType = typeof(Transform))]
        public Transform[] Transforms
        {
            get { return this.transforms; }
            set { this.transforms = value; }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Writes all of the command-line parameters for the tool to a response file, one parameter per line.
        /// </summary>
        /// <param name="writer">The output writer.</param>
        protected override void WriteOptions(TextWriter writer)
        {
            base.WriteOptions(writer);

            foreach (Transform transform in this.Transforms)
            {
                writer.WriteLine("-t \"{0}\" \"{1}\"", transform.Baseline, transform.FilePath);
            }
        }
    }
}