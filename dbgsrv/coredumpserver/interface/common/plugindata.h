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
// Defines ECOM-plugin related classes used accross the Core Dump client/server interface 
//



/**
 @file
 @publishedPartner 
 @released
*/

#ifndef PLUGINDATA_H
#define PLUGINDATA_H

/**
@publishedPartner 
@released

Class used between Core Dump Server and its clients to request 
the server to carry out actions on plugins, such as load/unload.
@see CCrashConfig::LoadPlugin()
*/
class TPluginRequest
{
public:

	
	inline TPluginRequest(){ iIndex = 0; }
	
	/** 
	Type of plugin to request
	*/
	enum TPluginType
		{
		/** A formatter plugin */
		EFormatter,
		/** A writer plugin */
		EWriter
		};


	/* Whether to load or unload the plugin */
	TBool						iLoad; 

	/** Type of plugin */
	TPluginType					iPluginType;

    /** Index of the plugin that requires action */
	TUint						iIndex;

	/** Plugin UID, used to check correct plugin */
	TUid						iUid;

    /** Index of the pair plugin */
	TUint                       iPair;

	/** @internalTechnology */
	TUint32 iSpare2;
};


/** Maximum length allowed for a plugin's name */
#define KPluginNameLength (50)

/** Maximum length allowed for a plugin's description */
#define KPluginDescriptionLength (250)

/**
@publishedPartner 
@released

Detailed information about a plugin.
@see RPluginList
@see TPluginInfoBlock
*/
class TPluginInfo
{
public:

	/** Plugin's own version.
	From rss file::IMPLEMENTATION_INFO::version_no */
	TUint					iVersion;

	/** Plugin's unique UID. 
	From rss file::IMPLEMENTATION_INFO::implementation_uid */
	TUid					iUid;

	/** Plugin's own name.
	From rss file::IMPLEMENTATION_INFO::display_name */
	TBufC<KPluginNameLength>			iName;

	/** Plugin's own description obtained from 
	CCoreDumpFormatter->GetDescription()
	@see CCoreDumpFormatter */
	TBufC<KPluginDescriptionLength>	iDescription;

	/** Plugin type : from TPluginRequest::TPluginType */
	TPluginRequest::TPluginType	iType;

	/** Plugin loaded state. */
	TBool					iLoaded;

	/** Reserved for future use
	@internalTechnology */
	TUint32 iSpare1;

	/** @internalTechnology */
	TUint32 iSpare2;
};


/**
@publishedPartner 
@released

List of TPluginInfo objects.
@see TPluginInfo
@see RArray
*/
typedef RArray<TPluginInfo>	RPluginList;


#include <streamelement.h>

/**
@publishedPartner 
@released

Class that represents a ECOM plugin instance. 
It is based on CStreamElementBase so it can be streamed between client and server.
@see CServerCrashDataSource::GetPluginListL
*/
class CPluginInfo : public CStreamElementBase
{

public:

	IMPORT_C static CPluginInfo* NewL( const TDesC &aName,
									   const TInt aUid,
                                       const TUint aVersion,
									   const TPluginRequest::TPluginType aType); 

	IMPORT_C static CPluginInfo* NewL( const TDesC8 & aStreamData );

	IMPORT_C ~CPluginInfo();

public:

	IMPORT_C void NameL( const TDesC & aName );
	IMPORT_C const TDesC & Name() const;

    IMPORT_C void Uid(const TInt aUid);
    IMPORT_C TInt Uid() const;

    IMPORT_C void Version(TUint aVersion);
    IMPORT_C TUint Version() const;

    IMPORT_C void Pair(TUint aIndex);
    IMPORT_C TUint Pair() const;

    IMPORT_C void Type(TPluginRequest::TPluginType aType);
    IMPORT_C TPluginRequest::TPluginType Type() const;

public:
	// Methods required by streaming interface 

	IMPORT_C TInt static MaxSize();

	IMPORT_C TInt Size() const;

	// Initializes ’this’ from stream
	IMPORT_C void InternalizeL( RReadStream & aStream );

	// Writes ’this’ to the stream
	IMPORT_C void ExternalizeL( RWriteStream & aStream, CBufFlat* buf );


private:

	CPluginInfo( const TInt aUid,
				 const TUint aVersion,
                 const TPluginRequest::TPluginType aType);

	CPluginInfo();

	void ConstructL( const TDesC & aName );

private:

    /** Plugin name */
	HBufC *iName;

    /** Plugin UID */
	TInt iUid;

    /** Plugin version */
	TUint iVersion;

	/** Plugin type : from TPluginRequest::TPluginType */
	TPluginRequest::TPluginType	iType;

    /** Index to paired plugin */
    TUint iPair;

	/** Externalized size */
	TUint				iSize;

	/** Reserved for future use
	@internalTechnology */
	TUint32 iSpare1;

	/** @internalTechnology */
	TUint32 iSpare2;

	};


/**
@publishedPartner 
@released

List of CPluginInfo object pointers
@see CPluginInfo
@see RPointerArray
*/
typedef RPointerArray<CPluginInfo>	RPluginPointerList;


#endif
