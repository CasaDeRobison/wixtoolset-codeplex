---
title: How To: Set Your Installer's Icon in Add/Remove Programs
layout: documentation
---
# How To: Set Your Installer&apos;s Icon in Add/Remove Programs
Windows Installer supports a standard property, <a href="http://msdn.microsoft.com/library/aa367593.aspx" target="_blank">ARPRPODUCTICON</a>, that controls the icon displayed in Add/Remove Programs for your application. To set this property you first need to include the icon in your installer using the [&lt;Icon&gt;](~/xsd/wix/icon.html) element, then set the property using the [&lt;Property&gt;](~/xsd/wix/property.html) element.

<pre>
<font size="2" color="#0000FF">&lt;<font size="2" color="#A31515">Icon</font> <font size="2" color="#FF0000">Id</font>=<font size="2">"</font>icon.ico<font size="2">"</font> <font size="2" color="#FF0000">SourceFile</font>=<font size="2">"MySourceFiles</font>\icon.ico<font size="2">"</font>/&gt;
&lt;</font><font size="2" color="#A31515">Property</font><font size="2" color="#0000FF"> </font><font size="2" color="#FF0000">Id</font><font size="2" color="#0000FF">=</font><font size="2">"</font><font size="2" color="#0000FF">ARPPRODUCTICON</font><font size="2">"</font><font size="2" color="#0000FF"> </font><font size="2" color="#FF0000">Value</font><font size="2" color="#0000FF">=</font><font size="2">"</font><font size="2" color="#0000FF">icon.ico</font><font size="2">"</font><font size="2" color="#0000FF"> /&gt;</font>
</pre>

These two elements can be placed anywhere in your WiX project under the Project element. There is no need to nest them in a Directory element. The Icon tag specifies the location of the icon on your source machine, and gives it a unique id for use later in the WiX project. The Property element sets the ARPPRODUCTION property to the id of the icon to use.