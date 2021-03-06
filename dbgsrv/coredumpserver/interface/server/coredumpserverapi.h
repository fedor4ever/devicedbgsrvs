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
// Definitions used between the Core Dump Server interface library and the server.
//



/**
 @file
 @publishedPartner 
 @released
*/

#ifndef CORE_DUMP_SERVER_API_H
#define CORE_DUMP_SERVER_API_H

#include <plugindata.h>
#include <optionconfig.h>

/** Core Dump Server executable name */
_LIT(KCoreDumpServerName, "coredump_svr");

/** Core Dump Server UID */
const TUid KCoreDumpServUid = { 0x10282FE5 };

/** Core Dump Server major version */
const TUint KCoreDumpServMajorVersionNumber=1;

/** Core Dump Server minor version */
const TUint KCoreDumpServMinorVersionNumber=1;

/** Core Dump Server build number */
const TUint KCoreDumpServBuildVersionNumber=2;

/** @see ECrashMediaName 
 * Length of each entry in the property ECrashMediaName 
 */
const TInt KMaxEntryLength = 160;

/** @see ECrashMediaName 
 * Number of entries in the property ECrashMediaName 
 */
const TInt KNumofEntries = 5;

/** @see ECrashMediaName
 * Size of Media Name in the property ECrashMediaName 
 */
const TInt KMaxMediaLength = 148;

/** @see ECrashMediaName
 * Length of UID in the property ECrashMediaName 
 */
const TInt KMaxUIDLength = 10;

/** @see ECrashMediaName
 * Length of the status in the property ECrashMediaName 
 */
const TInt KMaxStatusLength = 1;

/** @see ECrashMediaName
 * Length of the end character in the property ECrashMediaName 
 */
const TInt KMaxEndLength = 1;

/** @see ECrashMediaName
 * Length of the crash progress buffer ECrashProgress 
 */
const TInt KMaxCrashProgressBuffer = 50;

enum TCoreDumpConfigParameters
	{
	ECrashEventTypes,
	EPreCrashEventAction,
	EPostCrashEventAction,
	ELastParam
	};		

/** 
Core Dump Server client-server requests 
@internalTechnology 
*/
enum TCoreDumpServRqst
	{
	/** Obtain a plugin list. 
	@see RCoreDumpSession::GetPluginListL(). */
	ECoreDumpServGetPluginList,
	
	/** Generic call to obtain data about a list from the Core Dump Server. 
	@see RCoreDumpSession::GetListLC(). */
	ECoreDumpGetListInfo,

	/** Generic call to obtain the list from the Core Dump Server. 
	@see RCoreDumpSession::GetListLC(). */
	ECoreDumpGetListData,

	/** Request the loading or unloading of a plugin. 
	@see RCoreDumpSession::PluginRequestL(). */
	ECoreDumpPluginRequest,

	/** Obtain the current number of configuration parameters. 
	@see RCoreDumpSession::GetNumberConfigParametersL(). */
	ECoreDumpGetNumberConfigParams,

	/**  Obtain a configuration parameter.
	@see RCoreDumpSession::GetConfigParameterL(). */
	ECoreDumpGetConfigParam,

	/**  Change a configuration parameter.
	@see RCoreDumpSession::SetConfigParameterL(). */
	ECoreDumpSetConfigParam,


	/** Unused */
    ECoreDumpAttachThread,

	/** Unused */
    ECoreDumpDetachThread,

	/** Unused */
    ECoreDumpAttachProcess,

	/** Unused */
    ECoreDumpDetachProcess,

	/** Request the observation of a target or the cancellation of an observation.
	@see RCoreDumpSession::ObservationRequestL(). */
    ECoreDumpObservationRequest,

	/** Request the restoration of a configuration.
	@see RCoreDumpSession::LoadConfigL(). */
	ECoreDumpLoadConfig,

	/** Request the saving of a configuration.
	@see RCoreDumpSession::SaveConfigL(). */
	ECoreDumpSaveConfig,
	
	/** Deletes a crash in the flash partition */
	ECoreDumpDeleteLogRequest,
	
	/** Deletes entire flash partition */
	ECoreDumpDeleteCrashPartitionRequest,	
	
	/** Processes a crash in the flash partition */
	ECoreDumpProcessFlashCrash,
	
	/** Asynchronously processes a crash */
	ECoreDumpProcessCrashAsync,
	
	/** Cancel asynchronous processesing a crash */
	ECoreDumpCancelProcessCrashAsync,
	
	/**	Guard	*/
	ECoreDumpServEndMarker
	
	};

/** Number of asynchronous requests */
#define KTTMaxAsyncRequests		(4)

/** Default number of message slots per session
Same as KTTMaxAsyncRequests as synchronous requests provided
by framework */
#define KTTDefaultMessageSlots	(KTTMaxAsyncRequests)


/**
@internalTechnology
Number of plugin details in a plugin transfer block. 
Used between server library and cds
*/
#define KNumPluginDetails 5


/**
@internalTechnology
Class used for transfering fixed sized blocks of plugin information 
between the Core Dump Server and its clients. This makes it easier
accross the client/server interface.
@see TPluginInfo
*/
class TPluginInfoBlock
{
public:

	/**	
	A simple array is used as the transfer block across the 
	process boundary.
	*/
	TPluginInfo plugins[ KNumPluginDetails ];
};


/**
Enumerations that identify the RProperty objects shared between various parts 
of the core dump server system, plugins and clients.

The properties are defined by the Core Dump Session at startup. 
These properties have the uid of the Core Dump Server.

@publishedPartner 
@released
@see CCoreDumpSession::ConstructL().
*/
enum TCrashProgress 
	{ 
	/** Current crash status. Updated by formatters during a crash. 
	RProperty of type RProperty::EText. */
	ECrashProgress, 

	/** Cancels the generation of a crash when set to non-zero.
	RProperty of type RProperty::EInt. */
	ECancelCrash,

	/** Number of crashes served by this session of the Core Dump Server.
	RProperty of type RProperty::EInt. */
	ECrashCount,
	
	/** Name of the Crash Media Dump created. Updated by the CDS/Formatter
	RProperty of type RProperty::EText. */
	ECrashMediaName
	};



/**
@publishedPartner 
@released

Class for requesting lists from the Core Dump Server. 
The requestor appends the results of each request to obtain an entire list.
Uses "T<type>InfoBlock" classes to transfer data accross the process boundary.
*/
class TListRequest
{
public:

	/** Type of data to supply for this request */
	enum TListRequestType
		{
		/** This request is for the formatter plugin list */
		EFormatterList,

		/** This request is for the writer plugin list */
		EWriterList,

		/** This request is for the thread list */
		EThreadList,

		/** This request is for the process list */
		EProcessList,

		/** This request is for the executable list */
		EExecutableList,
		
		/** This request is for the crashes in flash */
		ECrashList,

		/** 
		@internalTechnology
		Last marker 
		*/
		EUnknownRequest

		};

	/** Type of data to supply for this request */
	TListRequestType	iListType;

	/** Sub identifiers for the request. These are auxiliary ids that 
	can be used to refine the request. */
	TUint32				iSubId1;
	TUint32				iSubId2;

	/** Requestor sets iIndex. iIndex==0 means the supplier should refresh the 
	list and supply from index 0. iIndex > 0 means supply element 
	[iIndex-1] and onwards. Requestor should thus increase iIndex 
	by how many elements were supplied on each call. */
	TUint				iIndex;

	/** Supplier writes iSupplied according to how many items 
	it copied into the buffer on this call */
	TUint				iSupplied;

	/** Supplier writes iRemaining to indicate how many items it 
	has yet to supply. */
	TUint				iRemaining;

	/** Number of bytes required to hold the resulting data */
	TUint				iRequiredDescriptorSize;

	/** 
	Spare variables for future expansion.
	@internalTechnology */
	TUint32				iSpare0;
	/** @internalTechnology */
	TUint32				iSpare1;
	/** @internalTechnology */
	TUint32				iSpare2;
	/** @internalTechnology */
	TUint32				iSpare3;

};

/**
@publishedPartner 
@released

Class for requesting change in configuration parameter from the CDS and loaded plugins. 
*/
class TConfigRequest
{
public:
	/** Type of data to supply for this request */
    COptionConfig::TParameterSource iSource;

	/** Sub identifiers for the request. These are auxiliary ids that 
	can be used to refine the request. */
	TUint32				iIndex;
	TUint32				iInstance;

	/** 
	Spare variables for future expansion.
	@internalTechnology */
	TUint32				iSpare0;
	/** @internalTechnology */
	TUint32				iSpare1;
};

#endif // CORE_DUMP_SERVER_API_H
