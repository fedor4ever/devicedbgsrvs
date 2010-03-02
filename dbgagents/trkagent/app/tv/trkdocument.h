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

#ifndef __TRKDOCUMENT_H
#define __TRKDOCUMENT_H

#include <eikdoc.h>

//
//
// CTrkDocument
//
//
class CTrkDocument : public CEikDocument
{
public:

	static CTrkDocument* NewL(CEikApplication& aApp);
	CTrkDocument(CEikApplication& aApp);
	void ConstructL();

private: 

	CEikAppUi* CreateAppUiL();
};

#endif //__TRKDOCUMENT_H
