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

@echo off
setlocal
set starttime=%time%

REM Call our batch files first

call DT-coredump-server-001.bat
call DT-coredump-server-002.bat
call DT-coredump-server-003.bat

call DT-coredump-monitor-002.bat
call DT-coredump-monitor-004.bat
call DT-coredump-monitor-005.bat
call DT-coredump-monitor-007.bat
call DT-coredump-monitor-009.bat
call DT-coredump-monitor-010.bat
call DT-coredump-monitor-011.bat
call DT-coredump-monitor-012.bat
call DT-coredump-monitor-013.bat

call DT-formatters-SELF-001.bat
REM call DT-formatters-SELF-003.bat
call DT-formatters-SELF-005.bat
call DT-formatters-SELF-009.bat

call DT-coredump-performance-001.bat

echo Start Time=%starttime% - End Time=%time%