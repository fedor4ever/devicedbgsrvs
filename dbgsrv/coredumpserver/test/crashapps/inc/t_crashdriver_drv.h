// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Core Dump Server Crash Driver
//



/**
 @file
 @internalTechnology
 @prototype
*/

#ifndef __T_CRASHDRIVER_DRV_H__
#define __T_CRASHDRIVER_DRV_H__

#include <e32cmn.h>
#include <e32ver.h>
#ifndef __KERNEL_MODE__
#include <e32std.h>
#endif

//size of message buffer
const TInt KMaxBufferSize = 150;

/**
User interface for 'crashdriver'
*/
class RKernelCrashDrv : public RBusLogicalChannel
    {
public:
	/**
    Structure for holding driver capabilities information
    (Just a version number in this example.)
    */
    class TCaps
        {
    public:
        TVersion iVersion;
        };
    
public:
    TInt Open();   
    void SendNullPointerFault(TRequestStatus& aStatus);
    void SendPrefetchAbortFault(TRequestStatus& aStatus);
    void SendDataReadFault(TRequestStatus& aStatus);
    void SendDataWriteFault(TRequestStatus& aStatus);  
    void SendUndefInstructionFault(TRequestStatus& aStatus);
    void SendDivByZeroFault(TRequestStatus& aStatus);
    void SendStackOverFlowFault(TRequestStatus& aStatus);
    void TestSCMConfigInsertMethod(TRequestStatus& aStatus, TBool& aPass, TDes8& aBuf);
    
    inline static const TDesC& Name();
    inline static TVersion VersionRequired();

public:
     /**
    Enumeration of Request messages. Causes kernel Panics
    */
    enum TRequest
        {
        ENullPointer,
        EPrefetchAbort,
        EDataRead,
        EDataWrite,
        EUndefInst,
        EDivByZero,
        EStackOverflow,
        ETestSCMConfigInsertMethod,
        };      
    };


/**
  Returns the driver's name
*/
inline const TDesC& RKernelCrashDrv::Name()
    {
    _LIT(KCrashDriverName,"crashdriver");
    return KCrashDriverName;
    }

/**
  Returns the version number of the driver
*/
inline TVersion RKernelCrashDrv::VersionRequired()
    {
    const TInt KMajorVersionNumber=1;
    const TInt KMinorVersionNumber=0;
    const TInt KBuildVersionNumber=KE32BuildVersionNumber;
    return TVersion(KMajorVersionNumber,KMinorVersionNumber,KBuildVersionNumber);
    }

/*
  NOTE: The following member functions would normally be exported from a seperate client DLL
  but are included inline in this header file for convenience.
*/

#ifndef __KERNEL_MODE__

/**
  Opens a logical channel to the driver

  @return One of the system wide error codes.
*/
TInt RKernelCrashDrv::Open()
    {
    __UHEAP_MARK;
    TInt x = DoCreate(Name(),VersionRequired(),KNullUnit,NULL,NULL,EOwnerThread);    
    __UHEAP_MARKEND;
    return x;
    }


/** 
 *   Sends data to the device.
 *   Only one send request may be pending at any time.
 *   @param aStatus The request to be signalled when the data has been sent.
 *                  The result value will be set to KErrNone on success;
 *                  or set to one of the system wide error codes when an error occurs.
 */
void RKernelCrashDrv::SendPrefetchAbortFault(TRequestStatus& aStatus)
    {
    DoRequest(EPrefetchAbort,aStatus);
    }

/**
 *   Sends data to the device.
 *   Only one send request may be pending at any time.
 *   @param aStatus The request to be signalled when the data has been sent.
 *                  The result value will be set to KErrNone on success;
 *                  or set to one of the system wide error codes when an error occurs.
 */
void RKernelCrashDrv::SendDataReadFault(TRequestStatus& aStatus)
    {
    DoRequest(EDataRead,aStatus);
    }

/**
 *   Sends data to the device.
 *   Only one send request may be pending at any time.
 *   @param aStatus The request to be signalled when the data has been sent.
 *                  The result value will be set to KErrNone on success;
 *                  or set to one of the system wide error codes when an error occurs.
 */
void RKernelCrashDrv::SendDataWriteFault(TRequestStatus& aStatus)
    {
    DoRequest(EDataWrite,aStatus);
    }

/**
 *   Sends data to the device.
 *   Only one send request may be pending at any time.
 *   @param aStatus The request to be signalled when the data has been sent.
 *                  The result value will be set to KErrNone on success;
 *                  or set to one of the system wide error codes when an error occurs.
 */
void RKernelCrashDrv::SendUndefInstructionFault(TRequestStatus& aStatus)
    {
    DoRequest(EUndefInst,aStatus);
    }

/**
 *   Sends data to the device.
 *   Only one send request may be pending at any time.
 *   @param aStatus The request to be signalled when the data has been sent.
 *                  The result value will be set to KErrNone on success;
 *                  or set to one of the system wide error codes when an error occurs.
 */
void RKernelCrashDrv::SendDivByZeroFault(TRequestStatus& aStatus)
    {
    DoRequest(EDivByZero,aStatus);
    }

/**
 *   Sends data to the device.
 *   Only one send request may be pending at any time.
 *   @param aStatus The request to be signalled when the data has been sent.
 *                  The result value will be set to KErrNone on success;
 *                  or set to one of the system wide error codes when an error occurs.
 */
void RKernelCrashDrv::SendStackOverFlowFault(TRequestStatus& aStatus)
    {
    DoRequest(EStackOverflow,aStatus);
    }

/**
 *   Sends data to the device.
 *   Only one send request may be pending at any time.
 *   @param aStatus The request to be signalled when the data has been sent.
 *                  The result value will be set to KErrNone on success;
 *                  or set to one of the system wide error codes when an error occurs.
 */
void RKernelCrashDrv::TestSCMConfigInsertMethod(TRequestStatus& aStatus, TBool& aPass, TDes8& aBuf)
    {
    DoRequest(ETestSCMConfigInsertMethod, aStatus, reinterpret_cast<TAny*>(&aPass), reinterpret_cast<TAny*>(&aBuf));
    }



#endif   // !__KERNEL_MODE__

#endif

