---
title: Adding to the WiX Documentation
layout: documentation
after: votive_development
---

# Adding to the WiX Documentation

WiX documentation is compiled into the file WiX.chm as a part of the WiX build process. The source files for help are located in the wix\src\chm directory.

## What the WiX help compiler does

The WiX help compiler does the following:

* Parses the file TOC.xml to determine the table of contents to construct in the CHM file and determine what HTML files to include in the CHM file.
* Includes all the .htm files listed in the project file in the list of documentation to build into the CHM
* Parses .xsd schema files referenced in TOC.xml and generates help topics for the attributes and elements that are annotated in the .xsd files.

## How to add a new topic to WiX.chm

Adding a new topic to WiX.chm requires the following steps:

* Add a new HTML file with the contents of the new topic to the WiX source tree under src\chm\html.
* Add an entry for the new HTML to the src\chm\chm.proj file.
* Add any relevant images to the src\chm\imgs\ sub-directory in the WiX source tree.
* Add an entry for the new images to the src\chm\chm.proj file.
* Add a reference to the new HTML file to TOC.xml in the desired location in the table of contents.

Help topics may contain links to external Web pages, and may also contain relative links to other help topics or attributes or elements defined in one of the .xsd schema files.

To build the new content type <i>msbuild</i> from the command line in the src\chm directory.
