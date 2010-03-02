// Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include "t_crashdriver_drv.h"
#include "d_crashdriver_ldd.h"
#include "crash.h"
#include <arm_types.h>
//#include <scmonitor.h>

/**
 * Utility method to append a string to a kernel side descriptor
 */
void AppendString(TDes8& aDes, const char* aFmt, ...)
	{
	VA_LIST list;
	VA_START(list,aFmt);
	Kern::AppendFormat(aDes,aFmt,list);
	}

/**
 * DCrashDriverFactory
 * Standard export function for LDDs. This creates a DLogicalDevice derived object,
 * in this case, our DCrashDriverFactory
 */
DECLARE_STANDARD_LDD()
    {
    return new DCrashDriverFactory;
    }

/**
 * Constructor
 */
DCrashDriverFactory::DCrashDriverFactory()
    {
    // Set version number for this device
    iVersion=RKernelCrashDrv::VersionRequired();
    }
     
     
/**
  Destructor
*/
DCrashDriverFactory::~DCrashDriverFactory()
    {
    }

/**
  Second stage constructor for DKernelCrashAppFactory.
  This must at least set a name for the driver object.

  @return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DCrashDriverFactory::Install()
    {
    return SetName(&RKernelCrashDrv::Name());
    }

/**
  Return the drivers capabilities.
  Called in the response to an RDevice::GetCaps() request.

  @param aDes Descriptor into which capabilities information is to be written.
*/
void DCrashDriverFactory::GetCaps(TDes8& aDes) const
    {
    // Create a capabilities object
    RKernelCrashDrv::TCaps caps;
    caps.iVersion = iVersion;
    // Write it back to user memory
    Kern::InfoCopy(aDes,(TUint8*)&caps,sizeof(caps));
    }


/**
  Called by the kernel's device driver framework to create a Logical Channel.
  This is called in the context of the user thread (client) which requested the creation
  of the Logical Channel (e.g. through a call to RBusLogicalChannel::DoCreate).
  The thread is in a critical section.

  @param aChannel Set to point to the created Logical Channel

  @return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DCrashDriverFactory::Create(DLogicalChannelBase*& aChannel)
    {
    aChannel=new DCrashDriverChannel;
    if(!aChannel)
        return KErrNoMemory;

    return KErrNone;
    }


//
// Logical Channel
//

/**
  Constructor
*/
DCrashDriverChannel::DCrashDriverChannel() :
	iDfcQ(NULL)
    {
    // Get pointer to client thread's DThread object
    iClient=&Kern::CurrentThread();

    // Open a reference on client thread so its control block can't disappear until
    // this driver has finished with it.
    // Note, this call to Open() can't fail since it is the thread we are currently running in.
    iClient->Open();
    recurseCount = 0;
    }


/**
  Destructor
*/
DCrashDriverChannel::~DCrashDriverChannel()
    {
    if(iDfcQ)
    	iDfcQ->Destroy();
    
    // Cancel all processing that we may be doing
    //DoCancel(RKernelCrashApp::EAllRequests); not needed as of now check while debugging - praveenc
    //Close our reference on the client thread
    Kern::SafeClose((DObject*&)iClient,NULL);
    }


/**
  Second stage constructor called by the kernel's device driver framework.
  This is called in the context of the user thread (client) which requested the creation
  of the Logical Channel (e.g. through a call to RBusLogicalChannel::DoCreate())
  The thread is in a critical section.

  @param aUnit The unit argument supplied by the client to RBusLogicalChannel::DoCreate()
  @param aInfo The info argument supplied by the client to RBusLogicalChannel::DoCreate()
  @param aVer The version argument supplied by the client to RBusLogicalChannel::DoCreate()

  @return KErrNone if successful, otherwise one of the other system wide error codes.
*/
TInt DCrashDriverChannel::DoCreate(TInt /*aUnit*/, const TDesC8* /*aInfo*/, const TVersion& aVer)
    {
    if(!Kern::CurrentThreadHasCapability(ECapability_None,__PLATSEC_DIAGNOSTIC_STRING("Checked by crashdriver")))
        return KErrPermissionDenied;
     
    // Check version
    if (!Kern::QueryVersionSupported(RKernelCrashDrv::VersionRequired(),aVer))
        return KErrNotSupported;
    
    // Setup LDD for receiving client messages.
	TInt err = Kern::DynamicDfcQCreate(iDfcQ, 25, RKernelCrashDrv::Name());
	if(err != KErrNone)
		{
		Kern::Printf("Creating Dfc queue failed.");  	
		}
	SetDfcQ(iDfcQ);       
    iMsgQ.Receive();

    return KErrNone;    
    }


/**
  Called when a user thread requests a handle to this channel.
*/
TInt DCrashDriverChannel::RequestUserHandle(DThread* aThread, TOwnerType aType)
    {
    // Make sure that only our client can get a handle
    if (aType!=EOwnerThread || aThread!=iClient)
    	{    
        return KErrAccessDenied;
    	}
    return KErrNone;    
    }


/**
  Process a message for this logical channel.
  This function is called in the context of a DFC thread.

  @param aMessage The message to process.
                  The iValue member of this distinguishes the message type:
                  iValue==ECloseMsg, channel close message
                  iValue==KMaxTInt, a 'DoCancel' message
                  iValue>=0, a 'DoControl' message with function number equal to iValue
                  iValue<0, a 'DoRequest' message with function number equal to ~iValue
*/
void DCrashDriverChannel::HandleMsg(TMessageBase* aMsg)
    {    
    TThreadMessage& m=*(TThreadMessage*)aMsg;//needs checking -praveenc

    // Get message type
    TInt id=m.iValue;

    // Decode the message type and dispatch it to the relevent handler function...    
    if (id==(TInt)ECloseMsg) //check later praveenc
        {
        // Channel Close        
        m.Complete(KErrNone, EFalse);
        return;
        }   

    if (id<0)
        {
        TRequestStatus* pS=(TRequestStatus*)m.Ptr0();
        TInt r=DoRequest(~id,pS,m.Ptr1(), m.Ptr2());
        if (r==KErrNone)
            Kern::RequestComplete(iClient,pS,r);
        m.Complete(KErrNone,ETrue);
        }
    }

/**
 * Loop thread - infinite loop so we have a thread in a known state
 * @param aPtr
 * @return One of the symbian wide return codes
 */
TInt Loop(TAny* aPtr)
	{    
    TInt32 array[3];
	array[0] = 0xDEADDEAD;
	array[1] = 0xF000BEEF;
	array[2] = 0xBEEBEE00;
	
	//set registers for known state for testing
	SetRegs();
	
	//go into infinite loop
	InfLoop();
	
	return array[0]; //to avoid warning - will never get here anyway
	}

TInt StartInfLoop()
	{
    //set registers for known state for testing
    SetUpperRegs();
	
    //Launch another kernel thread
    SThreadCreateInfo info;
    info.iType = EThreadSupervisor;
    info.iFunction = Loop; 
    info.iPtr = NULL;
    info.iSupervisorStack=NULL;
    info.iSupervisorStackSize=0;    // zero means use default value
    info.iInitialThreadPriority= 25;
    info.iName.Set(_L("cdsknownthread"));
    info.iTotalSize = sizeof(info);
    
    NKern::ThreadEnterCS();
    TInt err = Kern::ThreadCreate(info);
    NKern::ThreadLeaveCS();
    
    if(err != KErrNone)
    	return err;
    
    DThread* kernTh = (DThread*)info.iHandle;
    Kern::ThreadResume(*kernTh);
    
    NKern::Sleep(NKern::TimerTicks(3000)); //wait 3 seconds to ensure our other thread has been scheduled
    
    return KErrNone;
	}

/**
  Process asynchronous requests
*/
TInt DCrashDriverChannel::DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2)
    {               
    
    
	Kern::Printf("DCrashDriverChannel::DoRequest");

    TInt err = KErrNone;   
    
    switch(aReqNo)
        {      
        case RKernelCrashDrv::EPrefetchAbort:
        	{    
        	err = StartInfLoop();  
        	if(err != KErrNone)
        		return err;
        	PrefetchAbort();
        	break;
        	}            
        case RKernelCrashDrv::EDataRead:
        	{
        	err = StartInfLoop();  
        	if(err != KErrNone)
        		return err;
        	DataRead();
            break;
        	}
        case RKernelCrashDrv::EDataWrite:
        	{
        	err = StartInfLoop();  
        	if(err != KErrNone)
        		return err;
        	DataWrite();
            break;
        	}         
        case RKernelCrashDrv::EUndefInst:
        	{
        	err = StartInfLoop();  
        	if(err != KErrNone)
        		return err;
        	UndefinedInst();
            break;
        	}            
        case RKernelCrashDrv::EDivByZero:
        	{
        	err = StartInfLoop();  
        	if(err != KErrNone)
        		return err;
        	DivByZero();
        	break;
        	}        	
        case RKernelCrashDrv::EStackOverflow:
        	{
        	err = StartInfLoop();  
        	if(err != KErrNone)
        		return err;
        	StackOverFlow();
            break;
        	}
        case RKernelCrashDrv::ETestSCMConfigInsertMethod:
        	{
        	//read the argmuents
        	HBuf8* tmp = HBuf8::New(KMaxBufferSize);
        	
        	TInt err = Kern::ThreadDesRead(iClient, a2, *tmp, 0, 0 );
			if (err != KErrNone || tmp->MaxLength() < KMaxBufferSize)
			{				
				Kern::Printf("Unable to read argument or buffer too small");

				// We could not read information from the user, so the a2 argument is probably wrong
				Kern::RequestComplete(iClient, aStatus, KErrArgument);
				return KErrArgument;
			}
        	
			//run the test and gather the results
			TBool pass;
			
			//run test
        	//TestConfigInsertL(pass, *tmp);
        	
        	//write the results back to client thread
        	err = Kern::ThreadRawWrite(iClient, a1, (TUint8*)&pass, sizeof(TBool), iClient);
        	if(err != KErrNone)
        		{
				Kern::Printf("Unable to write data back to client");

				// We could not read information from the user, so the a2 argument is probably wrong
				Kern::RequestComplete(iClient, aStatus, KErrArgument);
				return err;
        		}
        	
        	//and write the buffer back...
        	err = Kern::ThreadDesWrite(iClient, a2, *tmp, 0, 0, iClient);
        	if(err != KErrNone)
        		{
        		Kern::Printf("Unable to write data back to client");
        		
				// We could not read information from the user, so the a2 argument is probably wrong
				Kern::RequestComplete(iClient, aStatus, KErrArgument);
				return err;
        		}
        	
        	break;
        	}
        default:
        	{        	
            err=KErrNotSupported;
            break;
        	}
        }

    return err;
    }


/**
  Process cancelling of asynchronous requests //need later -praveenc
*/
void DCrashDriverChannel::DoCancel(TUint aMask)
    {
    //give definition later - praveenc
    //if(aMask&(1<<RKernelCrashApp::ESendData))
      //  SendDataCancel();
    //if(aMask&(1<<RKernelCrashApp::EReceiveData))
      //  ReceiveDataCancel();
    }

/**
 * Method to cause board to crash with a prefetch abort
 */
void DCrashDriverChannel::PrefetchAbort()
	{
    Tfunc f = NULL;
    f();
	}

/**
 * Method to cause board to read from a location we arent allowed
 */
void DCrashDriverChannel::DataRead()
	{
	Kern::Printf("Causing a data read crash");
	TInt* r = (TInt*) 0x1000;
   	TInt rr = (TInt)*r;

	// Stop compilation warning. Should not get here anyway.
	rr++;	
	
	}

/**
 * Method to cause board to crash from writing to a location we are not allowed
 */
void DCrashDriverChannel::DataWrite()
	{
	Kern::Printf("Causing a data write crash");
  	TInt* r = (TInt*) 0x1000;
   	*r = 0x42;
	}

/**
 * Method to cause board to crash from executing an undefined instruction
 */
void DCrashDriverChannel::UndefinedInst()
	{
	Kern::Printf("Executing an undefined instruction");
   	TUint32 undef = 0xE6000010;
   	Tfunc f = (Tfunc) &undef;
   	f();
	}

/**
 * Method to cause board to crash with a division by zero
 */
void DCrashDriverChannel::DivByZero()
	{
	Kern::Printf("\tDivision By zero crash...");
	int x = 5;
	int y = 5;
	for(y = 5; y>0; --y)
		{
		//this is to avoid compiler errors
		}
	x = x/y;
	}

/**
 * Method to cause board to crash with a stack overflow
 */
void DCrashDriverChannel::StackOverFlow()
	{
	Kern::Printf("\tAbout to cause a stackoverflow");
	TUint32 array[128];
	array[0] = ++recurseCount;
	StackOverFlow();
	TUint warnRem = array[0];
	array[0] = warnRem;
	}

// Testing methods for SCMConfig //

/**
 * This tests we can succesfully enter and retrieve items from our config class
 *//*
void DCrashDriverChannel::TestConfigInsertL(TBool& aPass, TDes8& aResult)
	{	
	//create our config object (We want a fresh one every time)
	SCMConfiguration* config = new SCMConfiguration();
	
	TInt32 highPriority = 200;
	TInt32 highishPriority = 150;
	TInt32 mediumPriority = 100;
	TInt32 lowishPriority = 50;
	TInt32 lowPriority = 10;
	
	TUint8 numbytes1 = 20;
	TUint8 numbytes2 = 30;
	TUint8 numbytes3 = 40;
	TUint8 numbytes4 = 50;
	TUint8 numbytes5 = 60;
	TUint8 numbytes6 = 10;
	
	config->SetConfigItem(SCMConfiguration::ECrashedProcessMetaData, highishPriority, numbytes2);
	config->SetConfigItem(SCMConfiguration::ECrashedThreadMetaData, lowPriority, numbytes5);
	config->SetConfigItem(SCMConfiguration::ECrashedThreadUsrRegisters, highPriority, numbytes1);
	config->SetConfigItem(SCMConfiguration::EExceptionStacks, mediumPriority, numbytes3);
	config->SetConfigItem(SCMConfiguration::EThreadsSvrStack, lowishPriority, numbytes4);
	config->SetConfigItem(SCMConfiguration::EProcessMetaData, lowishPriority, numbytes6);
	
	//now we make sure they come back in the right order
	TInt32 size = 0;
	SCMConfiguration::TSCMDataType type;
	
	//for each item, ensure that the expected size and type is correct
	type = config->GetNextItemToDump(size);
	aPass = (type == SCMConfiguration::ECrashedThreadUsrRegisters ) && (size == numbytes1);
		
	if(!aPass)
		{
		delete config;
		AppendString(aResult, "Expected ECrashedThreadUsrRegisters but got %d and expected size to be %d but got %d", type, numbytes1, size );
		return;
		}
	
	type = config->GetNextItemToDump(size);
	aPass = (type == SCMConfiguration::ECrashedProcessMetaData ) && (size == numbytes2);
	
	if(!aPass)
		{
		delete config;
		AppendString(aResult, "Expected ECrashedProcessMetaData but got %d and expected size to be %d but got %d", type, numbytes2, size );
		return;
		}
	
	type = config->GetNextItemToDump(size);
	aPass = (type == SCMConfiguration::EExceptionStacks ) && (size == numbytes3);
	
	if(!aPass)
		{
		delete config;
		AppendString(aResult, "Expected EExceptionStacks but got %d and expected size to be %d but got %d", type, numbytes3, size);
		return;
		}
	
	//these next 2 went in at the same priority but the last one that goes in will be retrieved first
	type = config->GetNextItemToDump(size);
	aPass = (type == SCMConfiguration::EProcessMetaData ) && (size == numbytes6);
	
	if(!aPass)
		{
		delete config;
		AppendString(aResult, "Expected EProcessMetaData but got %d and expected size to be %d but got %d", type, numbytes6, size);
		return;
		}
	
	type = config->GetNextItemToDump(size);
	aPass = (type == SCMConfiguration::EThreadsSvrStack ) && (size == numbytes4);
	
	if(!aPass)
		{
		delete config;
		AppendString(aResult, "Expected EThreadsSvrStack but got %d and expected size to be %d but got %d", type, numbytes4, size);
		return;
		}
	
	type = config->GetNextItemToDump(size);
	aPass = (type == SCMConfiguration::ECrashedThreadMetaData ) && (size == numbytes5);
	
	if(!aPass)
		{
		delete config;
		AppendString(aResult, "Expected ECrashedThreadMetaData but got %d and expected size to be %d but got %d", type, numbytes5, size);
		return;
		}
	
	//if we get here, test has passed
	aResult.Zero();
	aResult.SetLength(50);
	AppendString(aResult, "TestConfigInsertL passed");
	aPass = ETrue;
	
	
	delete config;
	}*/

//eof d_crashdriver_ldd.cpp








