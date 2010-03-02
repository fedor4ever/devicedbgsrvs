/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __TRKXTICOMM_H__
#define __TRKXTICOMM_H__

//
// class TCapsTrkXtiDriver
//
class TCapsTrkXtiDriver
{
public:
	TVersion	iVersion;
};




//
// class RTrkXtiDriver
//
class RTrkXtiDriver : public RBusLogicalChannel
{
public:

	enum TControl
	{
		EControlWrite = 0,
	};
	
	enum TRequest
	{
		ERequestRead=0x0, ERequestReadCancel=0x1,
		ERequestWrite=0x1,ERequestWriteCancel=0x2,
	};	
		
public:

	inline TInt Open();
	inline void Read(TRequestStatus &aStatus, TDes8 &aDes);
	inline void Read(TRequestStatus &aStatus, TDes8 &aDes, TInt aLength);
	inline void ReadOneOrMore(TRequestStatus &aStatus, TDes8 &aDes);
	inline void ReadCancel();
	inline void Write(TRequestStatus &aStatus, const TDesC8 &aDes);
	inline void Write(TRequestStatus &aStatus, const TDesC8 &aDes, TInt aLength);
	inline TInt Write(const TDesC8 &aDes);
	inline TInt Write(const TDesC8 &aDes, TInt aLength);
	inline void WriteCancel();

};


_LIT(KTrkXtiDriverName,"TrkXti Driver");

// Version information
const TInt KMajorXtiVersionNumber=1;
const TInt KMinorXtiVersionNumber=0;
const TInt KBuildXtiVersionNumber=0;


inline TInt RTrkXtiDriver::Open()
{
	#ifdef EKA2
	return DoCreate(KTrkXtiDriverName, TVersion(KMajorXtiVersionNumber, KMinorXtiVersionNumber, KBuildXtiVersionNumber), KNullUnit, NULL, NULL);
	#else
	return DoCreate(KTrkXtiDriverName, TVersion(KMajorXtiVersionNumber, KMinorXtiVersionNumber, KBuildXtiVersionNumber), NULL, KNullUnit, NULL, NULL);
	#endif
}

inline void RTrkXtiDriver::Read(TRequestStatus &aStatus, TDes8 &aDes)
{
	TInt len = aDes.MaxLength();
	DoRequest(ERequestRead, aStatus, &aDes, &len);
}

inline void RTrkXtiDriver::Read(TRequestStatus &aStatus, TDes8 &aDes, TInt aLength)
{
	DoRequest(ERequestRead, aStatus, &aDes, &aLength);
}

inline void RTrkXtiDriver::ReadOneOrMore(TRequestStatus &aStatus, TDes8 &aDes)
{
	TInt len = (-aDes.MaxLength());
	DoRequest(ERequestRead, aStatus, &aDes, &len);
}

inline void RTrkXtiDriver::ReadCancel()
{
	DoCancel(ERequestReadCancel);
}

inline void RTrkXtiDriver::Write(TRequestStatus &aStatus, const TDesC8 &aDes)
{ 
	TInt len=aDes.Length();
	DoRequest(ERequestWrite, aStatus, (TAny *)&aDes, &len);
}

inline void RTrkXtiDriver::Write(TRequestStatus &aStatus, const TDesC8 &aDes, TInt aLength)
{
	DoRequest(ERequestWrite, aStatus, (TAny *)&aDes, &aLength);
}

inline TInt RTrkXtiDriver::Write(const TDesC8 &aDes)
{ 
	TInt len=aDes.Length();
	return DoControl(EControlWrite, reinterpret_cast<TAny*>(len), (TAny*)&aDes);
}

inline TInt RTrkXtiDriver::Write(const TDesC8 &aDes, TInt aLength)
{
	return DoControl(EControlWrite, reinterpret_cast<TAny*>(aLength), (TAny*)&aDes);
}

inline void RTrkXtiDriver::WriteCancel()
{
	DoCancel(ERequestWriteCancel);
}


#endif // __TRKXTICOMM_H__
