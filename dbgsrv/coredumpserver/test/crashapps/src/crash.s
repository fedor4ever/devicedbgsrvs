;
; Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
; All rights reserved.
; This component and the accompanying materials are made available
; under the terms of "Eclipse Public License v1.0"
; which accompanies this distribution, and is available
; at the URL "http://www.eclipse.org/legal/epl-v10.html".
;
; Initial Contributors:
; Nokia Corporation - initial contribution.
;
; Contributors:
;
; Description: 
;
;

        AREA |crash$$Code|, CODE, READONLY, ALIGN=6

	CODE32

	; ARM tests
	
; 
; Non-PC modifying
;
	EXPORT InfLoop
	EXPORT SetRegs
	EXPORT SetUpperRegs
	
InfLoop
	bl		InfLoop

SetRegs
	mov r7, #0xF000
	mov r8, #0xEA
	bx  lr
	
SetUpperRegs
	mov r8, #0xF000
	mov r11, #0xEA
	bx  lr

	END

; End of file - crash.s
