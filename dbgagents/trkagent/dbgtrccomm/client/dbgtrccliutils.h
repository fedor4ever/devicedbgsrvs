/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

#ifndef __DbgTrcCliUtils_H__
#define __DbgTrcCliUtils_H__

// System includes
#include <e32base.h>


//
//  DbgTrcCliUtils (header)
//
class DbgTrcCliUtils
{
public:										// ENUMERATIONS
	/** Panic numbers associated with the DBTRCCLI panic category. */
	enum TDbgTrcCliPanic
	{
		EDbgTrcCliPanicArgument			= 0,
		EDbgTrcCliPanicCancel			= 1,
		EDbgTrcCliPanicLast
	};
	/** Panic numbers associated with the DBGTRCCLIFAULT panic category. */
	enum TDbgTrcCliFault
	{
		EDbgTrcCliFaultLogic				= 0,
		EDbgTrcCliFaultData					= 1,
		EDbgTrcCliFaultCouldNotLoadServer	= 2,
		EDbgTrcCliFaultDebugFuncError		= 3,
		EDbgTrcCliFaultLast
	};

public:

	static void	Panic(TDbgTrcCliPanic aPanic);

	static void	Fault(TDbgTrcCliFault aFault);

	static TInt	StartDbgTrcServer();
};

#endif //__DbgTrcCliUtils_H__

