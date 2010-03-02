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


#ifndef __TRKENGINE_H__
#define __TRKENGINE_H__

#include <e32base.h>
#include "TrkEngineCallback.h"
#include "TrkConnectionListener.h"
#include "TrkCommPortListener.h"
#include "TrkConnData.h"
#include "TrkCommPort.h"
//
// Forward declarations
//
class CTrkFramingLayer;
class CTrkDispatchLayer;
class CTrkCommPort;
class CTrkEngineModel;
class TTrkConnData;


//
// class CTrkEngine
//
// Provides the interface to the Trk engine
//
class CTrkEngine : public CBase, public MTrkConnectionListener
{
public:

	IMPORT_C static CTrkEngine* NewL();
	IMPORT_C static CTrkEngine* NewLC();
	IMPORT_C static CTrkEngine* NewL(MTrkEngineCallback* aCallback);
	IMPORT_C static CTrkEngine* NewLC(MTrkEngineCallback* aCallback);

	IMPORT_C ~CTrkEngine();

	IMPORT_C void StartL();
	IMPORT_C void StartL(TTrkConnType aConnType);
	IMPORT_C void Stop();
	IMPORT_C void GetVersionInfo(TInt &aMajorVersion, TInt &aMinorVersion, TInt &aMajorAPIVersion, TInt &aMinorAPIVersion, TInt &aBuildNumber);
	IMPORT_C void GetConnectionInfo(TDes& aMessage);
	IMPORT_C void GetErrorInfo(TDes& aMessage);
	IMPORT_C TTrkConnStatus GetConnectionStatus();
	
	IMPORT_C void GetConnectionData(TTrkConnData& aConnData);
	IMPORT_C TInt SetConnectionData(TTrkConnData& aConnData);
	IMPORT_C TBool IsDebugging();
	IMPORT_C TTrkConnType GetConnectionType();
	IMPORT_C TBool GetPlugPlaySetting();

public:
	// From MTrkConnectionListener
	void AsyncConnectionSuccessfulL();
	void AsyncConnectionFailed();
	
public:
	// call backs to dispatch layer
	void DebuggingStarted();
	void DebuggingEnded();	


private:

private:
	CTrkEngine();
	void DoStartL();
	void CreateCommInterfaceL();
	void ConstructL(MTrkEngineCallback* aCallback=NULL);
	void StartInactivityTimerDisabler();

private:

	CTrkCommPort* iCommPort;
	CTrkDispatchLayer* iDispatchLayer;
	CTrkEngineModel* iModel;
	TBuf<KMaxPath> iErrorMessage;
	
	CTrkCommPortListener* iCommPortListener;
	CInactivityTimerDisabler *iInactivityTimerDisabler;
	MTrkEngineCallback* iCallback;
	
	TTrkConnData iCurrentConnData;
	TTrkConnType iCurrentConnType;
};


#endif // __TRKENGINE_H__

