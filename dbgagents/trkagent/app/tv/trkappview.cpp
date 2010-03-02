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

#include <viewcli.h>
#include "trkappview.h"
#include "trkservereventlistener.h"
#include "trkdebugstatelistener.h"
#include "trkconnstatelistener.h"

CTrkAppView::CTrkAppView(RTrkSrvCliSession& aTrkSession)
	: iVwsSession(NULL),
	  iTrkSession(aTrkSession),
	  iMajorVersion(0),
	  iMinorVersion(0),
	  iMajorAPIVersion(0),
	  iMinorAPIVersion(0),
	  iError(KErrNone)
{
}

CTrkAppView* CTrkAppView::NewL(const TRect& aRect,  RTrkSrvCliSession& aTrkSession)
{
	CTrkAppView* self = new(ELeave) CTrkAppView(aTrkSession);
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	CleanupStack::Pop();
	return self;
}

CTrkAppView::~CTrkAppView()
{
    // Disconnect here
	iTrkSession.DisConnect();
	
	if (iVwsSession)
	{
		iVwsSession->EnableServerEventTimeOut(ETrue);
		delete iVwsSession;
	}
}

void CTrkAppView::ConstructL(const TRect& aRect)
{
	CreateWindowL();
	SetRect(aRect);
	ActivateL();
	
	iVwsSession = CVwsSessionWrapper::NewL();
	
    iTrkSession.GetTrkVersion(iMajorVersion, iMinorVersion, iMajorAPIVersion, iMinorAPIVersion, iBuildNumber); 
    // Ideally we need to connect here, but when the TRK server is started, connect automatically happens.
    // So we don't need to explicitly connect here. If the server is changed not to connect automatically at startup
    // then we need to connect here.
    //iTrkSession.Connect();
}

void CTrkAppView::Draw(const TRect& /*aRect*/) const
{
    TTrkConnStatus iConnStatus;
	CWindowGc& gc = SystemGc();

	TRect      drawRect = Rect();
	
	gc.Clear();

	const CFont *font = iEikonEnv->AnnotationFont();
	gc.UseFont(font);
	
	TBuf<KMaxPath> buf;

	_LIT(KTrkWelcome,"Welcome to Trk for Symbian OS");
	_LIT(KTrkVersion,"Version %d.%d.%d");
	_LIT(KTrkAPIVersion,"Implementing Trk API version %d.%d");

	TInt vert = 15;
	
	gc.DrawText(KTrkWelcome, drawRect, vert);
	vert += font->HeightInPixels();
	buf.Format(KTrkVersion, iMajorVersion, iMinorVersion, iBuildNumber);
	gc.DrawText(buf, drawRect, vert);
	vert += font->HeightInPixels();
	buf.Format(KTrkAPIVersion, iMajorAPIVersion, iMinorAPIVersion);
	gc.DrawText(buf, drawRect, vert);
	vert += font->HeightInPixels()*2;

	iTrkSession.GetDebugConnStatus(iConnStatus, buf);
	
	_LIT(KLfCr, "\r\n");
	
	TPtrC ptr(buf);
	TInt pos = 0;
	TInt lineStart = 0;
	TInt length = 0;
	
	while (KErrNotFound != (pos = ptr.Find(KLfCr)))
	{
		ptr.Set(&buf[lineStart], pos);
		lineStart = lineStart + pos + 2;
		gc.DrawText(ptr, drawRect, vert);
		vert += font->HeightInPixels();
		length = buf.Length() - lineStart;
		if (length > 0)
		{
			ptr.Set(&buf[lineStart], length);
		}
	}

	gc.DiscardFont();

	iVwsSession->EnableServerEventTimeOut(EFalse);	
}
