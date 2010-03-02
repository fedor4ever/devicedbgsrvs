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



// INCLUDE FILES
#include "TrkSettingsData.h"

// ================= MEMBER FUNCTIONS =========================================

// ----------------------------------------------------------------------------
// CTrkSettingsData::NewL
// Two-phase constructor
// ----------------------------------------------------------------------------
//
CTrkSettingsData* CTrkSettingsData::NewL(RTrkSrvCliSession& aTrkSession)
{
    CTrkSettingsData* data = new(ELeave) CTrkSettingsData(aTrkSession);
    CleanupStack::PushL(data);
    data->ConstructL();
    CleanupStack::Pop(); // data
    return data;
}

// Destructor
//
CTrkSettingsData::~CTrkSettingsData()
{
}

// Constructor
//
CTrkSettingsData::CTrkSettingsData(RTrkSrvCliSession& aTrkSession)
:iTrkSession(aTrkSession)
{
    iConnTypeIndex = 1;

	iPortNumber = KDefaultUsbPortNumber;
	iBaudRateIndex = 8;
}

// Epoc default constructor
//
void CTrkSettingsData::ConstructL()
{
    iTrkSession.GetCurrentConnData(iTrkConnData);
	iConnTypeIndex = iTrkConnData.iConnType;

#ifdef __OEM_TRK__
	if (iConnTypeIndex>ETrkDcc)
#else
	if (iConnTypeIndex>ETrkBt)
#endif
	{
		iTrkConnData.iConnType = ETrkBt;
		iConnTypeIndex = ETrkBt;
	}
	iPlugAndPlay = iTrkConnData.iPlugPlay;
	iPortNumber = iTrkConnData.iPortNumber;
	iBaudRateIndex = GetIndexForBaudRate(iTrkConnData.iRate);
	
	if(iPlugAndPlay == 1)
	  iPlugAndPlayType = 0;
	else
	  iPlugAndPlayType = 1;
	
}

TInt CTrkSettingsData::SaveSettings()
{
	iTrkConnData.iConnType = (TTrkConnType)iConnTypeIndex;
	iTrkConnData.iRate = GetBaudRateForIndex(iBaudRateIndex);
	iTrkConnData.iPortNumber = iPortNumber;
	
	if(iPlugAndPlayType == 1)
	    iPlugAndPlay = 0;
	else
	    iPlugAndPlay = 1;
	
	
	switch (iTrkConnData.iConnType)
	{
		case ETrkUsbDbgTrc:
		{
			iTrkConnData.iCommType = EDbgTrcComm;
			iTrkConnData.iPDD.Copy(KDefaultUsbPDD);
			iTrkConnData.iLDD.Copy(KDefaultUsbLDD);
			iTrkConnData.iCSY.Copy(KDefaultUsbCSY);
			iTrkConnData.iPlugPlay = iPlugAndPlay;
			break;
		}
		case ETrkBt:
		{
			iTrkConnData.iCommType = EBtSocket;
			iTrkConnData.iPDD = KNullDesC;
			iTrkConnData.iLDD = KNullDesC;
			iTrkConnData.iCSY = KNullDesC;
			iTrkConnData.iPlugPlay = iPlugAndPlay;
			break;
		}
		case ETrkXti:
		{
			iTrkConnData.iCommType = EXti;
			iTrkConnData.iPDD = KNullDesC;
			iTrkConnData.iLDD.Copy(KTrkXTILDD);
			iTrkConnData.iCSY = KNullDesC;
			iTrkConnData.iPlugPlay = iPlugAndPlay;
			break;
		}
		case ETrkDcc:
		{
			iTrkConnData.iCommType = EDcc;
			iTrkConnData.iPDD = KNullDesC;
			iTrkConnData.iLDD.Copy(KTrkDCCLDD);
			iTrkConnData.iCSY = KNullDesC;
			iTrkConnData.iPortNumber = 42;
			iTrkConnData.iPlugPlay = iPlugAndPlay;
			break;
		}
		case ETrkUsb:
		{
			iTrkConnData.iCommType = ESerialComm;
			iTrkConnData.iPDD.Copy(KDefaultUsbPDD);
			iTrkConnData.iLDD.Copy(KDefaultUsbLDD);
			iTrkConnData.iCSY.Copy(KDefaultUsbCSY);
			iTrkConnData.iPlugPlay = iPlugAndPlay;
			break;
		}
		case ETrkIr:
		{
			iTrkConnData.iCommType = ESerialComm;
			iTrkConnData.iPDD.Copy(KDefaultIrPDD);
			iTrkConnData.iLDD.Copy(KDefaultIrLDD);
			iTrkConnData.iCSY.Copy(KDefaultIrCSY);
			iTrkConnData.iPlugPlay = iPlugAndPlay;
			break;
		}
		default:
		{
			iTrkConnData.iCommType = ESerialComm;
			break;
		}
	}
	return iTrkSession.SetCurrentConnData(iTrkConnData);
}

TInt CTrkSettingsData::GetIndexForBaudRate(TUint aRate)
{
	switch (aRate)
	{
		case 300:     return 0;
		case 1200:    return 1;
		case 2400:    return 2;
		case 4800:    return 3;
		case 9600:    return 4;
		case 19200:   return 5;
		case 38400:   return 6;
		case 57600:   return 7;
		case 115200:  return 8;
		case 234000:  return 9;
		case 460800:  return 10;
		case 576000:  return 11;
		case 1152000: return 12;
		case 4000000: return 13;
		
		default: return 8;
		
	}
	
}

TUint CTrkSettingsData::GetBaudRateForIndex(TInt aIndex)
{
	switch (aIndex)
	{
		case 0:  return 300;
		case 1:  return 1200;
		case 2:  return 2400;
		case 3:  return 4800;
		case 4:  return 9600;
		case 5:  return 19200;
		case 6:  return 38400;
		case 7:  return 57600;
		case 8:  return 115200;
		case 9:  return 234000;
		case 10: return 460800;
		case 11: return 576000;
		case 12: return 1152000;
		case 13: return 4000000;
		
		default: return 115200;
		
	}
	
}
// End of File
