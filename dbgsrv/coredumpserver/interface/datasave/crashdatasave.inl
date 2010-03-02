// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Implements some of the ECOM Core Dump Writer interface.
// The rest is implemented by the inheritors.
//



/**
 @file
 @publishedPartner 
 @released
 @see CCrashDataSave
*/

// Set default to be first implementation found by resolver
_LIT8(KDefaultImpl, "*");

inline CCrashDataSave::~CCrashDataSave()
    {
    // Destroy any instance variables and then
    // inform the framework that this specific
    // instance of the interface has been destroyed.
    REComSession::DestroyedImplementation(iDtor_ID_Key);
    }

inline CCrashDataSave* CCrashDataSave::NewL()
    {
    // Set up the interface find for the default resolver.
    TEComResolverParams resolverParams;
    resolverParams.SetDataType(KDefaultImpl());
    resolverParams.SetGenericMatch(ETrue);     // Allow wildcard matching

    // Set up some empty initialisation parameters
    TExampleInterfaceInitParams initParams;
    initParams.integer      = 0;
    initParams.descriptor   = NULL;

    return reinterpret_cast<CCrashDataSave*>(
                            REComSession::CreateImplementationL(KCCrashDataSaveUid,
                                                             _FOFF(CCrashDataSave, iDtor_ID_Key),
                                                             &initParams,
                                                             resolverParams));
    }

inline CCrashDataSave* CCrashDataSave::NewL(const TDesC8& aCue)
	{
	TEComResolverParams resolverParams;
	resolverParams.SetDataType(aCue);
	resolverParams.SetGenericMatch(ETrue);
	TAny* fileWriter = REComSession::CreateImplementationL(KCCrashDataSaveUid, _FOFF(CCrashDataSave, iDtor_ID_Key),
		resolverParams);
	return reinterpret_cast<CCrashDataSave*>(fileWriter);
	}

inline CCrashDataSave* CCrashDataSave::NewL(const TUid& aUid)
	{
	TAny* fileWriter = REComSession::CreateImplementationL(aUid, _FOFF(CCrashDataSave, iDtor_ID_Key));
	return reinterpret_cast<CCrashDataSave*>(fileWriter);
	}

inline void CCrashDataSave::ListAllImplementationsL(RImplInfoPtrArray& aImplInfoArray)
	{
	REComSession::ListImplementationsL(KCCrashDataSaveUid, aImplInfoArray);
	}


