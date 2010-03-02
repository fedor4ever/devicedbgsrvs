/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __TrkSettingsData_h__
#define __TrkSettingsData_h__

// INCLUDES
#include <e32base.h>

#include "trksrvclisession.h"
#include "TrkConnData.h"

/**
* CTrkSettingsData 
*/
class CTrkSettingsData : public CBase
{
    public:
        static CTrkSettingsData* NewL(RTrkSrvCliSession& aTrkSession);
        ~CTrkSettingsData();
        TInt SaveSettings();

    private:
        CTrkSettingsData(RTrkSrvCliSession& aTrkSession);
        void ConstructL();
		TInt GetIndexForBaudRate(TUint aRate);
		TUint GetBaudRateForIndex(TInt aIndex);
    public:       	
    	TInt 	  iConnTypeIndex;
    	TInt      iPortNumber;
    	TInt	  iBaudRateIndex;
    	TInt      iPlugAndPlay;
    	TInt      iPlugAndPlayType;
    	
    	TTrkConnData iTrkConnData;
    
    private:
        RTrkSrvCliSession& iTrkSession;
};


#endif //__TrkSettingsData_h__

// End of File
