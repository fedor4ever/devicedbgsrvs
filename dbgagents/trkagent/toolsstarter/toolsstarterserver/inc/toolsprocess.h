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


#ifndef     _TOOLSPROCESS_H_
#define     _TOOLSPROCESS_H_

//  INCLUDES
#include <e32base.h>
#include <e32std.h>

#define KMaxCmdLineConnArgsLen 10

// DATA TYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION
class CToolsProcess;

enum TCmdLineConnType
{
	EUsb = 0,
	EXti,
};

//
// class CProcessTracker
//
// Watches to see when a process exists
//
class CProcessTracker : public CActive
{
public:

	CProcessTracker(CToolsProcess *aToolsProcess, TUint32 aProcessId);
	~CProcessTracker();

	void Watch();

protected:

	void RunL();
	void DoCancel();

private:

	RProcess iProcess;
	TUint32 iProcessId;
	CToolsProcess *iToolsProcess;
};


class CToolsProcess: public CBase
{
    public:  

		static CToolsProcess* NewL(const TDesC& aPath, const TDesC& aArgsUsb, const TDesC& aArgsXti);
		static CToolsProcess* NewLC(const TDesC& aPath, const TDesC& aArgsUsb, const TDesC& aArgsXti);
        ~CToolsProcess();
        
	public:
		TInt Start(TCmdLineConnType aConnType);
		void Stop();       
		
		void ProcessDied(TInt aExitReason);
		TDesC GetProcessPath() { return iPath; }

	private:
		CToolsProcess();
		void ConstructL(const TDesC& aPath, const TDesC& aArgsUsb, const TDesC& aArgsXti);		
		        
	private:
		TBuf<KMaxPath> iPath;
		TBuf<KMaxCmdLineConnArgsLen> iArgsUsb;
		TBuf<KMaxCmdLineConnArgsLen> iArgsXti;
		
		TBool iRunning;            
		
		TUint32 iProcessId;
		
		CProcessTracker* iProcessTracker;
		RProcess iProcess;
		
		static TBool iFirstTimeAfterBoot;
};


#endif      //  _TOOLSPROCESS_H_


//  End of File
