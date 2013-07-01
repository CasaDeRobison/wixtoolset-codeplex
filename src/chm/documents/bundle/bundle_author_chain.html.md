---
title: Chain Packages into a Bundle
layout: documentation
after: bundle_define_searches
---
# Chain Packages into a Bundle

To add a chained package, you can do so either by putting the package definition directly under the <a href="wix_xsd_chain.htm">&lt;Chain&gt;</a> element or by doing a <a href="wix_xsd_packagegroupref.htm">&lt;PackageGroupRef&gt;</a> inside <a href="wix_xsd_chain.htm">&lt;Chain&gt;</a> to reference a shared package definition.

Here's an example of having the definition directly under <a href="wix_xsd_chain.htm">&lt;Chain&gt;</a>:

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;&gt;
      &lt;Bundle&gt;
        &lt;BootstrapperApplicationRef Id=&quot;WixStandardBootstrapperApplication.RtfLicense&quot; /&gt;

        <strong class="highlight">&lt;Chain&gt;
            &lt;ExePackage
              SourceFile=&quot;path\to\MyPackage.exe&quot;
              DownloadUrl=&quot;http://example.com/?mypackage.exe&quot;
              InstallCommand=&quot;/q /ACTION=Install&quot;
              RepairCommand=&quot;/q ACTION=Repair /hideconsole&quot;/&gt;
        &lt;/Chain&gt;</strong>
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>

<p>Here's an example of referencing a shared package definition:</p>

<pre>    &lt;?xml version=&quot;1.0&quot;&gt;
    &lt;Wix xmlns=&quot;http://schemas.microsoft.com/wix/2006/wi&quot;&gt;
      &lt;Bundle&gt;
        &lt;BootstrapperApplicationRef Id=&quot;WixStandardBootstrapperApplication.RtfLicense&quot; /&gt;

        <strong class="highlight">&lt;Chain&gt;
            &lt;PackageGroupRef Id=&quot;MyPackage&quot; /&gt;
        &lt;/Chain&gt;</strong>
      &lt;/Bundle&gt;
    &lt;/Wix&gt;</pre>
