---
title: Extensions
layout: documentation
after: extension_development_intro
---

  <h1>Extensions</h1>

  <p>WiX supports the following classes of extensions:</p>

  <ul>
    <li><b>Binder Extensions</b> allow clients to modify the behavior of the Binder.</li>

    <li><b>BinderFileManager Extensions</b> allow clients to simply modify the file source resolution and file differencing features of the Binder.</li>

    <li><b>Compiler Extensions</b> allow clients to custom compile authored XML into internal table representation before it is written to binary form.</li>

    <li><b>Decompiler Extensions</b> allow clients to decompile custom tables into XML.</li>

    <li><b>Harvester Extensions</b> allow clients to modify the behavior of the Harvester.</li>

    <li><b>Inspector Extensions</b> allow clients to inspect source, intermediate, and output documents at various times during the build to validate business rules as early as possible.</li>

    <li><b>Mutator Extensions</b> allow clients to modify the behavior of the Mutator.</li>

    <li><b>Preprocessor Extensions</b> allow clients to modify authoring files before they are processed by the compiler.</li>

    <li><b>Unbinder Extensions</b> allow clients to modify the behavior of the Unbinder.</li>

    <li><b>Validator Extensions</b> allow clients to process ICE validation messages. By default, ICE validation messages are output to the console.</li>

    <li><b>WixBinder Extensions</b> allow clients to completely change the Binder to, for example, create different output formats from WiX authoring.</li>
  </ul>

For information on how to use WiX extensions on the command line or inside the Visual Studio IDE, please visit the <a href='extension_usage_introduction.htm'>Using WiX extensions</a> topic.

For information on how to use localized WiX extensions, please visit the <a href='localized_extensions.htm'>Localized extensions</a> topic.
