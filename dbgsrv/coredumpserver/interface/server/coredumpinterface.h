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
// Define the interface to the Core Dump Server
//



/**
 @file
 @publishedPartner 
 @released
*/

#ifndef CORE_DUMP_API_H
#define CORE_DUMP_API_H

#include <e32base.h>


#include <processdata.h>
#include <executabledata.h>
#include <threaddata.h>
#include <coredumpserverapi.h>
#include <optionconfig.h>

#include <debuglogging.h>
#include <crashdata.h>

/**
@publishedPartner
@released

Definition of the Core Dump Server interface.
This is used by clients to interact with the server.

*/
class RCoreDumpSession	: public RSessionBase
	{
        static TInt StartServer();

	public:		// Construction
		IMPORT_C RCoreDumpSession();
		
	public:
		IMPORT_C TInt Connect();
		IMPORT_C void Disconnect();

		IMPORT_C void GetPluginListL( RPluginList &aPluginList ) const;
		IMPORT_C void PluginRequestL(const TPluginRequest &aPluginRequest ) const;

        IMPORT_C void GetProcessesL( RProcessPointerList &aProcessList ) const;
        IMPORT_C void GetThreadsL( RThreadPointerList &aThreadsList, const TUint64 aPid = (TUint64)-1 ) const;
		IMPORT_C void GetExecutablesL( RExecutablePointerList &aExecutableList ) const;

        IMPORT_C void ObservationRequestL( const TDesC &aTargetName, const TDesC &aTargetOwnerName, TBool aObserve) const;

		IMPORT_C TInt GetNumberConfigParametersL() const;
		IMPORT_C COptionConfig * GetConfigParameterL( const TInt aIndex ) const;
	    IMPORT_C void SetConfigParameterL( const COptionConfig &aConfig ) const;

		IMPORT_C void LoadConfigL( const TDesC & aLoadPath ) const;
		IMPORT_C void SaveConfigL( const TDesC & aSavePath ) const;

        IMPORT_C void GetFormattersL( RPluginPointerList &aFormatterList ) const;
        IMPORT_C void GetWritersL( RPluginPointerList &aWriterList ) const;

        IMPORT_C void ListCrashesInFlashL(RCrashInfoPointerList& aCrashes) const;
        IMPORT_C void DeleteCrashLogL(TInt aCrashId) const;
        IMPORT_C void DeleteCrashPartitionL() const;
        IMPORT_C void ProcessCrashLogL(const TInt aCrashId);
        
        IMPORT_C void ProcessCrashLog(TInt aCrashId, TRequestStatus &aStatus);
        IMPORT_C TInt CancelProcessCrashLog(TInt aCrashId);
        
	private:	
        const TDesC8 &GetListLC(const TListRequest &aRequest) const;
		TVersion Version() const;

	private:	// Data members

        enum {KConnectionRetries = 3};

		TBool iConnected;

		TInt iMaxConfigParamSize;

		TUint32 iSpare0;
		TUint32 iSpare1;
		TUint32 iSpare2;
		TUint32 iSpare3;
	};


#endif // CORE_DUMP_API_H
