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

#ifndef __DbgTrcPortMgr_h__
#define __DbgTrcPortMgr_h__

// System includes
#include <e32base.h>
#include <c32comm.h>

#include "dbgtrccomm.h"

class CPortReader;
class CPortWriter;

class CDbgTrcPortMgr : public CBase
{

	public:
		static 	CDbgTrcPortMgr*	NewL();
				~CDbgTrcPortMgr();
	
	//private construct
	private:
			 CDbgTrcPortMgr() {};
		void ConstructL();

	public:
		TInt OpenPortL();
		TInt ClosePort();
		
		void GetPortConfig(TDes8& aDes);
		TInt SetPortConfig(const TDesC8& aDes);
		
		CPortWriter* GetPortWriter();
		CPortReader* GetPortReader();

	// data members		
	private:
		
		TBuf<32> iPDD;
		TBuf<32> iLDD;
		TBuf<32> iCSY;
		TUint iPortNumber;
		TBaudRates iRate;

		RCommServ iServer;
		RComm  iPort;

		TBool iServerStarted;
		TBool iConnected;
		TBool iActiveConnections;

		CPortReader* iReader;
		CPortWriter* iWriter;
		
		TAcmConfigV01 iAcmConfig;
						
};

#endif //__DbgTrcPortMgr_h__   
