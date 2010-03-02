// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//
//

Implement Core Dump Server and ECOM plugins.


- Component Summary

-- Core Dump Server : Server that manages the connection to the Debug Security Server (DSS), 
loading plugins and controlling the creation of core dumps. 

A stub SIS file is also provided to allow installations after ROM build.

Two libraries are also provided, coredumpinterface and cdssupport.
The coredumpinterface library is a standard Symbian interface to the Core Dump Server.
The cdssupport library contains common code used by UI clients, the server and the ECOM plugins.


-- Plugins : ECOM plugins that implement the creation of core dumps in differen formats (formatters) 
and the saving of core dumps to different medium/media (writers). 

By default there are two formatters (Symbian ELF, D_EXC) and one writer (File Writer).

-- User Interfaces : User interfaces to the Core Dump Server (CDS). 
By default there are two: TechView GUI and core dump script.


- External Requirements

-- Debug Security Server
The Core Dump Server uses the DSS, which is available in the MCL from M04250. 

-- Target Hardware
Since the DSS does not run on the emulator, the CDS cannot run on the emulator either. 
It has been tested on H2 and H4 boards.


- Components Details

-- Core Dump Server 
IBY : server\src\coredumpserver.iby
Source : server

-- Core Dump Server STUB SIS file
IBY : server\src\coredumpserverstub.iby
Source : server\plugins


-- Plugins

--- Symbian ELF

Source : plugins/formatters/symbianelf/group/symbianelfformatter.iby

IBY : plugins\formatters\symbianelf\group\symbianelfformatter.iby

Symbian ELF V1 : plugins/formatters/symbianelf/src/
Symbian ELF V2 : plugins/formatters/symbianelf/srcv2/

--- D_EXC

IBY : plugins\formatters\dexc\group\dexcformatter.iby
Source : plugins\formatters\dexc

--- File Writer
IBY : plugins\writers\file\group\filewriter.iby
Source : plugins\writers\file

--- UTrace
IBY : plugins\formatters\utrace\group\utraceformatter.iby
Source : plugins\formatters\utrace


-- User Interfaces

--- TechView UI
IBY : ui\coredumpui\group\coredumpui.iby
Source : \ui\coredumpui

--- Core Dump Script UI
IBY : ui\scriptconfig\coredumpscriptconfig.iby
Source : ui\scriptconfig



- Component Build Options

Each of the abovementioned components can be built separately, and thus a ROM can contain a mixture of 
components. To obtain the default set of components without the STUB SIS file:
 \epoc32\rom>buildrom h2 techview coredumpserver.iby symbianelfformatter.iby filewriter.iby dexcformatter.iby coredumpui.iby

The STUB SIS file is required to upgrade and/or add plugins after ROM build. 

Both the plugins and the server can be installed after ROM build.


- Basic Usage

-- The TechView UI 

The TechView UI allows the user to:
 . Display a list of running processes and threads and choose which ones to observe for crashes 
 . Display a list of plugins and choose which ones to load
 . Configure the CDS options:
    - Core dump trigger : Hardware Exception and/or Kill Event 
    - Actions to carry out upon crash event 
    - Actions to carry out after core dump is complete


-- D_EXC Output
The D_EXC formatter will, by default, generate the same output in the same location (C: drive) as the original D_EXC tool. 


-- Symbian ELF Output
The Symbian ELF output is documented in 
  plugins\formatters\symbianelf\documentation\sgl_ts0028_027_symbian_core_dump_file_format

The Symbian ELF formatter will, by default, generate all the optional output. 
By default the core dump location is the E: drive. 
The name of the file contains an optional user supplied string, followed by the lower 32 
bits of the crash timestamp. If the user does not supply a file name to the file writer plugin, the
default base name of the core dump file is "core<LSW time of crash>".

Version 1.01.002
=================
Made by Stephen Roberts (17/11/2009)

1.	DPDEF142882: Updated so crash files don't overwrite old ones if configured to use the same name.
2.	DPDEF142849: Memory Leak Found when CDS exits after processing System Crash.


Version 1.01.001
=================
Made by Sakhya Ghosh (22/10/2009)

1. 	Updated Core Dump Server and SELF Formatter V2 with the new RProperty Signaling mechanism. A new RProperty added to 
notify the clients of Core Dump Server about the core dump ELF media name. 

