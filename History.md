* MikeGC: Fix Settings Engine to behave better when remote database is not always available due to either USB drive unplugged or a network disconnection.

* MikeGC: Fix bug in Settings Engine auto sync related to pushing AND pulling information automatically when first adding a remote database.

* MikeGC: Settings Engine now more reliably connects to remote databases on windows startup, even if it runs before the network has fully initialized.

* RobMen: Merge recent changes through Wix v3.8.1021.0

* RobMen: Merge recent changes through Wix v3.8.1014.0

* MikeGC: Implement automatic synchronization of settings within Settings Engine / Settings Browser (using MonUtil).

* MikeGC: Make Settings Browser automatically start upon login, start as a tray icon, and minimize back to tray.

* MikeGC: Fix quite a few bugs in Settings Engine and Settings Browser.

## WixBuild: Version 4.0.1007.0

* RobMen: Merge recent changes through Wix v3.8.1007.0

* RobMen: Merge source code reorganization.

* RobMen: Merge recent changes through WiX v3.8.904.0

* MikeGC: MonUtil: Add 32-bit and 64-bit awareness, add support for large numbers of monitors (>64), carefully differentiate between recursive and non-recursive waits, and fix several bugs.

* MikeGC: SceUtil: Add interface to detect whether changes to the database have occurred during a session.

* RobMen: Merge recent changes through WiX v3.8.826.0

* MikeGC: Make Settings Browser window resizable, and enable AutoResize functionality in ThmUtil.

* MikeGC: Introducing MonUtil, which allow easily monitoring directories and registry keys for changes.

* RobMen: Merge recent changes through WiX v3.8.722.0

## WixBuild: Version 4.0.701.0

* RobMen: Merge recent changes through WiX v3.8.628.0.

* RobMen: Merge recent changes through WiX v3.8.611.0.

* MikeGC: Fix bug in settings browser "one instance" lock, switch from a mutex to a per-user lock, and fix some UI thread issues

* MikeGC: Fix pdbs zip and create new udms zip for settings engine manifests

* RobMen: Merge recent changes from WiX v3.8.

* MikeGC: Introducing WiX Settings Engine.

* RobMen: Merge recent changes from WiX v3.8.

## WixBuild: Version 4.0.424.0

* RobMen: Merge recent changes from WiX v3.8.

* RobMen: Add version to schema namespaces.

* RobMen: Move extension schema namespaces under "wxs" to align better with Simplified WiX.
* RobMen: Update Simplified WiX namespaces to match changes "wxs" namespace.

* RobMen: Fix bad old references to thmutil.xsd.

* RobMen: More SxS'ification of folders, registry keys, etc.
* RobMen: Fix Votive registration to correctly load in VS.
* RobMen: Add Simplified WiX Toolset to binaries.zip

* RobMen: Update WixCop to help with all namespace changes (including extensions).
* RobMen: Update thmutil.xsd namespace to be consistent with other changes.

## WixBuild: Version 4.0.4.0

* RobMen: Introducing Simplified WiX Toolset.

* RobMen: Rename "Windows Installer Xml Toolset" to "WiX Toolset".
* RobMen: Improve support for building WiX Toolset with VS2012.
* RobMen: Continue to fix namespace breaking changes.

* RobMen: Change namespaces to prepare for breaking changes.

* RobMen: WiX v4.0

## WixBuild: Version 4.0.0.0
