---
title: Author Bootstrapper Application for a Bundle
layout: documentation
after: authoring_bundle_skeleton
---
# Author Bootstrapper Application for a Bundle

Every bundle requires a bootstrapper application to drive the Burn engine. The <a href="wix_xsd_bootstrapperapplication.htm">&lt;BootstrapperApplication&gt;</a> element is used to define a new bootstrapper application DLL. The <a href="wix_xsd_bootstrapperapplicationref.htm">&lt;BootstrapperApplicationRef&gt;</a> element is used to refer to a bootstrapper application that exists in a <a href="wix_xsd_fragment.htm">&lt;Fragment&gt;</a> or WiX extension.

The <a href="wixstdba_intro.htm">WiX Standard Bootstrapper Application</a> exists in the WixBalExtension.dll. The following shows how to use it in a bundle:

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;&gt;
      &lt;Bundle&gt;
<strong class="highlight">        &lt;BootstrapperApplicationRef Id=&quot;WixStandardBootstrapperApplication.RtfLicense&quot; /&gt;</strong>
        &lt;Chain&gt;
        &lt;/Chain&gt;
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>

The WiX Standard Bootstrapper Application may not provide all functionality a specialized bundle requires so a custom bootstrapper application DLLs may be developed. The following example creates a bootstrapper application using a fictional &quot;ba.dll&quot;:

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;&gt;
      &lt;Bundle&gt;
<strong class="highlight">        &lt;BootstrapperApplication SourceFile=&quot;path\to\ba.dll&quot; /&gt;</strong>
        &lt;Chain&gt;
        &lt;/Chain&gt;
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>

Inside the <a href="wix_xsd_bootstrapperapplication.htm">&lt;BootstrapperApplication&gt;</a> element and <a href="wix_xsd_bootstrapperapplicationref.htm">&lt;BootstrapperApplicationRef&gt;</a> element, you may add additional payload files such as resources files that are required by the bootstrapper application DLL as follows:

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;&gt;
      &lt;Bundle&gt;
        &lt;BootstrapperApplication SourceFile=&quot;path\to\ba.dll&quot;&gt;
<strong class="highlight">          &lt;Payload SourceFile=&quot;path\to\en-us\resources.dll&quot;/&gt;
          &lt;PayloadGroupRef Id=&quot;ResourceGroupforJapanese&quot;/&gt;</strong>
        &lt;/BootstrapperApplication&gt;
        &lt;Chain&gt;
        &lt;/Chain&gt;
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>

This example references a payload file that is on the local machine named resources.dll as well as a group of payload files that are defined in a <a href="wix_xsd_payloadgroup.htm">&lt;PayloadGroup&gt;</a> element inside a <a href="wix_xsd_fragment.htm">&lt;Fragment&gt;</a> elsewhere.

The next step is to <a href="authoring_bundle_package_manifest.htm">add installation packages to the chain</a>.
