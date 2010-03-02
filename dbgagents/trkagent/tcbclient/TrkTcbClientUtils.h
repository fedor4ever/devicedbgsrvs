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

#ifndef __TrkTcbClientUtils_H__
#define __TrkTcbClientUtils_H__

// System includes
#include <e32base.h>


//
//  TrkTcbClientUtils (header)
//
class TrkTcbClientUtils
{
public:										// ENUMERATIONS
	/** Panic numbers associated with the TRKTCBCLI panic category. */
	enum TTrkTcbClientPanic
	{
		ETrkTcbClientPanicArgument			= 0,
		ETrkTcbClientPanicCancel			= 1,
		ETrkTcbClientPanicLast
	};
	/** Panic numbers associated with the TRKTCBCLIFAULT panic category. */
	enum TTrkTcbClientFault
	{
		ETrkTcbClientFaultLogic					= 0,
		ETrkTcbClientFaultData					= 1,
		ETrkTcbClientFaultCouldNotLoadServer	= 2,
		ETrkTcbClientFaultDebugFuncError		= 3,
		ETrkTcbClientFaultLast
	};

public:

	static void	Panic(TTrkTcbClientPanic aPanic);

	static void	Fault(TTrkTcbClientFault aFault);

	static TInt	StartTrkTcbServer();
};

#endif //__TrkTcbClientUtils_H__

