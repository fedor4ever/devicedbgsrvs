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

REM EXECUTE STUFF*******************************************************************
ECHO RUN THE CRASHAPP

call testdriver run -p armv5 -b udeb -s file:/O:\perforce_root\1700\coredumpserver\personal\test\automatictests\tcds_kernel\group\tcds_kernel.driver#testCDS.testCrashSystem.testCrashPrefetchAbort
call sleepandrestart.bat

ECHO Run the process crash
REM RUN the processcrash script
call testdriver run -p armv5 -b udeb -s file:/O:\perforce_root\1700\coredumpserver\personal\test\automatictests\tcds_kernel\group\tcds_kernel.driver#testCDS.testProcessCrash.testProcessHeader

ECHO DONE
REM ************************************************************************************