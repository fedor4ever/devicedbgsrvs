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
rem

@ECHO OFF
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO Wait for a minute for the crash to happen
ping -n 85 127.0.0.1 >NUL
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .
ECHO .

call hardwareswitch off
ECHO Board Switched OFF
ping -n 11 127.0.0.1 >NUL
ECHO ---------------------
ECHO Board Switched ON
call hardwareswitch on
ping -n 37 127.0.0.1 >NUL
