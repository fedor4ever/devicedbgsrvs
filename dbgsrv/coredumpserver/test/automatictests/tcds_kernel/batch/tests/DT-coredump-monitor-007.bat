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

time /t
ECHO *********************************************
ECHO DT-coredump-monitor-007.bat
ECHO *********************************************

ECHO TO CHECK FOR NEWEST CRASH

ECHO *********************************************
ECHO CLEAN FLASH PARTITION
ECHO *********************************************
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}\tcds_kernel\group\tcds_kernel.driver#system_crash.flash_utility.clean_partition

ECHO *********************************************
ECHO CONFIGURING SCM TO HOLD MINIMUM INFORMATION
ECHO *********************************************
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.processcrash.testSCMZeroConfig

ECHO *********************************************
ECHO CRASHING BOARD FOR THREE TIMES
ECHO *********************************************

ECHO *********************************************
ECHO CRASH BOARD PREFETCH ABORT
ECHO *********************************************
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.crash_system.crash_prefetch_abort

ECHO *********************************************
ECHO SLEEP
ECHO *********************************************
call ..\util\sleepandrestart.bat

ECHO *********************************************
ECHO CRASH BOARD DATA READ ERROR
ECHO *********************************************
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.crash_system.crash_data_read

ECHO *********************************************
ECHO SLEEP
ECHO *********************************************
call ..\util\sleepandrestart.bat

ECHO *********************************************
ECHO CRASH BOARD DATA WRITE ERROR
ECHO *********************************************
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.crash_system.crash_data_write

ECHO *********************************************
ECHO SLEEP
ECHO *********************************************
call ..\util\sleepandrestart.bat

ECHO *********************************************
ECHO CHECK FOR FOUR SELF FILES TO BE GENERATED
ECHO *********************************************
call testdriver run -p armv5 -b udeb -s file:/${sourceroot}/tcds_kernel/group/tcds_kernel.driver#system_crash.coredump-monitor.coredump-monitor-007
time /t
