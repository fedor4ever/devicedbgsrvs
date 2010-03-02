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


#include "trkappui.h"

void CTrkAppUi::ConstructL()
{
    BaseConstructL();
    TInt err = iTrkSession.ConnectToServer();
   	iAppView = CTrkAppView::NewL(ClientRect(), iTrkSession);
}

CTrkAppUi::~CTrkAppUi()
{
	delete iAppView;
	iTrkSession.ShutDownServer();
	iTrkSession.Close();
}

void CTrkAppUi::HandleCommandL(TInt aCommand)
{
	switch (aCommand)
	{
		case EEikCmdExit:
		{
			Exit();
			break;
		}
	}
}


