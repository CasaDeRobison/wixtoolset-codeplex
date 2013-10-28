//-------------------------------------------------------------------------------------------------
// <copyright file="HeatCore.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The WiX Toolset Harvester application core.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Tools
{
    using System;
    using System.Reflection;
    using WixToolset.Extensibilty;
    using Wix = WixToolset.Serialize;

    /// <summary>
    /// The WiX Toolset Harvester application core.
    /// </summary>
    public sealed class HeatCore : IHeatCore
    {
        private bool encounteredError;
        private Harvester harvester;
        private Mutator mutator;

        /// <summary>
        /// Instantiates a new HeatCore.
        /// </summary>
        /// <param name="messageHandler">The message handler for the core.</param>
        public HeatCore(MessageEventHandler messageHandler)
        {
            this.MessageHandler = messageHandler;
            this.harvester = new Harvester();
            this.mutator = new Mutator();
        }

        /// <summary>
        /// Event for messages.
        /// </summary>
        private event MessageEventHandler MessageHandler;

        /// <summary>
        /// Gets whether the mutator core encountered an error while processing.
        /// </summary>
        /// <value>Flag if core encountered an error during processing.</value>
        public bool EncounteredError
        {
            get { return this.encounteredError; }
        }

        /// <summary>
        /// Gets the harvester.
        /// </summary>
        /// <value>The harvester.</value>
        public Harvester Harvester
        {
            get { return this.harvester; }
        }

        /// <summary>
        /// Gets the mutator.
        /// </summary>
        /// <value>The mutator.</value>
        public Mutator Mutator
        {
            get { return this.mutator; }
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs mea)
        {
            if (mea is WixErrorEventArgs)
            {
                this.encounteredError = true;
            }

            if (null != this.MessageHandler)
            {
                this.MessageHandler(this, mea);
                if (MessageLevel.Error == mea.Level)
                {
                    this.encounteredError = true;
                }
            }
        }
    }
}
