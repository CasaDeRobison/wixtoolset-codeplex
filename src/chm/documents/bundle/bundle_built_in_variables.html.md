---
title: Burn Built-in Variables
layout: documentation
after: authoring_bundle_package_manifest
---
# Burn Built-in Variables

The Burn engine offers a set of commonly-used variables so you can use them without defining your own. Here is the list of the built-in variable names:

* AdminToolsFolder - gets the well-known folder for CSIDL_ADMINTOOLS.
* AppDataFolder - gets the well-known folder for CSIDL_APPDATA.
* CommonAppDataFolder - gets the well-known folder for CSIDL_COMMON_APPDATA.
* CommonFilesFolder - gets the well-known folder for CSIDL_PROGRAM_FILES_COMMONX86.
* CommonFiles64Folder - gets the well-known folder for CSIDL_PROGRAM_FILES_COMMON.
* CommonFiles6432Folder - gets the well-known folder for CSIDL_PROGRAM_FILES_COMMON on 64-bit Windows and CSIDL_PROGRAM_FILES_COMMONX86 on 32-bit Windows.
* CompatibilityMode - non-zero if the operating system launched the bootstrapper in compatibility mode.
* ComputerName - name of the computer as returned by GetComputerName() function.
* Date - gets the current date using the short date format of the current user locale.
* DesktopFolder - gets the well-known folder for CSIDL_DESKTOP.
* FavoritesFolder - gets the well-known folder for CSIDL_FAVORITES.
* FontsFolder - gets the well-known folder for CSIDL_FONTS.
* InstallerName - gets the name of the installer engine ("WiX Burn").
* InstallerVersion - gets the version of the installer engine.
* LocalAppDataFolder - gets the well-known folder for CSIDL_LOCAL_APPDATA.
* LogonUser - gets the current user name.
* MyPicturesFolder - gets the well-known folder for CSIDL_MYPICTURES.
* NTProductType - numeric product type from OS version information.
* NTSuiteBackOffice - non-zero if OS version suite is Back Office.
* NTSuiteDataCenter - non-zero if OS version suite is Datacenter.
* NTSuiteEnterprise - non-zero if OS version suite is Enterprise.
* NTSuitePersonal - non-zero if OS version suite is Personal.
* NTSuiteSmallBusiness - non-zero if OS version suite is Small Business.
* NTSuiteSmallBusinessRestricted - non-zero if OS version suite is Restricted Small Business.
* NTSuiteWebServer - non-zero if OS version suite is Web Server.
* PersonalFolder - gets the well-known folder for CSIDL_PERSONAL.
* Privileged - non-zero if the process could run elevated (on Vista+) or is running as an Administrator (on WinXP).
* ProgramFilesFolder - gets the well-known folder for CSIDL_PROGRAM_FILESX86.
* ProgramFiles64Folder - gets the well-known folder for CSIDL_PROGRAM_FILES.
* ProgramFiles6432Folder - gets the well-known folder for CSIDL_PROGRAM_FILES on 64-bit Windows and CSIDL_PROGRAM_FILESX86 on 32-bit Windows.
* ProgramMenuFolder - gets the well-known folder for CSIDL_PROGRAMS.
* RebootPending - non-zero if the system requires a reboot. Note that this value will reflect the reboot status of the system when the variable is first requested.
* SendToFolder - gets the well-known folder for CSIDL_SENDTO.
* ServicePackLevel - numeric value representing the installed OS service pack.
* StartMenuFolder - gets the well-known folder for CSIDL_STARTMENU.
* StartupFolder - gets the well-known folder for CSIDL_STARTUP.
* SystemFolder - gets the well-known folder for CSIDL_SYSTEMX86.
* SystemLanguageID - gets the language ID for the system locale.
* TempFolder - gets the well-known folder for temp location
* TemplateFolder - gets the well-known folder for CSIDL_TEMPLATES.
* TerminalServer - non-zero if the system is running in application server mode of Remote Desktop Services.
* UserLanguageID - gets the language ID for the current user locale.
* VersionMsi - version value representing the Windows Installer engine version.
* VersionNT - version value representing the OS version. The result is a version variable (v#.#.#.#) which differs from the MSI Property 'VersionNT' which is an integer. For example, to use this variable in a Bundle condition try: "VersionNT &gt; v6.1"
* VersionNT64 - version value representing the OS version if 64-bit. Undefined if running a 32-bit operating system. The result is a version variable (v#.#.#.#) which differs from the MSI Property 'VersionNT64' which is an integer. For example, to use this variable in a Bundle condition try: "VersionNT &gt; v6.1"
* WindowsFolder - gets the well-known folder for CSIDL_WINDOWS.
* WindowsVolume - gets the well-known folder for the windows volume.
* WixBundleAction - set to the numeric value of BOOTSTRAPPER_ACTION from the command-line and updated during the call to IBootstrapperEngine::Plan().
* WixBundleDirectoryLayout - set to the folder provided to the -layout switch (default is directory containin the bundle executable). This variable can also be set by the bootstrapper application to modify where files will be laid out.
* WixBundleElevated - gets whether the bundle was launched elevated and will be set to 1 once the bundle is elevated. Use this variable, for example, to show or hide the elevation shield in the bootstrapper application UI.
* WixBundleForcedRestartPackage - gets the id of the package that caused a force restart during apply. This value is reset on the next call to apply.
* WixBundleInstalled - gets whether the bundle was already installed and will be set to 1 once the bundle is installed.
* WixBundleLastUsedSource - gets the path of the last successful source resolution for a container or payload.
* WixBundleName - gets the name of the bundle (from Bundle/@Name). This variable can also be set by the bootstrapper application to modify the bundle name at runtime.
* WixBundleManufacturer - gets the manufacturer of the bundle (from Bundle/@Manufacturer).
* WixBundleOriginalSource - gets the source path from where the bundle was originally installed.
* WixBundleProviderKey - gets the bundle dependency provider key.
* WixBundleTag - gets the developer-defined tag string for this bundle (from Bundle/@Tag).
* WixBundleVersion - gets the version for this bundle (from Bundle/@Version).
