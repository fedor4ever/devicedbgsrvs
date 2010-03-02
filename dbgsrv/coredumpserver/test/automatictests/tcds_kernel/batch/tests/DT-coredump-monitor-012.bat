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

rem Parse the input parameters
set numCrashes=0

:while
if "%1"=="" goto Main
if "%1"=="/c" goto GetCrashNum
if "%1"=="/d" goto DisplayTestInfo
if "%1"=="/?" goto DisplayHelp
shift
goto while

:GetCrashNum
shift
set numCrashes=%1
goto:while

:DisplayHelp
echo.
echo	Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
echo	All rights reserved.
echo.
echo		Usage:	DT-coredump-monitor-012.bat /c=c [/desc] [/?]
echo.
echo			num is the number of crashes to test. Must be greater than 1	/c num
echo			Display Usage							/?
echo			Display Description						/d
exit /B n

:DisplayTestInfo
echo.
echo		Test Description:
echo		Test crash information in Multiple ELF file namely
echo			1 Crash Type
echo			2 Thread Name Validation
echo			3 Process ID
exit /B n

:Main
if %numCrashes% GTR 1 (
	call:CleanFlashPartition
	call:ConfigureScm
	for /L %%i in (1,1,%numCrashes%) do (
		call:CrashBoard
		call:Sleep
		)
	call:ValidateFiles
)else (
	call:DisplayHelp
)
set numCrashes=

exit /B n

:CleanFlashPartition
echo Cleaning flash partition...
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.processcrash.testListAndDeleteAllCrashes
goto:eof

:ConfigureScm
echo Configuring SCM...
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.processcrash.testSCMZeroConfig
goto:eof

:CrashBoard
echo Crashing board...
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.crash_system.crash_prefetch_abort
goto:eof

:Sleep
echo Sleeping...
call ..\util\sleepandrestart.bat
goto:eof

:ValidateFiles
echo Validating...
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.coredump-monitor.coredump-monitor-012
goto:eof
