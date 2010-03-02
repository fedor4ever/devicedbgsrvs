/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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

#ifdef __UI_FRAMEWORKS_V2__
#include <eikstart.h>
#endif

#include "trkapplication.h"


#ifdef __UI_FRAMEWORKS_V2__

	LOCAL_C CApaApplication* NewApplication()
		{
		return new CTrkApplication;
		}

	GLDEF_C TInt E32Main()
		{
		return EikStart::RunApplication(NewApplication);
		}

	#if defined(__WINS__) && !defined(EKA2)
	EXPORT_C TInt WinsMain(TDesC* aCmdLine)
		{
		return EikStart::RunApplication(NewApplication, aCmdLine);
		}

	GLDEF_C TInt E32Dll(TDllReason)
		{
		return KErrNone;
		}
	#endif //__WINS__ && !EKA2
	
#else //__UI_FRAMEWORKS_V2__

	EXPORT_C CApaApplication* NewApplication()
	{
		return new CTrkApplication;
	}

	#ifndef EKA2
	GLDEF_C TInt E32Dll(TDllReason)
	{
		return KErrNone;
	}
	#endif
	
#endif //__UI_FRAMEWORKS_V2__



