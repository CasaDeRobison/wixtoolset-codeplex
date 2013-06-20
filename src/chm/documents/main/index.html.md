---
title: Windows Installer XML (WiX)
layout: default
---
# Introduction to Windows Installer XML (WiX) toolset

## What is WiX?

WiX is a set of tools that allows you to create Windows Installer-based 
deployment packages for your application. The WiX toolset is based on a 
declarative XML authoring model. You can use WiX on the command line by using the WiX tools 
or MSBuild. In addition, there is also a WiX Visual Studio plug-in that supports 
VS2005, VS2008, and VS2010. The WiX toolset supports building the following types of 
Windows Installer files:

* Installer (.msi)
* Patches (.msp)
* Merge Modules (.msm)
* Transforms (.mst)

WiX supports a broad spectrum of Windows Installer features. In addition, WiX 
also offers a set of built-in custom actions that can be used and incorporated 
in Windows Installer packages. The custom actions are offered in a set of WiX 
extensions. Some common WiX extensions include support for Internet Information 
System (IIS), Structured Query Language (SQL), the .NET Framework, Visual 
Studio, and Windows etc.

## How does WiX work?

The WiX source code is written in XML format with a .wxs file extension. 
The WiX tools follow the traditional compile and link model used to create 
executables from source code. At build time, the WiX source files are validated 
against the core WiX schema, then processed by a preprocessor, compiler, and 
linker to create the final result. There are a set of WiX tools that can be used 
to produce different output types.

For a complete list of file types and tools in WiX, see the <a href='files.htm'>File Types</a> 
and the [List of Tools](alltools.htm) sections.

See the following topics for more detailed information:

* <a href="~/overview/index.html">Fundamental Tools and Concepts</a>
* <a href="authoring_bundle_intro.htm">Creating Installation Package Bundles</a>
* <a href="votive.htm">Working in Visual Studio</a>
* <a href="msbuild.htm">Working with MSBuild</a>
* <a href="toc.htm">How To Guides</a>
* <a href="standard_customactions.htm">Standard Custom Actions</a>
* <a href="patching.htm">Creating an Installation Patch</a>
* <a href="schema_index.htm">WiX Schema Reference</a>
* <a href="wixdev.htm">Developing for WiX</a>
