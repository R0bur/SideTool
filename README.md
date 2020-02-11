#SideTool

##Software requirements

Running the SideTool requires Microsoft Windows operating system (version 2000 or later).

Compiling the SideTool requires:
- Digital Mars C/C++ compiler;
- Windows® Server 2003 SP1 Platform SDK.

##Configuring the build environment

1. Install *[Windows® Server 2003 SP1 Platform SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6510)*.
2. Extract *[Digital Mars C/C++ Compiler Version 8.57](http://ftp.digitalmars.com/Digital_Mars_C++/Patch/dm857c.zip)* to the folder C:\.
3. Modify the configuration file *C:\DM\bin\sc.ini*:
	...
	INCLUDE="%ProgramFiles%\Microsoft Platform SDK\Include";%@P%\..\include";"%@P%\..\mfc\include";%INCLUDE%
	...
4. Create the folder *C:\WORK\SideTool* with source files from *[this](https://github.com/R0bur/SideTool)* repository.
5. Create the folder *C:\WORK\IADF* with source files from the *[IADF](https://github.com/R0bur/IADF)* repository.

##Compiling

Open the Windows command line *cmd.exe* and type the commands:

	PATH C:\DM\bin;%PATH%
	CD \WORK\SideTool
	make.exe

The resulting file *sidetool.exe* will appear in the folder *C:\WORK\SideTool*.
It can be placed anywhere in the file system and executed.
