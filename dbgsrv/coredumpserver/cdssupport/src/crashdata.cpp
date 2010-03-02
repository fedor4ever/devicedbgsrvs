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
// Implemetation of class TRegisterData methods. 
//



/**
 @file
 @see TRegisterData
*/

#include <e32std.h>
#include <e32const.h>
#include <rm_debug_api.h>
#include <s32mem.h>
#include <crashdata.h>
#include <debuglogging.h> 

using namespace Debug;

/** Return the register identifier. 
@see Sym32_reginfod::rd_id
*/
EXPORT_C TUint16 TRegisterData::GetId() const
	{
	return iId;
	}


/** Return the register sub-identifier. 
@see Sym32_reginfod::rd_repre
*/
EXPORT_C TUint16 TRegisterData::GetSubId() const
	{
	return iSubId;
	}

/** Return the register size in number of bits.
@see Sym32_reginfod::rd_repre
*/
EXPORT_C TUint8  TRegisterData::GetSize() const
	{
	return iSize;
	}

/** Return the register class
@see Sym32_reginfod::rid_class
*/
EXPORT_C TUint8  TRegisterData::GetClass() const
	{
	return iRegClass;
	}


/** Return the register value as 8 bits */	
EXPORT_C TUint8  TRegisterData::GetContent8()  const
	{
	return iValue8;
	}

/** Return the register value as 16 bits */	
EXPORT_C TUint16 TRegisterData::GetContent16() const
	{
	return iValue16;
	}

/** Return the register value as 32 bits */	
EXPORT_C TUint32 TRegisterData::GetContent32() const
	{
	return iValue32;
	}

/** Return the register value as 64 bits */	
EXPORT_C TUint64 TRegisterData::GetContent64() const
	{
	return iValue64;
	}


/** 
Returns ETrue if all the register's attributes are the same, exclugin the contents 
@param other The register to compare the attributes against
*/	
EXPORT_C TBool TRegisterData::SameRegister( const TRegisterData & other ) const
	{

	if( other.iRegClass != iRegClass )
		{
		return EFalse;
		}

	if( other.iId != iId )
		{
		return EFalse;
		}

	if( other.iSubId != iSubId )
		{
		return EFalse;
		}

	if( other.iSize != iSize )
		{
		// Not sure this is a valid check, but should be same
		return EFalse;
		}

	return ETrue;

	}


/** 
Takes the content from another TRegisterData
@param other The register to copy the contents from
*/	
EXPORT_C void	TRegisterData::SetContent( const TRegisterData & other )
	{
	switch( iSize )
		{
		case ERegRepr8:
			iValue8 = other.GetContent8();
			break;
		case ERegRepr16:
			iValue16 = other.GetContent16();
			break;
		case ERegRepr32:
			iValue32 = other.GetContent32();
			break;
		case ERegRepr64:
			iValue64 = other.GetContent64();
			break;
		default:
			// Best leave untouched
			break;
		}
	}

/** 
Set the register value as 8 bits 
@param aVal The value to set the register contents to
*/	
EXPORT_C void	TRegisterData::SetContent8( const TUint8 aVal )
	{
	iValue8 = aVal;
	}

/** 
Set the register value as 16 bits 
@param aVal The value to set the register contents to
*/	
EXPORT_C void	TRegisterData::SetContent16( const TUint16 aVal )
	{
	iValue16 = aVal;
	}

/** 
Set the register value as 32 bits
@param aVal The value to set the register contents to
*/	
EXPORT_C void	TRegisterData::SetContent32( const TUint32 aVal )
	{
	iValue32 = aVal;
	}

/** 
Set the register value as 64 bits
@param aVal The value to set the register contents to
*/	
EXPORT_C void	TRegisterData::SetContent64( const TUint64 aVal )
	{
	iValue64 = aVal;
	}

/** 
Set the register value, casting it to the correct size.
@param aVal The value to set the register contents to.
*/
EXPORT_C void	TRegisterData::SetContent( const TUint aVal )
	{
	switch( GetSize() )
		{
		case ERegRepr8:
			iValue8 = (TUint8) aVal;
			break;
		case ERegRepr16:
			iValue16 = (TUint16) aVal;
			break;
		case ERegRepr32:
			iValue32 = (TUint32) aVal;
			break;
		case ERegRepr64:
			iValue64 = (TUint64) aVal;
			break;
		default:
			// Best leave untouched
			break;
		}
	}


/** Return ETrue if the register content was obtained correctly */
EXPORT_C TBool TRegisterData::Available( ) const
	{
	return iAvailable;
	}


/** Set the register content availability */
EXPORT_C void TRegisterData::SetAvailable( const TBool aAvailable )
	{
	iAvailable = aAvailable;
	}

EXPORT_C TCrashInfo* TCrashInfo::NewL(const TDesC8 & aStreamData)
	{	
	TCrashInfo* self = new (ELeave) TCrashInfo();	
	CleanupStack::PushL(self);
	
	RDesReadStream stream(aStreamData);
	CleanupClosePushL(stream);

	self->InternalizeL(stream);
	CleanupStack::PopAndDestroy(&stream); // finished with the stream

	CleanupStack::Pop(self);

	return self;
	}

EXPORT_C TCrashInfo* TCrashInfo::NewL(const TCrashInfoHeader& aCrashHeader)
	{	
	TCrashInfo* self = new (ELeave) TCrashInfo(aCrashHeader);	
	CleanupStack::PushL(self);
	
	self->ConstructL(aCrashHeader);
	
	CleanupStack::Pop(self);

	return self;
	}

EXPORT_C TCrashInfo::TCrashInfo()
	{	
	}

TCrashInfo::TCrashInfo(const TCrashInfoHeader& aCrashHeader) :
	iType(ECrashException), 
	iExcNumber(aCrashHeader.iExcCode), 
	iReason(aCrashHeader.iExitReason), 
	iTid(aCrashHeader.iTid), 
	iPid(aCrashHeader.iPid),
	iTime(aCrashHeader.iCrashTime), 
	iCrashId(aCrashHeader.iCrashId), 
	iSize(aCrashHeader.iLogSize),
	iCrashSource(ELast)
	{

	}

void TCrashInfo::ConstructL(const TCrashInfoHeader& aCrashHeader)
	{
	RBuf inter;
	inter.Create(aCrashHeader.iCategorySize * 2);
	inter.CleanupClosePushL();
	inter.Copy(aCrashHeader.iCategory);	
	
	iCategory.SetLength(0);	
	TInt lengthToCopy = (aCrashHeader.iCategorySize >= iCategory.MaxLength()) ? iCategory.MaxLength() : aCrashHeader.iCategorySize;
	iCategory.Copy(inter.Ptr(), lengthToCopy);
	
	CleanupStack::PopAndDestroy(&inter);
	}

/**
Initialise this object with the contents of RReadStream aStream.
The descriptor contains an externalised version of an object.
This method is typically used to obtain a TCrashInfo object from 
the core dump server. 
Also note that the methods used from RReadStream (>> or ReadUint32L) 
can behave differently, especially for descriptors.
@param aStream Stream with streamed object
@see ExternalizeL
@see RReadStream
*/
EXPORT_C void TCrashInfo::InternalizeL(RReadStream & aStream)
	{	
	iType = static_cast<TCrashInfo::TCrashType>(aStream.ReadUint32L());	
	iExcNumber = aStream.ReadUint32L();		
	iReason = aStream.ReadInt32L();
	
	TUint32 nameLength = aStream.ReadUint32L();
	aStream.ReadL(iCategory, nameLength);	
	
	iTid = MAKE_TUINT64(aStream.ReadUint32L(), aStream.ReadUint32L());
	iPid = MAKE_TUINT64(aStream.ReadUint32L(), aStream.ReadUint32L());
	iTime = MAKE_TUINT64(aStream.ReadUint32L(), aStream.ReadUint32L());
	
	aStream.ReadL((TUint8*)(&iContext), sizeof(TRmdArmExcInfo));
	iCrashId = aStream.ReadUint32L();
	iCrashSource = static_cast<TCrashInfo::TCrashSource>(aStream.ReadUint32L());
	
	iSize = aStream.ReadUint32L();
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
EXPORT_C void TCrashInfo::ExternalizeL(RWriteStream & aStream, CBufFlat* aBuf)
	{
	LOG_MSG("TCrashInfo::ExternalizeL()");
	
	// Take the size of the buffer before we add anything to it.
	TUint startBufSize = aBuf->Size();
	
	aStream.WriteUint32L( static_cast<TCrashInfo::TCrashType>(iType) );
	aStream.WriteUint32L( static_cast<TUint32>(iExcNumber) );
	aStream.WriteInt32L( static_cast<TUint32>(iReason) );
	
	//write the category name
	TUint nameLength = iCategory.Length();
	aStream.WriteUint32L(nameLength);
	
	if(nameLength > 0)
		{		
		aStream.WriteL(iCategory);
		}
	
	aStream.WriteUint32L(static_cast<TUint32>I64HIGH(iTid));
	aStream.WriteUint32L(static_cast<TUint32>I64LOW(iTid));
	
	aStream.WriteUint32L(static_cast<TUint32>I64HIGH(iPid));
	aStream.WriteUint32L(static_cast<TUint32>I64LOW(iPid));
	
	aStream.WriteUint32L(static_cast<TUint32>I64HIGH(iTime));
	aStream.WriteUint32L(static_cast<TUint32>I64LOW(iTime));
	
	aStream.WriteL((TUint8*)(&iContext), sizeof(TRmdArmExcInfo) );
	aStream.WriteUint32L(static_cast<TUint32>(iCrashId));
	aStream.WriteUint32L(static_cast<TCrashInfo::TCrashSource>(iCrashSource));
	
	// The real exteranlized size is the size of the buffer up to here plus the 
	// 4 bytes for the size itself
	iSize = aBuf->Size() - startBufSize + 4;
	aStream.WriteUint32L( iSize );
	}

/** 
 * Get the maximum size allowed for this object. This is needed as the object is passed
 * across the Client Server interface.
 * @return maxSize
 */
EXPORT_C TInt TCrashInfo::MaxSize()
	{
	const TInt maxSize = 256;
	return maxSize;
	}

//eof

