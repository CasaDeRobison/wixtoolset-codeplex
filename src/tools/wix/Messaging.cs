//-------------------------------------------------------------------------------------------------
// <copyright file="Messaging.cs" company="Outercurve Foundation">
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
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using WixToolset.Extensibility;

    public class Messaging : IMessageHandler
    {
        private static readonly Messaging instance = new Messaging();

        private HashSet<int> suppressedWarnings = new HashSet<int>();
        private HashSet<int> warningsAsErrors = new HashSet<int>();
        private string longAppName;
        private string shortAppName;

        static Messaging()
        {
        }

        private Messaging()
        {
        }

        public static Messaging Instance { get { return Messaging.instance; } }

        /// <summary>
        /// Event fired when messages are to be displayed.
        /// </summary>
        public event DisplayEventHandler Display;

        /// <summary>
        /// Gets a bool indicating whether an error has been found.
        /// </summary>
        /// <value>A bool indicating whether an error has been found.</value>
        public bool EncounteredError { get; private set; }

        /// <summary>
        /// Gets the last error code encountered during messaging.
        /// </summary>
        /// <value>The exit code for the process.</value>
        public int LastErrorNumber { get; private set; }

        /// <summary>
        /// Gets or sets the option to show verbose messages.
        /// </summary>
        /// <value>The option to show verbose messages.</value>
        public bool ShowVerboseMessages { get; set; }

        /// <summary>
        /// Gets or sets the option to suppress all warning messages.
        /// </summary>
        /// <value>The option to suppress all warning messages.</value>
        public bool SuppressAllWarnings { get; set; }

        /// <summary>
        /// Gets and sets the option to treat warnings as errors.
        /// </summary>
        /// <value>The option to treat warnings as errors.</value>
        public bool WarningsAsError { get; set; }

        /// <summary>
        /// Implements IMessageHandler to display error messages.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs mea)
        {
            if (null != this.Display)
            {
                string message = this.GetMessageString(mea);
                if (!String.IsNullOrEmpty(message))
                {
                    this.Display(this, new DisplayEventArgs() { Message = message });
                }
            }
            else if (mea is WixErrorEventArgs)
            {
                throw new WixException((WixErrorEventArgs)mea);
            }
        }

        /// <summary>
        /// Sets the app names.
        /// </summary>
        /// <param name="shortName">Short application name; usually 4 uppercase characters.</param>
        /// <param name="longName">Long application name; usually the executable name.</param>
        public Messaging InitializeAppName(string shortName, string longName)
        {
            this.shortAppName = shortName;
            this.longAppName = longName;

            return this;
        }

        /// <summary>
        /// Adds a warning message id to be elevated to an error message.
        /// </summary>
        /// <param name="warningNumber">Id of the message to elevate.</param>
        /// <remarks>
        /// Suppressed warnings will not be elevated as errors.
        /// </remarks>
        public void ElevateWarningMessage(int warningNumber)
        {
            this.warningsAsErrors.Add(warningNumber);
        }

        /// <summary>
        /// Adds a warning message id to be suppressed in message output.
        /// </summary>
        /// <param name="warningNumber">Id of the message to suppress.</param>
        /// <remarks>
        /// Suppressed warnings will not be elevated as errors.
        /// </remarks>
        public void SuppressWarningMessage(int warningNumber)
        {
            this.suppressedWarnings.Add(warningNumber);
        }

        /// <summary>
        /// Get a message string.
        /// </summary>
        /// <param name="sender">Sender of the message.</param>
        /// <param name="mea">Arguments for the message event.</param>
        /// <returns>The message string.</returns>
        public string GetMessageString(MessageEventArgs mea)
        {
            MessageLevel messageLevel = this.CalculateMessageLevel(mea);

            if (MessageLevel.Nothing == messageLevel)
            {
                return null;
            }

            return this.GenerateMessageString(messageLevel, mea);
        }

        /// <summary>
        /// Determines the level of this message, when taking into account warning-as-error, 
        /// warning level, verbosity level and message suppressed by the caller.
        /// </summary>
        /// <param name="mea">Event arguments for the message.</param>
        /// <returns>MessageLevel representing the level of this message.</returns>
        private MessageLevel CalculateMessageLevel(MessageEventArgs mea)
        {
            MessageLevel messageLevel = MessageLevel.Nothing;

            if (mea is WixVerboseEventArgs)
            {
                if (this.ShowVerboseMessages)
                {
                    messageLevel = MessageLevel.Information;
                }
            }
            else if (mea is WixWarningEventArgs)
            {
                if (this.SuppressAllWarnings || this.suppressedWarnings.Contains(mea.Id))
                {
                    messageLevel = MessageLevel.Nothing;
                }
                else if (this.WarningsAsError || this.warningsAsErrors.Contains(mea.Id))
                {
                    messageLevel = MessageLevel.Error;
                }
                else
                {
                    messageLevel = MessageLevel.Warning;
                }
            }
            else if (mea is WixErrorEventArgs)
            {
                messageLevel = MessageLevel.Error;
            }
            else if (mea is WixGenericMessageEventArgs)
            {
                messageLevel = mea.Level;
            }
            else
            {
                throw new ArgumentException(String.Format(CultureInfo.InvariantCulture, "Unknown MessageEventArgs type: {0}.", mea.GetType().ToString()));
            }

            mea.Level = messageLevel;
            return messageLevel;
        }

        /// <summary>
        /// Creates a properly formatted message string.
        /// </summary>
        /// <param name="messageLevel">Level of the message, as generated by MessageLevel(MessageEventArgs).</param>
        /// <param name="mea">Event arguments for the message.</param>
        /// <returns>String containing the formatted message.</returns>
        private string GenerateMessageString(MessageLevel messageLevel, MessageEventArgs mea)
        {
            if (MessageLevel.Nothing == messageLevel)
            {
                return String.Empty;
            }

            if (MessageLevel.Error == messageLevel)
            {
                this.EncounteredError = true;
            }

            List<string> fileNames = new List<string>();
            string errorFileName = this.longAppName ?? "WIX";
            for (SourceLineNumber sln = mea.SourceLineNumbers; null != sln; sln = sln.Parent)
            {
                if (sln.LineNumber.HasValue)
                {
                    if (0 == fileNames.Count)
                    {
                        errorFileName = String.Format(CultureInfo.CurrentUICulture, WixStrings.Format_FirstLineNumber, sln.FileName, sln.LineNumber);
                    }

                    fileNames.Add(String.Format(CultureInfo.CurrentUICulture, WixStrings.Format_LineNumber, sln.FileName, sln.LineNumber));
                }
                else
                {
                    if (0 == fileNames.Count)
                    {
                        errorFileName = sln.FileName;
                    }

                    fileNames.Add(sln.FileName);
                }
            }

            string messageType = String.Empty;
            if (MessageLevel.Warning == messageLevel)
            {
                messageType = WixStrings.MessageType_Warning;
            }
            else if (MessageLevel.Error == messageLevel)
            {
                this.LastErrorNumber = mea.Id;
                messageType = WixStrings.MessageType_Error;
            }

            StringBuilder messageBuilder = new StringBuilder();
            string message = String.Format(CultureInfo.InvariantCulture, mea.ResourceManager.GetString(mea.ResourceName), mea.MessageArgs);
            if (MessageLevel.Information == messageLevel)
            {
                messageBuilder.AppendFormat(WixStrings.Format_InfoMessage, message);
            }
            else
            {
                messageBuilder.AppendFormat(WixStrings.Format_NonInfoMessage, errorFileName, messageType, this.shortAppName ?? "WIX", mea.Id, message);
            }

            if (1 < fileNames.Count)
            {
                messageBuilder.AppendFormat(WixStrings.INF_SourceTrace, Environment.NewLine);
                foreach (string fileName in fileNames)
                {
                    messageBuilder.AppendFormat(WixStrings.INF_SourceTraceLocation, fileName, Environment.NewLine);
                }

                messageBuilder.Append(Environment.NewLine);
            }

            return messageBuilder.ToString();
        }
    }
}
