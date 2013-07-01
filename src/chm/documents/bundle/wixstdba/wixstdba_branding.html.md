---
title: Changing the WiX Standard Bootstrapper Application Branding
layout: documentation
after: wixstdba_license
---
# Changing the WiX Standard Bootstrapper Application Branding

The WiX Standard Bootstrapper Application displays a generic logo in the bottom left corner of the user interface. It is possible to change the image displayed using the WixStandardBootstrapperApplication element provided by WixBalExtension. The following example uses a &quot;customlogo.png&quot; file found in the &quot;path\to&quot; folder relative to the linker bind paths.

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;
         xmlns:bal="http://schemas.microsoft.com/wix/BalExtension"&gt;
      &lt;Bundle&gt;
        &lt;BootstrapperApplicationRef Id=&quot;WixStandardBootstrapperApplication.RtfLicense&quot;&gt;
          &lt;bal:WixStandardBootstrapperApplication
            LicenseFile="path\to\license.rtf"
            <strong class="highlight">LogoFile="path\to\customlogo.png"</strong>
            /&gt;
        &lt;/BootstrapperApplicationRef&gt;

        &lt;Chain&gt;
          ...
        &lt;/Chain&gt;
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>

<p>For the HyperlinkSidebarLicense UI there are two logos and they can be configured as follows:</p>

    <pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;
         xmlns:bal="http://schemas.microsoft.com/wix/BalExtension"&gt;
      &lt;Bundle&gt;
        &lt;BootstrapperApplicationRef Id=&quot;WixStandardBootstrapperApplication.HyperlinkSidebarLicense&quot;&gt;
          &lt;bal:WixStandardBootstrapperApplication
            LicenseUrl="License.htm"
            <strong class="highlight">LogoFile="path\to\customlogo.png" LogoSideFile="path\to\customsidelogo.png"</strong>
            /&gt;
        &lt;/BootstrapperApplicationRef&gt;

        &lt;Chain&gt;
          ...
        &lt;/Chain&gt;
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>
