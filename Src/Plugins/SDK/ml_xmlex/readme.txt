ml_xmlex.dll - Readme

This is an example of implementation of Winamp's built in XML parser.  Please make note that Winamp's XML service will parse all syntactically correct xml.

XML file form for this plugin is exemplified in xmltest.xml, which is provided with the source for this plugin.  
XML files should be in the format of 

	<Library>
		<Song Filename="" Artist="" Title="" />        (or)
		<Song Filename="" Artist"" Title=""></Song>
	</Library> 

with the extension .xml for this plugin to read it correctly.  