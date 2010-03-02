// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "coredumpui.h"
#include <eikstart.h>

//             The entry point for the application code. It creates
//             an instance of the CApaApplication derived
//             class, CResourceApplication.
//


LOCAL_C CApaApplication* NewApplication()
	{
	return new CResourceApplication;
	}

GLDEF_C TInt E32Main()
	{
	__UHEAP_MARK;
	TInt result = EikStart::RunApplication(NewApplication);
	__UHEAP_MARKEND;
	return result;
	}
