---
title: WixUI_FeatureTree Dialog Set
layout: documentation
---
# WixUI_FeatureTree Dialog Set

WixUI_FeatureTree is a simpler version of <a href="WixUI_Mondo.htm">WixUI_Mondo</a> that omits the setup type dialog. Instead, the wizard proceeds directly from the license agreement dialog to the feature customization dialog. WixUI_FeatureTree is more appropriate than WixUI_Mondo when your product installs all features by default.

This dialog set is defined in the file <b>WixUI_FeatureTree.wxs</b> in the WixUIExtension in the WiX source code.

## WixUI_FeatureTree Dialogs

WixUI_FeatureTree includes the following dialogs:

<ul>
<li>BrowseDlg</li>
<li>CustomizeDlg</li>
<li>DiskCostDlg</li>
<li>LicenseAgreementDlg</li>
<li>WelcomeDlg</li>
</ul>

In addition, WixUI_FeatureTree includes the following common dialogs that appear in all WixUI dialog sets:

<ul>
<li>CancelDlg</li>
<li>ErrorDlg</li>
<li>ExitDlg</li>
<li>FatalError</li>
<li>FilesInUse</li>
<li>MaintenanceTypeDlg</li>
<li>MaintenanceWelcomeDlg</li>
<li>MsiRMFilesInUse</li>
<li>OutOfDiskDlg</li>
<li>OutOfRbDiskDlg</li>
<li>PrepareDlg</li>
<li>ProgressDlg</li>
<li>ResumeDlg</li>
<li>UserExit</li>
<li>VerifyReadyDlg</li>
<li>WaitForCostingDlg</li>
</ul>

See <a href="WixUI_dialogs.htm">the WixUI dialog reference</a> for detailed descriptions of each of the above dialogs.
