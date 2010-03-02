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
 @file CServerDataSourceWrapper.h
 @internalTechnology
 @prototype 
*/

#ifndef __CSERVERDATASOURCEWRAPPER_H__
#define __CSERVERDATASOURCEWRAPPER_H__

#include <datawrapper.h>
#include <rm_debug_api.h>

#include "servercrashdatasource.h" 

using namespace Debug;

/**
This is the version of the security server that we have developed our code against
and it comes from the interface definition of the DSS at the time of compilation.
*/
const TVersion dssVersion( 
				KDebugServMajorVersionNumber, 
				KDebugServMinorVersionNumber, 
				KDebugServPatchVersionNumber );

_LIT(KCrashAppFileName,"Z:\\sys\\bin\\crashapp.exe");

const TInt KTraceSizeFromCrashApp = 2560;
/**
 * This class is responsible for testing our CFlashDataSource class
 */
class CServerDataSourceWrapper : public CDataWrapper
	{
	
public:
	
	~CServerDataSourceWrapper();

	static CServerDataSourceWrapper* NewL();

	virtual TBool DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
	virtual TAny* GetObject() { return iObject; }
	virtual void SetObjectL(TAny* aObject);

protected:
	void ConstructL();

private:
	
	/** Constructor */
	CServerDataSourceWrapper();	
	
	/** Tests GetAvailableTraceSizeL */
	void DoCmdGetAvailableTraceSizeL_TestL();
	
	void PrepareTestL();
	
private:
	
	/** Object to be tested */
	CServerCrashDataSource* iObject;
	
	/** Handle to debug security server */
	RSecuritySvrSession iSecSess;
	
	};
#endif // __CFLASHDATASOURCEWRAPPER_H__
