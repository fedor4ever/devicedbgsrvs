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
// Implemetation of class CProcessInfo
//



/**
 @file
 @see CProcessInfo
*/

#include <e32debug.h>
#include <debuglogging.h>

#include <processdata.h>

/**
Allocates and constructs a CProcessInfo object.
@param aId Kernel process id
@param aName Kernel process name
@see CProcessInfo::CProcessInfo
*/
EXPORT_C CProcessInfo* CProcessInfo::NewL( 
										const TUint64	  aId,
										const TDesC		& aName )
	{

	const TUint size = 
		  sizeof( TUint32 )		// iId Low
		+ sizeof( TUint32 )		// iId High
		+ sizeof( TUint32 )		// When externalized, we send the name length, so must include this
		+ 2						// When externalized, the << operator writes 2 bytes for the descriptor size
		+ aName.Size()			// iName Size, in bytes.
		+ sizeof( TUint32 ) 	// Observed
		+ sizeof( TUint32 );	// iSize itself

	if( size >= MaxSize() )
		{
		LOG_MSG3( "CProcessInfo::NewL() : Descriptorized object = 0x%X bytes would exceed the maximum of 0x%X\n", 
			size, MaxSize() );

		User::Leave( KErrTooBig );
		}

	CProcessInfo * data = new (ELeave) CProcessInfo( aId );

	CleanupStack::PushL( data );
	
	data->ConstructL( aName );

	CleanupStack::Pop(data);

	return (data);

	}


/**
Allocates and constructs a CProcessInfo object from a descriptor. 
The descriptor contains an externalised version of a CProcessInfo object.
This method is typically used to obtain a CProcessInfo object from a 
descriptor returned by the core dump server.

@param aStreamData Descriptor with externalised/streamed object
@see InternalizeL
@see ExternalizeL
*/
EXPORT_C CProcessInfo* CProcessInfo::NewL( const TDesC8 & aStreamData )
	{

	CProcessInfo* self = new (ELeave) CProcessInfo();
	
	CleanupStack::PushL( self );
	
	// Open a read stream for the descriptor
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
EXPORT_C CProcessInfo::~CProcessInfo()
	{

	if( NULL != iName )
		{
		delete iName;
		}
	}


/**
First phase contructor. Sets the size to 0, name to NULL.
@see CProcessInfo::NewL()
*/
CProcessInfo::CProcessInfo(	const TUint64	  aId ) :
	iId			 ( aId ),
    iObserved    ( EFalse ),
	iSize        ( 0 )
	{
	iName = NULL;
	}


/**
Second phase constructor initialises the name of the process.
@param aName Process name
@see NameL()
*/
void CProcessInfo::ConstructL(  const TDesC & aName )
	{
	NameL( aName );
	}


/**
Initialise this object with the contents of RReadStream aStream
The descriptor contains an externalised version of an object.
This method is typically used to obtain a CProcessInfo object from 
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
EXPORT_C void CProcessInfo::InternalizeL( RReadStream & aStream )
	{

	TUint32 idLow = aStream.ReadUint32L(); 

	TUint32 idHigh = aStream.ReadUint32L(); 
	iId = MAKE_TUINT64( idHigh, idLow );

	// Read the number of character elements in the name. 
	TUint32 nameLength = aStream.ReadUint32L(); 

	if( NULL != iName )
		{
		LOG_MSG( " iName != NULL\n" );
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

	iObserved = static_cast<TBool>(aStream.ReadUint32L());

	iSize = aStream.ReadUint32L() ;
	}



/**
Make a streamed representation of this object to RWriteStream aStream.

This method is typically by the core dump server when contructing a list of 
CProcessInfo for a client.
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
EXPORT_C void CProcessInfo::ExternalizeL( RWriteStream & aStream, CBufFlat* buf )
	{

	const TUint32 idLow = I64LOW( iId ); 
	const TUint32 idHigh = I64HIGH( iId ); 

	// Take the size of the buffer before we add anything to it.
	TUint startBufSize = buf->Size();

	aStream.WriteUint32L( idLow ); 
	aStream.WriteUint32L( idHigh ); 

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

	aStream.WriteUint32L( static_cast<TUint32>(iObserved) );

	iSize = buf->Size() - startBufSize + 4;

	aStream.WriteUint32L( iSize );

	}


/**
Obtain the kernel process id.
*/
EXPORT_C const TUint64 & CProcessInfo::Id() const 
	 { 
	 return ( iId ); 
	 }


/**
Set the name of the process by deleting, allocating and then copying the parameter.
@param aName Name of the process to set to
@see ConstructL()
*/
EXPORT_C void CProcessInfo::NameL( const TDesC & aName )
	{

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
Obtain the kernel process name.
*/
EXPORT_C const TDesC & CProcessInfo::Name() const 
	{ 
	if(!iName)
		{
		return KNoThreadName;
		}
	
	return ( *iName ); 
	}

/**
Returns ETrue if the process is being observed for crashes by the Core Dump Server.
*/
EXPORT_C TBool CProcessInfo::Observed() const 
	 { 
	 return ( iObserved ); 
	 }

/**
Set whether this process is being observed for crashes by the Core Dump Server.
*/
EXPORT_C void CProcessInfo::Observed( TBool aFlag ) 
	 { 
	 iObserved = aFlag; 
	 }

CProcessInfo::CProcessInfo()
	{
	}


 /* 
 Get the maximum size allowed for this object. This is needed as the object is passed  
 across the Client Server interface.
 */
 EXPORT_C TInt CProcessInfo::MaxSize()
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
 EXPORT_C TInt CProcessInfo::Size() const
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
		  sizeof( TUint32 )		// iId Low
		+ sizeof( TUint32 )		// iId High
		+ sizeof( TUint32 )		// When externalized, we send the name length, so must include this
		+ extNameSize
		+ sizeof( TUint32 ) 	// iObserved
		+ sizeof( TUint32 );	// iSize When externalized, we send the real externalized size of the buffer

	return size;
	}


