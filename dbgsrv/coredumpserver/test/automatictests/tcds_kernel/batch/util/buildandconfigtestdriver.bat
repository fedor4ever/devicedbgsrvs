rem Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
rem All rights reserved.
rem This component and the accompanying materials are made available
rem under the terms of "Eclipse Public License v1.0"
rem which accompanies this distribution, and is available
rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
rem
rem Initial Contributors:
rem Nokia Corporation - initial contribution.
rem
rem Contributors:
rem
rem Description:
rem buildandconfiguretestdriver.bat
rem

@ECHO OFF
ECHO Configure Test Driver

call testdriver config --import ..\..\testdriver\td_proj_tefblock.xml

call testdriver import

call testdriver config

REM BUILD COMPONENTS**************************************************************
ECHO BUILD COMPONENTS

call testdriver build 