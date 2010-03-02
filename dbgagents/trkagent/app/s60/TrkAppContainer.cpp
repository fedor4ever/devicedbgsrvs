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

// INCLUDE FILES
#include <barsread.h>       // TResourceReader
#include <gdi.h>

#include "TrkAppUi.h"
#include "TrkAppView.h"
#include "TrkConnData.h"
#include "TrkAppContainer.h"

// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CTrkAppContainer::CTrkAppContainer
// C++ constructor
// ---------------------------------------------------------------------------
//
CTrkAppContainer::CTrkAppContainer(CTrkAppView* aTrkAppView)
:iTrkAppView(aTrkAppView)
{
}


// ---------------------------------------------------------------------------
// CTrkAppContainer::ConstructL
// Symbian OS 2nd phase constructor
// ---------------------------------------------------------------------------
//
void CTrkAppContainer::ConstructL(const TRect& aRect)
{
	CreateWindowL();
	//Window().SetShadowDisabled(EFalse);

	SetBlank();

	SetRect(aRect);
	ActivateL(); 
}


// Destructor
CTrkAppContainer::~CTrkAppContainer()
{
}

// ---------------------------------------------------------
// CConnTestContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CTrkAppContainer::Draw(const TRect& /*aRect*/) const
{
    CWindowGc& gc = SystemGc();
    gc.Clear(Rect());
	
	TRect drawRect = Rect();
	
	const CFont *font = iEikonEnv->AnnotationFont();
	gc.UseFont(font);
	
	TBuf<KMaxPath> buf;

	_LIT(KTrkWelcome,"Welcome to TRK for Symbian OS");

	TInt vert = 25;
	
	gc.DrawText(KTrkWelcome, drawRect, vert, CGraphicsContext::ELeft, 2);
	vert += font->HeightInPixels()*2;
	
	if (iTrkAppView)
	{
        
        TTrkConnStatus statusType = iTrkAppView->GetConnectionStatus();
	    switch(statusType)
        {
		    case ETrkConnected:
			{
				gc.DrawText(KStatusConnected, drawRect, vert, CGraphicsContext::ELeft, 2);
		        break;
		        
			}
		    case ETrkNotConnected:
			{
				gc.DrawText(KStatusDisconnected, drawRect, vert, CGraphicsContext::ELeft, 2);
		        break;
	        }
		    case ETrkConnecting:
	        {
		        gc.DrawText(KStatusConnecting, drawRect, vert, CGraphicsContext::ELeft, 2);
		        break;
	        }
		    case ETrkConnectionError:
	        {
		        gc.DrawText(KStatusError, drawRect, vert, CGraphicsContext::ELeft, 2);
		        break;	            
	        }         
        }
		
		vert += font->HeightInPixels()*1.5;
		
		iTrkAppView->GetConnectionMsg(buf);
		_LIT(KLfCr, "\r\n");
		
		TPtrC ptr(buf);
		TInt pos = 0;
		TInt lineStart = 0;
		TInt length = 0;
		
		while (KErrNotFound != (pos = ptr.Find(KLfCr)))
		{
			ptr.Set(&buf[lineStart], pos);
			lineStart = lineStart + pos + 2;
			gc.DrawText(ptr, drawRect, vert, CGraphicsContext::ELeft, 2);
			vert += font->HeightInPixels();
			length = buf.Length() - lineStart;
			if (length > 0)
			{
				ptr.Set(&buf[lineStart], length);
			}
		}
	}
	
	gc.DiscardFont();
}

// End of File
