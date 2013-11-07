//-------------------------------------------------------------------------------------------------
// <copyright file="light.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The light linker application.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Tools
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.Linq;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Xml;
    using System.Xml.XPath;
    using WixToolset.Extensibility;

    /// <summary>
    /// The main entry point for light.
    /// </summary>
    public sealed class Light
    {
        LightCommandLine commandLine;
        private IEnumerable<IExtensionData> extensionData;
        private IEnumerable<IBinderExtension> binderExtensions;
        private IEnumerable<IBinderFileManager> fileManagers;

        /// <summary>
        /// The main entry point for light.
        /// </summary>
        /// <param name="args">Commandline arguments for the application.</param>
        /// <returns>Returns the application error code.</returns>
        [MTAThread]
        public static int Main(string[] args)
        {
            AppCommon.PrepareConsoleForLocalization();
            Messaging.Instance.InitializeAppName("LGHT", "light.exe").Display += Light.DisplayMessage;

            Light light = new Light();
            return light.Execute(args);
        }

        /// <summary>
        /// Handler for display message events.
        /// </summary>
        /// <param name="sender">Sender of message.</param>
        /// <param name="e">Event arguments containing message to display.</param>
        private static void DisplayMessage(object sender, DisplayEventArgs e)
        {
            Console.WriteLine(e.Message);
        }

        /// <summary>
        /// Main running method for the application.
        /// </summary>
        /// <param name="args">Commandline arguments to the application.</param>
        /// <returns>Returns the application error code.</returns>
        private int Execute(string[] args)
        {
            try
            {
                string[] unparsed = this.ParseCommandLineAndLoadExtensions(args);

                if (!Messaging.Instance.EncounteredError)
                {
                    if (this.commandLine.ShowLogo)
                    {
                        AppCommon.DisplayToolHeader();
                    }

                    if (this.commandLine.ShowHelp)
                    {
                        this.PrintHelp();
                        AppCommon.DisplayToolFooter();
                    }
                    else
                    {
                        foreach (string arg in unparsed)
                        {
                            Messaging.Instance.OnMessage(WixWarnings.UnsupportedCommandLineArgument(arg));
                        }

                        this.Run();
                    }
                }
            }
            catch (WixException we)
            {
                Messaging.Instance.OnMessage(we.Error);
            }
            catch (Exception e)
            {
                Messaging.Instance.OnMessage(WixErrors.UnexpectedException(e.Message, e.GetType().ToString(), e.StackTrace));
                if (e is NullReferenceException || e is SEHException)
                {
                    throw;
                }
            }

            return Messaging.Instance.LastErrorNumber;
        }

        /// <summary>
        /// Parse command line and load all the extensions.
        /// </summary>
        /// <param name="args">Command line arguments to be parsed.</param>
        private string[] ParseCommandLineAndLoadExtensions(string[] args)
        {
            this.commandLine = new LightCommandLine();
            string[] unprocessed = this.commandLine.Parse(args);
            if (Messaging.Instance.EncounteredError)
            {
                return unprocessed;
            }

            // Load extensions.
            ExtensionManager extensionManager = new ExtensionManager();
            foreach (string extension in this.commandLine.Extensions)
            {
                extensionManager.Load(extension);
            }

            // Extension data command line processing.
            this.extensionData = extensionManager.Create<IExtensionData>();
            foreach (IExtensionCommandLine dce in this.extensionData.Where(e => e is IExtensionCommandLine).Cast<IExtensionCommandLine>())
            {
                dce.MessageHandler = Messaging.Instance;
                unprocessed = dce.ParseCommandLine(unprocessed);
            }

            // Binder extensions command line processing.
            this.binderExtensions = extensionManager.Create<IBinderExtension>();
            foreach (IExtensionCommandLine bce in this.binderExtensions.Where(e => e is IExtensionCommandLine).Cast<IExtensionCommandLine>())
            {
                bce.MessageHandler = Messaging.Instance;
                unprocessed = bce.ParseCommandLine(unprocessed);
            }

            // File resolution command line processing.
            this.fileManagers = extensionManager.Create<IBinderFileManager>();
            if (this.fileManagers.Any())
            {
                foreach (IExtensionCommandLine fme in this.fileManagers.Where(e => e is IExtensionCommandLine).Cast<IExtensionCommandLine>())
                {
                    fme.MessageHandler = Messaging.Instance;
                    unprocessed = fme.ParseCommandLine(unprocessed);
                }
            }
            else // there are no extension file managers so add the default one.
            {
                List<IBinderFileManager> defaultBinderFileManager = new List<IBinderFileManager>();
                defaultBinderFileManager.Add(new BinderFileManager());

                this.fileManagers = defaultBinderFileManager;
            }

            return commandLine.ParsePostExtensions(unprocessed);
        }

        private void Run()
        {
            // Initialize the variable resolver from the command line.
            WixVariableResolver wixVariableResolver = new WixVariableResolver();
            foreach (var wixVar in this.commandLine.Variables)
            {
                wixVariableResolver.AddVariable(wixVar.Key, wixVar.Value);
            }

            // Initialize the linker from the command line.
            Linker linker = new Linker();
            linker.UnreferencedSymbolsFile = this.commandLine.UnreferencedSymbolsFile;
            linker.ShowPedanticMessages = this.commandLine.ShowPedanticMessages;
            linker.WixVariableResolver = wixVariableResolver;

            foreach (IExtensionData data in this.extensionData)
            {
                linker.AddExtensionData(data);
            }

            // Initialize the binder from the command line.
            WixToolset.Binder binder = new WixToolset.Binder();
            binder.CabCachePath = this.commandLine.CabCachePath;
            binder.ReuseCabinets = this.commandLine.ReuseCabinets;
            binder.ContentsFile = this.commandLine.ContentsFile;
            binder.BuiltOutputsFile = this.commandLine.BuiltOutputsFile;
            binder.OutputsFile = this.commandLine.OutputsFile;
            binder.WixprojectFile = this.commandLine.WixprojectFile;
            binder.BindPaths.AddRange(this.commandLine.BindPaths);
            binder.CabbingThreadCount = this.commandLine.CabbingThreadCount;
            binder.DefaultCompressionLevel = this.commandLine.DefaultCompressionLevel;
            binder.ExactAssemblyVersions = this.commandLine.ExactAssemblyVersions;
            binder.Ices.AddRange(this.commandLine.Ices);
            binder.SuppressIces.AddRange(this.commandLine.SuppressIces);
            binder.SetMsiAssemblyNameFileVersion = this.commandLine.SetMsiAssemblyNameFileVersion;
            binder.SuppressAclReset = this.commandLine.SuppressAclReset;
            binder.SuppressLayout = this.commandLine.SuppressLayout;
            binder.SuppressValidation = this.commandLine.SuppressValidation;
            binder.PdbFile = this.commandLine.SuppressWixPdb ? null : this.commandLine.PdbFile;
            binder.TempFilesLocation = Environment.GetEnvironmentVariable("WIX_TEMP") ?? Path.GetTempPath();
            binder.WixVariableResolver = wixVariableResolver;

            foreach (IBinderExtension extension in this.binderExtensions)
            {
                binder.AddExtension(extension);
            }

            foreach (IBinderFileManager fm in this.fileManagers)
            {
                binder.AddExtension(fm);
            }

            // Initialize the localizer.
            Localizer localizer = this.InitializeLocalization(linker.TableDefinitions);
            if (Messaging.Instance.EncounteredError)
            {
                return;
            }

            wixVariableResolver.Localizer = localizer;
            linker.Localizer = localizer;
            binder.Localizer = localizer;

            // Loop through all the believed object files.
            SectionCollection sections = new SectionCollection();
            Output output = null;
            foreach (string inputFile in this.commandLine.Files)
            {
                string inputFileFullPath = Path.GetFullPath(inputFile);

                // try loading as an object file
                try
                {
                    Intermediate intermediate = Intermediate.Load(inputFileFullPath, linker.TableDefinitions, this.commandLine.SuppressVersionCheck, true);
                    sections.AddRange(intermediate.Sections);
                    continue; // next file
                }
                catch (WixNotIntermediateException)
                {
                    // try another format
                }

                // try loading as a library file
                try
                {
                    Library library = Library.Load(inputFileFullPath, linker.TableDefinitions, this.commandLine.SuppressVersionCheck, true);
                    library.GetLocalizations(this.commandLine.Cultures, localizer);
                    sections.AddRange(library.Sections);
                    continue; // next file
                }
                catch (WixNotLibraryException)
                {
                    // try another format
                }

                // try loading as an output file
                output = Output.Load(inputFileFullPath, this.commandLine.SuppressVersionCheck, true);
            }

            // Stop processing if any errors were found loading object files.
            if (Messaging.Instance.EncounteredError)
            {
                return;
            }

            // and now for the fun part
            if (null == output)
            {
                OutputType expectedOutputType = OutputType.Unknown;
                if (!String.IsNullOrEmpty(this.commandLine.OutputFile))
                {
                    expectedOutputType = Output.GetOutputType(Path.GetExtension(this.commandLine.OutputFile));
                }

                ArrayList transforms = new ArrayList();
                output = linker.Link(sections, transforms, expectedOutputType);

                // If an error occurred during linking, stop processing.
                if (null == output)
                {
                    return;
                }
            }
            else if (0 != sections.Count)
            {
                throw new InvalidOperationException(LightStrings.EXP_CannotLinkObjFilesWithOutpuFile);
            }

            bool tidy = true; // clean up after ourselves by default.
            try
            {
                // only output the xml if its a patch build or user specfied to only output wixout
                string outputFile = this.commandLine.OutputFile;
                string outputExtension = Path.GetExtension(outputFile);
                if (this.commandLine.OutputXml || OutputType.Patch == output.Type)
                {
                    if (String.IsNullOrEmpty(outputExtension) || outputExtension.Equals(".wix", StringComparison.Ordinal))
                    {
                        outputExtension = (OutputType.Patch == output.Type) ? ".wixmsp" : ".wixout";
                        outputFile = Path.ChangeExtension(outputFile, outputExtension);
                    }

                    output.Save(outputFile, null, wixVariableResolver, binder.TempFilesLocation);
                }
                else // finish creating the MSI/MSM
                {
                    if (String.IsNullOrEmpty(outputExtension) || outputExtension.Equals(".wix", StringComparison.Ordinal))
                    {
                        outputExtension = Output.GetExtension(output.Type);
                        outputFile = Path.ChangeExtension(outputFile, outputExtension);
                    }

                    binder.Bind(output, outputFile);
                }
            }
            catch (WixException we) // keep files around for debugging IDT issues.
            {
                if (we is WixInvalidIdtException)
                {
                    tidy = false;
                }

                throw;
            }
            catch (Exception) // keep files around for debugging unexpected exceptions.
            {
                tidy = false;
                throw;
            }
            finally
            {
                if (null != binder)
                {
                    binder.Cleanup(tidy);
                }
            }

            return;
        }

        private Localizer InitializeLocalization(TableDefinitionCollection tableDefinitions)
        {
            Localizer localizer = null;

            // Instantiate the localizer and load any localization files.
            if (!this.commandLine.SuppressLocalization || 0 < this.commandLine.LocalizationFiles.Count || null != this.commandLine.Cultures || !this.commandLine.OutputXml)
            {
                List<Localization> localizations = new List<Localization>();

                // Load each localization file.
                foreach (string localizationFile in this.commandLine.LocalizationFiles)
                {
                    Localization localization = Localization.Load(localizationFile, tableDefinitions, true);
                    localizations.Add(localization);
                }

                localizer = new Localizer();
                if (null != this.commandLine.Cultures)
                {
                    // Alocalizations in order specified in cultures.
                    foreach (string culture in this.commandLine.Cultures)
                    {
                        foreach (Localization localization in localizations)
                        {
                            if (culture.Equals(localization.Culture, StringComparison.OrdinalIgnoreCase))
                            {
                                localizer.AddLocalization(localization);
                            }
                        }
                    }
                }
                else // no cultures specified, so try neutral culture and if none of those add all loc files.
                {
                    bool neutralFound = false;
                    foreach (Localization localization in localizations)
                    {
                        if (String.IsNullOrEmpty(localization.Culture))
                        {
                            // If a neutral wxl was provided use it.
                            localizer.AddLocalization(localization);
                            neutralFound = true;
                        }
                    }

                    if (!neutralFound)
                    {
                        // No cultures were specified and no neutral wxl are available, include all of the loc files.
                        foreach (Localization localization in localizations)
                        {
                            localizer.AddLocalization(localization);
                        }
                    }
                }

                // Load localizations provided by extensions with data.
                foreach (IExtensionData data in this.extensionData)
                {
                    Library library = data.GetLibrary(tableDefinitions);
                    if (null != library)
                    {
                        // Load the extension's default culture if it provides one and no cultures were specified.
                        string[] extensionCultures = this.commandLine.Cultures;
                        if (null == extensionCultures && null != data.DefaultCulture)
                        {
                            extensionCultures = new string[] { data.DefaultCulture };
                        }

                        library.GetLocalizations(extensionCultures, localizer);
                    }
                }
            }

            return localizer;
        }

        /// <summary>
        /// Prints usage help.
        /// </summary>
        private void PrintHelp()
        {
            string lightArgs = LightStrings.CommandLineArguments;

            Console.WriteLine(String.Format(LightStrings.HelpMessage, lightArgs));
        }
    }
}
