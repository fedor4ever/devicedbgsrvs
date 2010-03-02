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
echo		Usage:	DT-coredump-monitor-002.bat [/desc] [/?]
echo.
echo			Display Usage							/?
echo			Display Description						/d
exit /B n

:DisplayTestInfo
echo.
echo		Test Description:
echo		Ensure build time configuration parameters are correct
exit /B n

:Main
call:CleanFlashPartition
call:CrashBoardWithTrace
call:Sleep
call:Validate
exit /B n

:CleanFlashPartition
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.processcrash.testListAndDeleteAllCrashes
goto:eof

:CrashBoardWithTrace
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}\tcds_kernel\group\tcds_kernel.driver#system_crash.crash_system.crash_data_read_with_trace
goto:eof

:Sleep
call ..\util\sleepandrestart.bat
goto:eof

:Validate
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.coredump-monitor.coredump-monitor-009
goto:eof
