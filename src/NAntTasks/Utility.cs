//--------------------------------------------------------------------------------------------------
// <copyright file="Utility.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Utility methods for the Wix NAntTasks project.
// </summary>
//--------------------------------------------------------------------------------------------------

namespace WixToolset.NAntTasks
{
    using System;

    using NAnt.Core;
    using NAnt.Core.Util;

    /// <summary>
    /// Utility methods for the Wix NAntTasks project.
    /// </summary>
    internal sealed class Utility
    {
        #region Member Variables
        //==========================================================================================
        // Member Variables
        //==========================================================================================

        #endregion

        #region Constructors
        //==========================================================================================
        // Constructors
        //==========================================================================================

        /// <summary>
        /// Prevent direct instantiation of this class.
        /// </summary>
        private Utility()
        {
        }
        #endregion

        #region Properties
        //==========================================================================================
        // Properties
        //==========================================================================================

        #endregion
 
        #region Methods
        //==========================================================================================
        // Methods
        //==========================================================================================
        
        /// <summary>
        /// Surrounds the path with quotes if it has any embedded spaces.
        /// </summary>
        /// <param name="path">The path to quote.</param>
        /// <returns>The quoted path if it needs quotes.</returns>
        public static string QuotePathIfNeeded(string path)
        {
            // If the path is not null/emtpy, if it contains the space character somewhere,
            // and if it's not already surround in quotes, then surround it in quotes.
            if (!StringUtils.IsNullOrEmpty(path) && path.IndexOf(' ') >= 0)
            {
                if (path[0] != '"' || path[path.Length - 1] != '"' || path.Length == 1)
                {
                    path = "\"" + path + "\"";
                }
            }
            return path;
        }

        /// <summary>
        /// Verifies that the specified argument is not null and throws an exception if it is.
        /// </summary>
        /// <param name="argument">The argument to check.</param>
        /// <param name="argumentName">The name of the argument.</param>
        public static void VerifyArgumentNonNull(object argument, string argumentName)
        {
            if (argument == null)
            {
                throw new ArgumentNullException(argumentName);
            }
        }

        /// <summary>
        /// Verifies that the specifed string argument is not null and not empty and throws an
        /// exception if it is.
        /// </summary>
        /// <param name="argument">The argument to check.</param>
        /// <param name="argumentName">The name of the argument.</param>
        public static void VerifyStringArgument(string argument, string argumentName)
        {
            VerifyArgumentNonNull(argument, argumentName);
            if (argumentName.Length == 0)
            {
                throw new ArgumentException("An empty string is not allowed.", argumentName);
            }
        }
        #endregion
    }
}