---
title: Building WiX
layout: documentation
after: wixdev_getting_started
---

  <h1>Building WiX</h1>

  <p>Simply run <kbd>msbuild</kbd> from the wix directory; if you have Visual Studio 2012 installed it may be necessary to add <kbd>/p:VisualStudioVersion="11.0"</kbd>. This will build debug bits into the "build" directory by default. To build release bits, run <kbd>msbuild /p:Configuration=Release</kbd>.</p>

  <p>In order to fully build WiX, you must have the following Frameworks and SDKs installed:</p>

  <ul>
    <li>The following components from the <a href="http://www.microsoft.com/downloads/details.aspx?FamilyId=E6E1C3DF-A74F-4207-8586-711EBE331CDC" target="_blank">Windows SDK for Windows Server 2008 and .NET Framework 3.5</a>, Visual Studio 2008, Microsoft Windows 7 SDK, and/or Visual Studio 2010 and/or Visual Studio 2012:</li>

    <li style="list-style: none; display: inline">
      <ul>
        <li>x86 and x64 compilers, headers and libraries</li>

        <li>IA64 headers and libraries are optional, but they are necessary for IA64 custom action support</li>

        <li>If you want to be able to build optimized IA64 binaries, you'll need both the Windows SDK for Windows Server 2008 and .NET Framework 3.5 SDK <em>AND</em> Visual Studio 2008 installed; or the Microsoft Windows 7 SDK <em>AND</em> Visual Studio 2010.</li>

        <li><a href="http://msdn2.microsoft.com/library/ms670169.aspx" target="_blank">HTML Help SDK 1.4</a> or higher [installed to Program Files or Program Files (x86)]</li>
      </ul>
    </li>
  </ul>

  <p>To build Sconce and Votive, you must have the following SDKs installed:</p>

  <ul>
    <li><a href="http://wix.codeplex.com/SourceControl/BrowseLatest" target="_blank">Visual Studio 2005 SDK Version 4.0</a> (choose devbundle branch and browse to src\packages\VS2005SDK)</li>

    <li><a href="http://www.microsoft.com/en-us/download/details.aspx?id=21827" target="_blank">Visual Studio 2008 SDK</a></li>

    <li><a href="http://www.microsoft.com/en-us/download/details.aspx?id=21835" target="_blank">Visual Studio 2010 SP1 SDK</a></li>

    <li><a href="http://www.microsoft.com/en-us/download/details.aspx?id=30668" target="_blank">Visual Studio 2012 SDK</a></li>
  </ul>

  <p>More information about the Visual Studio SDK can be found at the <a href="http://msdn.microsoft.com/en-gb/vstudio/vextend.aspx" target="_blank">Visual Studio Extensibility Center</a>.</p>

  <p>To install Votive on Visual Studio 2005, 2008, 2010 or 2012, you must have the Standard Edition or higher.</p>

  <p>To build DTF help files, you need the following tools:</p>

  <ul>
    <li><a href="http://sandcastle.codeplex.com/releases/view/13873">Sandcastle May 2008 Release</a></li>

    <li><a href="http://shfb.codeplex.com/releases/view/29710">Sandcastle Help File Builder 1.8.0.3</a></li>
  </ul>

  <p>The DTF help build process looks for these tools in an "external" directory parallel to the WiX "src" directory:</p>

  <ul>
    <li>Sandcastle: external\Sandcastle</li>

    <li>Sandcastle Help File Builder: external\SandcastleBuilder</li>
  </ul>

  <p>To create a build that can be installed on different machines, create a new strong name key pair and point OFFICIAL_WIX_BUILD to it:</p>

  <kbd>sn -k wix.snk</kbd><br>
  <kbd>sn -p wix.snk wix.pub</kbd><br>
  <kbd>sn -tp wix.pub</kbd><br>

  <p>Copy the public key and add new InternalsVisibleTo lines in:</p>

  <ul>
    <li>src\Votive\sconce\Properties\AssemblyInfo.cs</li>
    <li>src\Votive\sdk_vs2010\common\source\csharp\project\AssemblyInfo.cs</li>
    <li>src\Votive\sdk_vs2010\common\source\csharp\project\attributes.cs</li>
  </ul>

Then run the build:

<kbd>msbuild /p:Configuration=Release /p:OFFICIAL_WIX_BUILD=C:\wix.snk</kbd>
