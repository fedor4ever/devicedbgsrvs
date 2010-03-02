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
// Implemetation of class CPluginInfo
//



/**
 @file
 @see CPluginInfo
*/

#include <e32debug.h>

#include <debuglogging.h>
#include <plugindata.h>


/**
Allocates and constructs a CPluginInfo object.

@see CPlguinInfo::CPluginInfo
*/
EXPORT_C CPluginInfo* CPluginInfo::NewL(const TDesC &aName,
									    const TInt aUid,
                                        const TUint aVersion,
									    const TPluginRequest::TPluginType aType)
	{
	const TUint size = 
		+ sizeof( TUint32 )		// When externalized, we send the name length, so must include this
		+ 2						// When externalized, the << operator writes 2 bytes for the descriptor size
		+ aName.Size()			// iName Size, in bytes.
		+ sizeof( TInt32 ) 	    // aUid
		+ sizeof( TUint32 ) 	// aVersion
		+ sizeof( TUint32 ) 	// aType
		+ sizeof( TUint32 ) 	// aPair
		+ sizeof( TUint32 );	// iSize itself


	if( size >= MaxSize() )
		{
		LOG_MSG3( "CPluginInfo::NewL() : Descriptorized object = 0x%X bytes would exceed the maximum of 0x%X\n", 
			size, MaxSize() );

		User::Leave( KErrTooBig );
		}

	CPluginInfo * data = new (ELeave) CPluginInfo( aUid, aVersion, aType );

	CleanupStack::PushL( data );
	
	data->ConstructL( aName );

	CleanupStack::Pop(data);

	return (data);
	}


/**
Allocates and constructs a CPluginInfo object from a descriptor. 
The descriptor contains an externalised version of a CPluginInfo object.
This method is typically used to obtain a CPluginInfo object from a 
descriptor returned by the core dump server.

@param aStreamData Descriptor with externalised/streamed object to initialize from.
@see InternalizeL
@see ExternalizeL
*/
EXPORT_C CPluginInfo* CPluginInfo::NewL( const TDesC8 & aStreamData )
	{
	CPluginInfo* self = new (ELeave) CPluginInfo();
	
	CleanupStack::PushL( self );
	
	RDesReadStream stream( aStreamData );

	CleanupClosePushL( stream );

	self->InternalizeL( stream );

	CleanupStack::PopAndDestroy( &stream ); // finished with the stream

	CleanupStack::Pop( self );

	return ( self );
	}


/**
Destructor. Deletes name if allocated.
*/
EXPORT_C CPluginInfo::~CPluginInfo()
	{

	if(iName)
		{
		delete iName;
		}
	}

/**
First phase contructor. Sets the size to 0, name to NULL.

@see CPluginInfo::NewL().
*/
CPluginInfo::CPluginInfo(const TInt aUid,
			const TUint aVersion,
            const TPluginRequest::TPluginType aType):

	iName(NULL),
    iUid(aUid ),
    iVersion(aVersion),
    iType(aType),
    iPair(KMaxTUint32),
	iSize(0)
	{
	}


/**
Second phase constructor initialises the name of the executable.
@param aName Plugin name
@see NameL()
*/
void CPluginInfo::ConstructL(const TDesC &aName)
	{
	NameL(aName);
	}


/**
Initialise this object with the contents of RReadStream aStream.
The descriptor contains an externalised version of an object.
This method is typically used to obtain a CPluginInfo object from 
the core dump server.
Any modifications to this method should be synchronised with ExternalizeL(). 
Also note that the methods used from RReadStream (>> or ReadUint32L) 
can behave differently, especially for descriptors.
@param aStream Stream with streamed object
@see ExternalizeL
@see RReadStream
@pre Call Externalise to obtain the stream containing an externalised 
version of this object.
*/
EXPORT_C void CPluginInfo::InternalizeL( RReadStream & aStream )
	{
	// Read the number of character elements in the name. 
	TUint32 nameLength = aStream.ReadUint32L(); 

	if( NULL != iName )
		{
		delete iName;
		iName = NULL;
		}

	if ( nameLength > 0 )
		{
		iName  = HBufC::NewL( aStream, nameLength ); 
		}
	else
		{
		iName  = NULL;
		}

	iUid = static_cast<TInt>(aStream.ReadInt32L());
	iVersion = static_cast<TUint>(aStream.ReadUint32L());
	iType = static_cast<TPluginRequest::TPluginType>(aStream.ReadUint32L());
	iPair = static_cast<TUint>(aStream.ReadUint32L());
	
	iSize = aStream.ReadUint32L() ;
	}


/**
Make a streamed representation of this object to a RWriteStream.

This method is typically by the core dump server when contructing a list of 
CPluginInfo for a client.
Any modifications to this method should be synchronised with InternalizeL().
Also note that the methods used from RWriteStream (>> or WriteUint32L) can behave differently,
especially for descriptors.
@param aStream Stream to stream object onto
@param buf Buffer onto the same stream, used to obtain the correct size of the externalised object
@see InternalizeL
@see RReadStream
@see RWriteStream
@post The stream contains an externalised version of this object.
*/
EXPORT_C void CPluginInfo::ExternalizeL( RWriteStream & aStream, CBufFlat* buf )
	{

	// Take the size of the buffer before we add anything to it.
	TUint startBufSize = buf->Size();

	TUint nameLength = 0;

	if ( ( NULL != iName ) && ( iName->Des().Length() > 0 ) )
		{

		nameLength = iName->Des().Length();

		if( nameLength > 0 )
			{
			// Write the number of character elements in the name. 
			aStream.WriteUint32L( nameLength ); 
			aStream << iName->Des();
			}
		}

	if( nameLength == 0 )
		{
		aStream.WriteUint32L( 0 ); 
		}

	aStream.WriteInt32L( static_cast<TInt32>(iUid) );
	aStream.WriteUint32L( static_cast<TUint32>(iVersion) );

	aStream.WriteUint32L( static_cast<TUint32>(iType) );
	aStream.WriteUint32L( static_cast<TUint32>(iPair) );
	
	iSize = buf->Size() - startBufSize + 4;

	aStream.WriteUint32L( iSize );

	}

/** 
*/
EXPORT_C TInt CPluginInfo::Uid() const 
	 { 
	 return ( iUid ); 
	 }

/** 
*/
EXPORT_C void CPluginInfo::Uid( TInt aUid ) 
	 { 
	 iUid = aUid; 
	 }

/** 
*/
EXPORT_C TUint CPluginInfo::Version() const 
	 { 
	 return ( iVersion ); 
	 }


/** 
*/
EXPORT_C void CPluginInfo::Version(TUint aVersion )
	 { 
	 iVersion = aVersion; 
	 }

/**
*/
EXPORT_C TPluginRequest::TPluginType CPluginInfo::Type() const 
	 { 
	 return ( iType ); 
	 }


/**
*/
EXPORT_C void CPluginInfo::Type( TPluginRequest::TPluginType aType ) 
	 { 
	 iType = aType; 
	 }
/** 
*/
EXPORT_C TUint CPluginInfo::Pair() const 
	 { 
	 return ( iPair ); 
	 }


/** 
*/
EXPORT_C void CPluginInfo::Pair(TUint aIndex )
	 { 
	 iPair = aIndex; 
	 }


/**
Set the name of the plugin by deleting, allocating and then copying the parameter.
@param aName Name of the executable to set to
@see ConstructL()
*/
EXPORT_C void CPluginInfo::NameL(const TDesC &aName)
	{

    if(iName)
        {
        delete iName; //missing in all other data files - leak?!?
        }

	if( aName.Length() > 0 )
		{
		TUint toCopy = aName.Length();
		iName = HBufC::NewL( toCopy );
		TPtr nameDes = iName->Des();

		nameDes.Copy( aName.Ptr(), toCopy );
		nameDes.SetLength( toCopy );
		}
	else
		{
		iName = NULL;
		}
	}


/**
Obtain the kernel executable name.
*/
EXPORT_C const TDesC & CPluginInfo::Name() const 
	{ 
	return ( *iName ); 
	}


CPluginInfo::CPluginInfo()
	{
	}


/** 
Get the maximum size allowed for this object. This is needed as the object is passed  
across the Client Server interface.
*/
EXPORT_C TInt CPluginInfo::MaxSize()
	{
	const TInt maxSize = 256;
	return maxSize;
	}


/**
Gets the size of the object when externalized. The sizeofs used to calculate this 
must match the operators used in ExternalizeL and InternalizeL.
Special attention must be paid to the name. If the object has not been 
externalized yet then this method returns the maximum that it could take.
The name descriptor is compressed when externalized, so it is not its Size().
Furthermore the << operator adds two bytes to the stream when externalizing 
a descriptor.
*/
EXPORT_C TInt CPluginInfo::Size() const
	{

	if( iSize != 0 )
		{
		return iSize;
		}

	TUint extNameSize = 0;
	if( iName )
		{
		extNameSize = 2					// When externalized, the << operator writes 2 bytes for the descriptor size
					+ iName->Size();	// iName itself, in bytes.
		}

	const TUint size = 
		+ sizeof( TUint32 )		// When externalized, we send the name length, so must include this
		+ extNameSize
		+ sizeof( TInt32 ) 	    // aUid
		+ sizeof( TUint32 ) 	// aVersion
		+ sizeof( TUint32 ) 	// aType
		+ sizeof( TUint32 ) 	// aPair
		+ sizeof( TUint32 );	// iSize When externalized, we send the real externalized size of the buffer

	return size;
	}


