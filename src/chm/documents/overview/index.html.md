---
title: Tools and Concepts
layout: default
after: ~/main/
---
# Tools and Concepts

The WiX toolset is tightly coupled with the Windows Installer technology. In 
order to fully utilize the features in WiX, you must be familiar with the 
Windows Installer concepts. This section assumes you have a working knowledge of 
the Windows Installer database format. For information on Windows Installer, see <a href="msi_useful_links.htm">Useful Windows Installer Information</a>.

## WiX File Types
There is a set of tools that WiX offers to fulfill the needs of building Windows 
Installer-based packages. Each tool outputs a type of file that can be consumed 
as inputs of another tool. After processing through the appropriate tools, the 
final installer is produced.

To get familiar with the WiX file types, see <a href="files.htm">File Types</a>.

## WiX Tools

Once you are familiar with the file types, see how the file types are produced 
by what WiX tools by visiting <a href="AllTools.htm">List of Tools</a>. For a graphical view of the WiX tools and how they interact with each other, see
<a href="tools.htm">WiX Toolset Diagram</a>.

## WiX Schema

The core WiX schema is a close mirror with the MSI tables. For helpful hints on how the WiX schema maps to MSI tables, see <a href="msitowix.htm">MSI Tables to WiX Schema</a>.
