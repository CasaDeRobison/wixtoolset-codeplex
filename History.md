* BMurri: Fix links and cleanup documentation formating.

* BobArnson: SFBUG:1747 - Handle missing tables and system tables that exist only when SQL queries are being run when determining row counts.

* BobArnson: SFBUG:2235 - Use overload of WaitHandle.WaitOne that's available on .NET 2.0.

* BobArnson: SFBUG:1677 - Update conditions for Firewall custom actions to support Windows 2003 Server SP1.

## WixBuild: Version 3.8.628.0

* RobMen: Updated release system.

* MiCarls: SFBUG:3288 - Fix unhandled exception when harvesting unsupported registry types from .reg files.

* Corfe83: SFBUG:3283 - Allow wix tools to load remote wix extensions / other assemblies

* BobArnson: SFBUG:3116 - Add @HideTarget to custom actions and @Hidden to custom action data properties.

* RobMen: New help build system.

## WixBuild: Version 3.8.611.0

* BorisDachev: SFBUG:3066 - harvest project links in MSBuild 4.0 correctly.

* BMurri: SFBUG:3285 - Explicitly set enable32BitAppOnWin64 in IIS7 AppPools.

* NeilSleighthold: Add probing for default sub language of the detected primary language.

* BarryNolte: SFBUG:3296 - quote NETFX4.5 log path.

* BarryNolte: SFBUG:3288 - improve heat.exe error message for unhandled registry types.

* PatOShea - SFBUG:2485;3119;3020;3223 - fixed links in html documentation.

## WixBuild: Version 3.8.520.0

* StefanAgner: SFBUG:3074 - fix harvesting to correctly handle WinForms.resx by changing current directory.

* BobArnson: SFBUG:3262 - Implement MediaTemplate/@CompressionLevel.

* BobArnson: SFBUG:3266 - reword error message to avoid implying required values can be omitted.

* BobArnson: SFBUG:3256 - be a bit more lax about accepting binder variables anywhere in version strings.

* BobArnson: SFBUG:3248 - alias UI and Control elements in .wxl files for LocUtil.

* BobArnson: SFBUG:3233 - verify bundle version elements are <=UInt16 once all binder variables are resolved.

* JCHoover: SFFEAT:727 - Added support for ActionData messages (ACTIONSTART) within WixStdBA.

* JCHoover: SFEAT:629 - skip shortcut scheduling if the CA is unable to create CLSID_InternetShortcut or CLSID_ShellLink.

* Corfe83: SFBUG:3251 - Fix heat to no longer hang with ProgIds whose CurVer is their own ProgId.

## WixBuild: Version 3.8.514.0

* JCHoover: SFFEAT:636;668 - support wildcard ProductCode and configuring UpgradeCode in InstanceTransforms.

* JHennessey: Add assemblyPublicKeyTokenPreservedCase and assemblyFullNamePreservedCase binder variables.

* RobMen: Add support for integration tests on xUnit.net and convert one Burn test.

* NeilSleightholm: Add HyperlinkSidebar, HyperlinkLarge and RtfLarge themes to wixstdba.

## WixBuild: Version 3.8.506.0

* NeilSleightholm: SFBUG:3289 - fix 'heat reg' output to not generate candle CNDL1138 warnings.

* RobMen: Add support for xUnit.net based tests.

## WixBuild: Version 3.8.422.0

* JoshuaKwan: Added strings in Error table for errors 1610,1611,1651 but not localized yet.

## WixBuild: Version 3.8.415.0

* RobMen: SFBUG:3263 - correctly register bundle when present in cache but pending removal on restart.

## WixBuild: Version 3.8.408.0

* RobMen: SFBUG:3178 - make NETFX v3.5 default base for WiX toolset.

* RobMen: SFBUG:3222 - fix Manufacturer in WiX installs.

* NeilSleightholm: Add support for BA functions to wixstdba.

* RobMen: Allow WebApplication to be defined under WebDir element.

## WixBuild: Version 3.8.401.0

* RobMen: Add arguments and ability to hide launch target in wixstdba.

* NeilSleightholm: Add support for additional controls on the install and options pages.

* NeilSleightholm: Add support for an auto generated version number to the preprocessor.

* NeilSleightholm: SFBUG:3191 - document NETFX v4.5 properties.

* NeilSleightholm: SFBUG:3221 - use correct function to get user language in LocProbeForFile().

## WixBuild: Version 3.8.326.0

* BruceCran: Use v110_xp instead of v110 for VS2012 to retain compatibility with XP.

* RobMen: SFBUG:3160 - pass "files in use" message to BA correctly.

* BobArnson: SFBUG:3236 - Enable Lux in binaries.zip and setup.

* RobMen: Enhance wixstdba to display user and warning MSI messages.

## WixBuild: Version 3.8.305.0

* RobMen: Add PromptToContinue attribute to CloseApplications.

* RobMen: Enhance wixstdba progress text for related bundles.

## WixBuild: Version 3.8.225.0

* RobMen: Add EndSession, TerminateProcess and Timeout attributes to CloseApplications.

* RobMen: Support transparent .pngs for Images in thmutil.

* RobMen: SFBUG:3216 - fix build to handle multiple platform SDK include paths.

* RobMen: WiX v3.8

## WixBuild: Version 3.8.0.0
