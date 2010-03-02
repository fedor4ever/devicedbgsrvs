@echo OFF

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

:while
if "%1"=="" goto Main
if "%1"=="/d" goto DisplayTestInfo
if "%1"=="/?" goto DisplayHelp
shift
goto while

:DisplayHelp
echo.
echo	Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
echo	All rights reserved.
echo.
echo		Usage:	DT-coredump-monitor-005.bat [/desc] [/?]
echo.
echo			Display Usage							/?
echo			Display Description						/d
exit /B n

:DisplayTestInfo
echo.
echo		Test Description:
echo		Untrusted clients should not be able to view crash logger configuration parameters.
exit /B n

:Main
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}\tcds_kernel\group\tcds_kernel.driver#system_crash.coredump-monitor.coredump-monitor-005
exit /B n

