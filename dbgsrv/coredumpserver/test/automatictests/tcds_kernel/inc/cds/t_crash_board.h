/**
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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



/**
 @file t_crash_board.h
 @internalTechnology
 @prototype
*/

#ifndef __CCRASHBOARDWRAPPER_H__
#define __CCRASHBOARDWRAPPER_H__

#include <datawrapper.h>
#include <rm_debug_api.h>
#include <coredumpinterface.h>
#include <coredumpserverapi.h>

#include "t_common_defs.h"

/**
 * This class is responsible for testing our crashing the system functionality
 */
class CCrashBoardWrapper : public CDataWrapper
	{

public:

	~CCrashBoardWrapper();

	static CCrashBoardWrapper* NewL();

	virtual TBool DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
	virtual TAny* GetObject() { }
	virtual void SetObjectL(TAny* aObject);

protected:
	void ConstructL();

private:
	/** Constructor */
	CCrashBoardWrapper();

	void PrepareTestL();

    void DoCmdCrashBoardL(const TDesC& );
    void DoCmdCleanCrashPartition();
	void DoCmdCleanEntireCrashPartitionL();

private:
//add members here
	//DSS session
	RSecuritySvrSession iSecSess;
	};
#endif // __CCrashBoardWRAPPER_H__
