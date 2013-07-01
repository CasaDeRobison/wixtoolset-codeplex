---
title: How To: Look Inside Your MSI With Orca
layout: documentation
---
<h1>How To: Look Inside Your MSI With Orca</h1>
<p>When building installers it can often be useful to look inside your installer to see the actual tables and values that were created by the WiX build process. Microsoft provides a tool with the <a href="http://www.microsoft.com/downloads/details.aspx?FamilyId=6A35AC14-2626-4846-BB51-DDCE49D6FFB6" target="_blank">Windows Installer 4.5 SDK</a>, called Orca, that can be used for this purpose. To install Orca, download and install the Windows Installer 4.5 SDK. After the SDK installation is complete navigate to the install directory (typically <strong>C:\Program Files\Windows Installer 4.5 SDK</strong>) and open the <strong>Tools</strong> folder. Inside the Tools folder run Orca.msi to complete the installation.</p>
<p>Once Orca is installed you can right click on any MSI file from Windows Explorer and select <strong>Edit with Orca</strong> to view the contents of the MSI.</p>
