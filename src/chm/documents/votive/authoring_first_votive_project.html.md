---
title: Creating a simple setup
layout: documentation
after: votive_project_template_default
---
# Creating a Simple Setup

In this tutorial, we will create a C# Windows Form Application and then use WiX to create an installer for the application.

<h2>Step 1: Create the C# Windows Form Application </h2>

<ol>
  <li>Click <b>File</b>, then select <b>New</b>, then select <b>Project</b>.</li>

  <li>Choose the <b>Visual C#</b> node in the <b>Project Types</b> tree, then select <b>Windows Forms Application</b>.</li>

  <li>Name your application &quot;MyApplication&quot; and press OK.</li>

</ol>

  <h2>Step 2: Create the installer for the application</h2>

<ol>
  <li>Click <b>File</b>, then click <b>New</b>, then click <b>Project.</b></li>

  <li>Choose the <b>Windows Installer XML</b> node in the <b>Project types</b> tree, then select <b>WiX Project</b></li>
    <li>Name your project &quot;MySetup&quot; and press OK.</li>
    <li>In the <b>MySetup</b> project, right-click on the <b>References</b> node and choose <b>Add Reference...</b>. </li>
    <li>Navigate to the <b>Projects</b> tab, click on the <b>MyApplication</b> project, and click the <b>Add</b> 
        button, and then press OK. </li>
    <li>Find the comment that says:
<pre>
  &lt;!-- TODO: Insert your files, registry keys, and other resources here. --&gt;</pre>
<p>Delete that line and replace it with the following lines of code:</p>
<pre>
  &lt;File Source="$(var.MyApplication.TargetPath)" /&gt;
  </pre>
    </li>
    <li>Build the WiX project.</li>
</ol>

  <p>
      That&#39;s it! Now you have a working installer that installs and uninstalls the 
      application.</p>

If you type that code into the editor (instead of copying and pasting from this example) you will notice that IntelliSense picks up the valid elements and attributes. IntelliSense with WiX in Visual Studio can save you significant amounts of typing and time when searching for the name of the elements or attributes as you become more comfortable with the WiX language.

The line of code you added instructs the WiX toolset to add a file resource to the setup package. The Source attribute specifies where to find the file for packaging during the build. Rather than hard-code values for these attributes into our source code, we use the WiX preprocessor variables that are passed to the WiX compiler. More information about using preprocessor variables, including a table of all supported values, can be found in the <a href='votive_project_references.htm'>Adding Project References topic</a>.
