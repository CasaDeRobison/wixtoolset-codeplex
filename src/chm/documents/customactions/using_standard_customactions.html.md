---
title: Using Standard Custom Actions
layout: documentation
---

# Using Standard Custom Actions

Custom actions add the ability to install and configure many new types of resources. Each of these resource types has one or more elements that allow you to install them with your MSI package. The only things you need to do are understand the appropriate elements for the resources you want to install and set the required attributes on these elements. The elements need to be prefixed with the appropriate namespace for the WiX extension they are defined in. You must pass the full path to the extension DLL as part of the command lines to the compiler and linker so they automatically add the all of the proper error messages, custom action records, and binary records into your final MSI.

## Example

First, let's try an example that creates a user account when the MSI is installed. This functionality is defined in WixUtilExtension.dll and exposed to the user as the &lt;User&gt; element.

<pre>
&lt;Wix xmlns='http://schemas.microsoft.com/wix/2006/wi' xmlns:util='http://schemas.microsoft.com/wix/UtilExtension' &gt;
    &lt;Product Id='PutGuidHere' Name='TestUserProduct' Language='1033' Version='0.0.0.0'&gt;
        &lt;Package Id='PUT-GUID-HERE' Description='Test User Package' InstallerVersion='200' Compressed='yes' /&gt;
            &lt;Directory Id='TARGETDIR' Name='SourceDir'&gt;
                &lt;Component Id='TestUserProductComponent' Guid='PutGuidHere'&gt;
                    &lt;util:User Id='TEST_USER1' Name='testName1' Password='pa$$$$word'/&gt;
                &lt;/Component&gt;
        &lt;/Directory&gt;

        &lt;Feature Id='TestUserProductFeature' Title='Test User Product Feature' Level='1'&gt;
            &lt;ComponentRef Id='TestUserProductComponent' /&gt;
        &lt;/Feature&gt;
    &lt;/Product&gt;
&lt;/Wix&gt;
</pre>

<p>This is a simple example that will create a new user on the machine named "testName1" with the password "pa$$word" (the preprocessor replaces $$$$ with $$).</p>

<p>To build the MSI from this WiX authoring:</p>

<ol>
<li>Put the above code in a file named yourfile.wxs.</li>

<li>Replace the "PUT-GUID-HERE" attributes with real GUIDs.</li>

<li>Run 'candle.exe yourfile.wxs -ext %full path to WixUtilExtension.dll%'</li>

<li>Run 'light.exe yourfile.wixobj -ext %full path to WixUtilExtension.dll% &ndash;out yourfile.msi yourfile.wixout'</li>
</ol>

Now, use Orca to open up the resulting MSI and take a look at the Error table, the CustomAction table, and the Binary table. You will notice that all of the relevant data for managing users has been added into the MSI. This happened because you have done two key things:

1. You made use of a &lt;User&gt; element under a &lt;Component&gt; element. This indicates that a user is to be installed as part of the MSI package, and the WiX compiler automatically added the appropriate MSI table data used by the custom action.
2. You linked with the appropriate extension DLL (WixUtilExtension.dll). This caused the linker to automatically pull all of the relevant custom actions, error messages, and binary table rows into the MSI.
