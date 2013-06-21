---
title: Working with WiX Standard Bootstrapper Application
layout: documentation
---
# Working with WiX Standard Bootstrapper Application

As described in the introduction to <a href="authoring_bundle_intro.htm">building installation package bundles</a>, every bundle requires a bootstrapper application DLL to drive the Burn engine. Custom bootstrapper applications can be created but require the developer to write native or managed code. Therefore the WiX toolset provides a standard bootstrapper application that developers can use and customize in particular ways.

There are several variants of the WiX Standard Bootstrapper Application.

<ol>
  <li>WixStandardBootstrapperApplication.RtfLicense - the first variant displays the license in the welcome dialog similar to the WixUI Advanced.</li>

  <li>WixStandardBootstrapperApplication.HyperlinkLicense - the second variant provides an optional hyperlink to the license agreement on the welcome dialog providing a more modern and streamlined look.</li>

  <li>WixStandardBootstrapperApplication.HyperlinkSidebarLicense - the third variant is based on HyperlinkLicense but provides a larger dialog and larger image on the initial page.</li>

  <li>WixStandardBootstrapperApplication.RtfLargeLicense - this variant is similar to RtfLicense but is a larger dialog and supports the option of displaying the version number.</li>

  <li>WixStandardBootstrapperApplication.HyperlinkLargeLicense - this variant is similar to HyperlinkLicense but is a larger dialog and supports the option of displaying the version number.</li>

  <li>WixStandardBootstrapperApplication.Foundation - the final variant is blank and requires the developer to provide a theme file that can completely customize the look and feel</li>
</ol>

<p>To include use the WiX Standard Bootstrapper Application, a <a href="wix_xsd_bootstrapperapplicationref.htm">&lt;BootstrapperApplicationRef&gt;</a> element should reference one of the above identifiers. The following example uses the bootstrapper application that displays the license: </p>

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;&gt;
      &lt;Bundle&gt;
<strong class="highlight">        &lt;BootstrapperApplicationRef Id=&quot;WixStandardBootstrapperApplication.RtfLicense&quot; /&gt;</strong>
        &lt;Chain&gt;
        &lt;/Chain&gt;
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>

<p>RtfLicense, HyperlinkLicense and Hyperlink2License can optionally display the application version on the welcome page: </p>

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;&gt;
      &lt;Bundle&gt;
        &lt;BootstrapperApplicationRef Id=&quot;WixStandardBootstrapperApplication.RtfLicense&quot;&gt;
          &lt;bal:WixStandardBootstrapperApplication
            LicenseFile="path\to\license.rtf"
            <strong class="highlight">ShowVersion="yes"</strong>
            /&gt;
        &lt;/BootstrapperApplicationRef&gt;
        &lt;Chain&gt;
        &lt;/Chain&gt;
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>

<p>When building the bundle, the WixBalExtension must be provided. If the above code was in a file called &quot;example.wxs&quot; the following steps would create an &quot;example.exe&quot; bundle:</p>

<pre>    candle.exe example.wxs -ext WixBalExtension
    light.exe example.wixobj -ext WixBalExtension</pre>

<p>The following topics provide information how to customize the WiX Standard Bootstrapper Application:</p>

<ul>
  <li><a href="wixstdba_license.htm">Specifying the WiX Standard Bootstrapper Application License</a></li>

  <li><a href="wixstdba_branding.htm">Changing the WiX Standard Bootstrapper Application Branding</a></li>
</ul>
