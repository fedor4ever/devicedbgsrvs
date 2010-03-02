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
// Implemetation of class CExecutableInfo
//



/**
 @file
 @see CExecutableInfo
*/

#include <e32debug.h>

#include <debuglogging.h>
#include <executabledata.h>


/**
Allocates and constructs a CExecutableInfo object.
@param aName Name of target executable.
@param aActivelyDebugged True if executable if being actively debugged by Debug Security Server.
@param aPassivelyDebugged True if executable if being passively debugged by Debug Security Server.

@see CExecutableInfo::CExecutableInfo
*/
EXPORT_C CExecutableInfo* CExecutableInfo::NewL( const TDesC		& aName,
												const TBool  	  aActivelyDebugged,
												const TBool  	  aPassivelyDebugged )
	{


	const TUint size = 
		+ sizeof( TUint32 )		// When externalized, we send the name length, so must include this
		+ 2						// When externalized, the << operator writes 2 bytes for the descriptor size
		+ aName.Size()			// iName Size, in bytes.
		+ sizeof( TUint32 ) 	// aActivelyDebugged
		+ sizeof( TUint32 ) 	// aPassivelyDebugged
		+ sizeof( TUint32 ) 	// Observed
		+ sizeof( TUint32 );	// iSize itself



	if( size >= MaxSize() )
		{
		LOG_MSG3( "CExecutableInfo::NewL() : Descriptorized object = 0x%X bytes would exceed the maximum of 0x%X\n", 
			size, MaxSize() );

		User::Leave( KErrTooBig );
		}

	CExecutableInfo * data = new (ELeave) CExecutableInfo( aActivelyDebugged, aPassivelyDebugged );

	CleanupStack::PushL( data );
	
	data->ConstructL( aName );

	CleanupStack::Pop(data);

	return (data);

	}


/**
Allocates and constructs a CExecutableInfo object from a descriptor. 
The descriptor contains an externalised version of a CExecutableInfo object.
This method is typically used to obtain a CExecutableInfo object from a 
descriptor returned by the core dump server.

@param aStreamData Descriptor with externalised/streamed object to initialize from.
@see InternalizeL
@see ExternalizeL
*/
EXPORT_C CExecutableInfo* CExecutableInfo::NewL( const TDesC8 & aStreamData )
	{

	CExecutableInfo* self = new (ELeave) CExecutableInfo();
	
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
EXPORT_C CExecutableInfo::~CExecutableInfo()
	{

	if( NULL != iName )
		{
		delete iName;
		}
	}

/**
First phase contructor. Sets the size to 0, name to NULL.

@param aActivelyDebugged Whether this executable is being actively 
debugged according to the Debug Security Server.
@param aPassivelyDebugged Whether this executable is being passively 
debugged according to the Debug Security Server.

@see CExecutableInfo::NewL().
*/
CExecutableInfo::CExecutableInfo( const TBool aActivelyDebugged,
									const TBool aPassivelyDebugged ) :
    iActivelyDebugged    ( aActivelyDebugged ),
    iPassivelyDebugged   ( aPassivelyDebugged ),
    iObserved    ( EFalse ),
	iSize        ( 0 )
	{
	iName = NULL;
	}


/**
Second phase constructor initialises the name of the executable.
@param aName Executable name
@see NameL()
*/
void CExecutableInfo::ConstructL(  const TDesC & aName )
	{
	NameL( aName );
	}


/**
Initialise this object with the contents of RReadStream aStream.
The descriptor contains an externalised version of an object.
This method is typically used to obtain a CExecutableInfo object from 
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
EXPORT_C void CExecutableInfo::InternalizeL( RReadStream & aStream )
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

	iActivelyDebugged = static_cast<TBool>(aStream.ReadUint32L());
	iPassivelyDebugged = static_cast<TBool>(aStream.ReadUint32L());

	iObserved = static_cast<TBool>(aStream.ReadUint32L());

	iSize = aStream.ReadUint32L() ;
	}


/**
Make a streamed representation of this object to a RWriteStream.

This method is typically by the core dump server when contructing a list of 
CExecutableInfo for a client.
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
EXPORT_C void CExecutableInfo::ExternalizeL( RWriteStream & aStream, CBufFlat* buf )
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

	aStream.WriteUint32L( static_cast<TUint32>(iActivelyDebugged) );
	aStream.WriteUint32L( static_cast<TUint32>(iPassivelyDebugged) );

	aStream.WriteUint32L( static_cast<TUint32>(iObserved) );

	iSize = buf->Size() - startBufSize + 4;

	aStream.WriteUint32L( iSize );

	}

/** 
Return ETrue if the executable if being actively debugged by a client of the Debug Security Server.
*/
EXPORT_C TBool CExecutableInfo::ActivelyDebugged() const 
	 { 
	 return ( iActivelyDebugged ); 
	 }


/** 
Return ETrue if the executable if being passively debugged by a client of the Debug Security Server.
*/
EXPORT_C TBool CExecutableInfo::PassivelyDebugged() const 
	 { 
	 return ( iPassivelyDebugged ); 
	 }


/** 
Set whether the executable is being actively debugged by a client of the Debug Security Server.
*/
EXPORT_C void CExecutableInfo::ActivelyDebugged( TBool aActivelyDebugged ) 
	 { 
	 iActivelyDebugged = aActivelyDebugged; 
	 }


/** 
Set whether the executable is being passively debugged by a client of the Debug Security Server.
*/
EXPORT_C void  CExecutableInfo::PassivelyDebugged( TBool aPassivelyDebugged ) 
	 { 
	 iPassivelyDebugged = aPassivelyDebugged; 
	 }


/**
Returns ETrue if the the Core Dump Server is observing this executable.
*/
EXPORT_C TBool CExecutableInfo::Observed() const 
	 { 
	 return ( iObserved ); 
	 }


/**
Set whether the the Core Dump Server is observing this executable.
*/
EXPORT_C void CExecutableInfo::Observed( TBool aFlag ) 
	 { 
	 iObserved = aFlag; 
	 }


/**
Set the name of the executable by deleting, allocating and then copying the parameter.
@param aName Name of the executable to set to
@see ConstructL()
*/
EXPORT_C void CExecutableInfo::NameL( const TDesC & aName )
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
Obtain the kernel executable name.
*/
EXPORT_C const TDesC & CExecutableInfo::Name() const 
	{ 
	return ( *iName ); 
	}


CExecutableInfo::CExecutableInfo()
	{
	}


/** 
Get the maximum size allowed for this object. This is needed as the object is passed  
across the Client Server interface.
*/
EXPORT_C TInt CExecutableInfo::MaxSize()
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
EXPORT_C TInt CExecutableInfo::Size() const
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
		+ sizeof( TUint32 ) 	// iActivelyDebugged
		+ sizeof( TUint32 ) 	// iPassivelyDebugged
		+ sizeof( TUint32 ) 	// iObserved
		+ sizeof( TUint32 );	// iSize When externalized, we send the real externalized size of the buffer

	return size;
	}


