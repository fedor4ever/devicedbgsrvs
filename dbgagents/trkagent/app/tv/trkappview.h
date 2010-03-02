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

#ifndef __TRKAPPVIEW_H
#define __TRKAPPVIEW_H

#include <eikenv.h>
#include <eikmenup.h>
#include "trksrvclisession.h"

//
//
// CTrkAppView
//
//
class CTrkAppView : public CCoeControl
{
public:

	static CTrkAppView* NewL(const TRect& aRect,  RTrkSrvCliSession& aTrkSession);
	CTrkAppView(RTrkSrvCliSession& aTrkSession);
	~CTrkAppView();
    void ConstructL(const TRect& aRect);
    
private:

	void Draw(const TRect& /*aRect*/) const;
	

private:

	CVwsSessionWrapper* iVwsSession;
	RTrkSrvCliSession& iTrkSession;
	TInt iMajorVersion;
	TInt iMinorVersion;
	TInt iMajorAPIVersion;
	TInt iMinorAPIVersion;
	TInt iBuildNumber;
	TInt iError;
};

#endif //__TRKAPPVIEW_H

