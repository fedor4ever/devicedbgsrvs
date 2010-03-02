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
// Kernel side interfaces to example Logical Device Drivers
//



#ifndef __D_CRASHDRIVER_LDD_H__
#define __D_CRASHDRIVER_LDD_H__


#include <kernel\kern_priv.h>

/**
 * Logical Device (factory class) for 'crashdriver'
 */
class DCrashDriverFactory : public DLogicalDevice
    {
public:    
	DCrashDriverFactory();
    ~DCrashDriverFactory();
    //  Inherited from DLogicalDevice
    virtual TInt Install();
    virtual void GetCaps(TDes8& aDes) const;
    virtual TInt Create(DLogicalChannelBase*& aChannel);
    };
    

/**
 *  Logical Channel class for 'crashdriver'
 */    
class DCrashDriverChannel : public DLogicalChannel
    {
public:
	DCrashDriverChannel();
    virtual ~DCrashDriverChannel();
    // Inherited from DObject
    virtual TInt RequestUserHandle(DThread* aThread, TOwnerType aType);
    // Inherited from DLogicalChannelBase
    virtual TInt DoCreate(TInt aUnit, const TDesC8* anInfo, const TVersion& aVer);
    // Inherited from DLogicalChannel
    virtual void HandleMsg(TMessageBase* aMsg);
private:
    // Panic reasons
    enum TPanic
        {
        ERequestAlreadyPending = 1
        };
    // Implementation for the differnt kinds of requests send through RBusLogicalChannel
    TInt DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2);
    void DoCancel(TUint aMask); 
    
    void PrefetchAbort();
    void DataRead();
    void DataWrite();
    void UndefinedInst();
    void DivByZero();
    void StackOverFlow();
    
    //void TestConfigInsertL(TBool& aPass, TDes8& aResult);
    
    typedef void (*Tfunc)();
   
private:
	TDynamicDfcQue* iDfcQ;
    DThread* iClient;
    TRequestStatus* iStatus;
    TInt recurseCount;
   
    };    

#endif

