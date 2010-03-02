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
//

#include <e32base.h>    //base stuff
#include <e32debug.h>   //logging
#include <e32cons.h>    //console
#include <e32property.h>
#include <e32utrace.h>
#include <d32btrace.h>
#include <uloggerclient.h>
#include "t_crashdriver_drv.h"
#include "crash.h"
#include "crashdefs.h"//definitions included

_LIT(KAlloc, "-a <num_of_kbytes> : memory to be allocated on the heap; default is zero\n");
_LIT(KChunks, "-n <num_of_chunks> :  first global - 1k, the rest 0.5k are local to the process; default is none\n");
_LIT(KDelay, "-d <num_of_seconds> : time before the app crashes; default is 60s\n");
_LIT(KFault, "-c <fault_type> : crash type; default is null pointer dereference\n");
_LIT(KFaultTypes, "None=0, PrefetchAbort=1, DataRead=2\n DataWrite=3, UndefInstr=4, UserPanic=5\n UserException=6, EStackOverflow=7, UserExit=8\n UserLeave=9, ThreadKill=10, ThreadPanic=11\n ThreadTerminate=12, DivByZero=13\n");
_LIT(KLoad, "-l : load esock.dll library into process address space; default is no\n");
_LIT(KSimon, "-s : to create a dummy child thread that is not going to crash; default is no\n");
_LIT(KThreads, "-m <num_of_threads> : that are going to be created and then crashed; if not specified the main thread is going to crash\n");
_LIT(KTrace, "-t: includes test trace data\n");
_LIT(KKernelCrash, "-k: causes the crash to occur in a device driver\n");
_LIT(KCrashDriverLddFileName, "crashdriverldd");

//crash app trace filter
#define CrashAppTraceFilter 200

/** Crash App UID */
const TUid KCrashAppUid = { 0x102831E5 };

using namespace Ulogger;

enum TFaultType
{
    ENone			=0,
    EPrefetchAbort	=1,
    EDataRead		=2,
    EDataWrite		=3,
    EUndefInstr		=4,
    EUserPanic		=5,
    EUserException	=6,
    EStackOverflow	=7,

    EUserExit		=8,
    EUserLeave		=9,
    EThreadKill		=10,
    EThreadPanic	=11,
    EThreadTerminate=12,
    EDivByZero		=13,

};

enum TProgress 
{ 
	EMainThread, 	
	EChildThread	
};

CConsoleBase *console;

_LIT(KPanicCategory, "example panic category");

TInt gAlloc = 0;
TInt gDelay = 60; 
TBool gLibrary = EFalse;
TInt gChunks = 0;
TInt gMultikill = 0;
TBool gScreamer = EFalse;
TFaultType gFault = ENone;
TBool gTrace = EFalse;
TBool gKernelSide = EFalse;

void WriteTraceData()
	{	
	RBTrace trace;
    TInt e = trace.Open();
    if(e != KErrNone)
    	{
    	RDebug::Printf("crashapp: opening trace buffer failed: %d\n", e);
    	trace.Close();
    	return;
    	}
    RDebug::Printf("crashapp: opening trace buffer succesful\n");
    
    trace.SetFilter(200, ETrue);
    trace.SetFilter2(1);
    trace.SetMode(RBTrace::EEnable);
	
	//do some tracing
	TPrimaryFilter prim = KTraceCategory; //arbitrary once between 192 and 253
	RDebug::Printf("crashapp: making trace calls");
	User::After(15000000);
	for(TInt i=0; i<KTRACENUMBER; i++)
		{				
		TUTrace::PrintfPrimary(prim, ETrue, ETrue, "crashapp trace call number %d", i);
		}
	
	
	
	trace.Close();		
	}

typedef void (*Tfunc)();

void PrefetchAbort()
{
    Tfunc f = NULL;
    f();
}

void DataRead()
{
  	TInt* r = (TInt*) 0x1000;
   	TInt rr = (TInt)*r;

	// Stop compilation warning. Should not get here anyway.
	rr++;
}

void DataWrite()
{
  	TInt* r = (TInt*) 0x1000;
   	*r = 0x42;                
}

void UndefInstr()
{
   	TUint32 undef = 0xE6000010;
   	Tfunc f = (Tfunc) &undef;
   	f();
}

void DivByZero()
{
    int i = 1;
    int j = 10;
    for(j=10; j>0; --j)
    	{
    	//this is here to avoid compiler div by zero warnings 
    	}
    i = i/j;
}

void UserPanic()
{
    User::Panic(KPanicCategory, KErrGeneral); 
}

void UserException()
{
    User::RaiseException(EExcGeneral);
}

void UserExit()
{
    User::Exit(KErrGeneral);
}

void UserLeaveL()
{
    User::Leave(KErrGeneral);
}

void ThreadKill()
{
    RThread thread;
    thread.Kill(KErrNone);
}

void ThreadPanic()
{
    RThread thread;
    thread.Panic(KPanicCategory, KErrGeneral);
}

void ThreadTerminate()
{
    RThread thread;
    thread.Terminate(KErrGeneral);
}

static TUint recurseCount;
void StackOverflow()
{
	TUint32 array[128];
	array[0] = ++recurseCount;
    StackOverflow();
	TUint warnRem = array[0];
	array[0] = warnRem;
}

void ParseCommandlineArgsL()
{
    TInt argc = User::CommandLineLength();

    if(argc > 0)
    {
	    HBufC* args = HBufC::NewLC(User::CommandLineLength());
        TPtr argv = args->Des();
	    User::CommandLine(argv);

	    TLex lex(*args);

        while(!lex.Eos())
        {
            if(lex.Get() == '-')
            {
                TChar c = lex.Get();
                if(c == '-')
                {
                    TPtrC16 token = lex.NextToken();
                    c = token[0];
                }

                lex.SkipSpace();
                switch(c)
                {
                    case 'a':
                        lex.Val(gAlloc);
                        break;
                    case 'd':
                        lex.Val(gDelay);
                        break;
                    case 'n':
                        lex.Val(gChunks);
                        break;
                    case 'c':
                        lex.Val((TInt&)gFault);
                        break;
                    case 'l':
                        gLibrary = ETrue;
                        break;
                    case 'm':
                        lex.Val(gMultikill);
                        break;
                    case 's':
                        gScreamer = ETrue;
                        break;
                    case 't':
                    	gTrace = ETrue;
                    	break;
                    case 'k':
                    	gKernelSide = ETrue;
                    	break;
                    case 'h':
                    default:
                        _LIT(KParams, "full parameter list:\n");
                        console->Printf(KParams);

                        console->Printf(KAlloc);
                        console->Printf(KChunks);
                        console->Printf(KDelay);
                        console->Printf(KFault);
						console->Printf(KFaultTypes);
                        console->Printf(KLoad);
                        console->Printf(KSimon);
                        console->Printf(KThreads);
                        console->Printf(KTrace);
                        console->Printf(KKernelCrash);
                        
                        _LIT(KPressAnyKey,"[press any key]");
                        console->Printf(KPressAnyKey);
	                    console->Getch();
                        User::Leave(KErrNone);
                }
            }
            lex.SkipSpace();
        }
	    CleanupStack::PopAndDestroy(args);  
    }
}

TInt ThreadSimon(TAny* /*aCall*/)
{
    RThread thread;
    TUint simonCounter = 1;
    while(simonCounter++)
    {
        User::After(1000000);
        RDebug::Printf("crashapp.exe - Simon[%Lu] says...%d\n", thread.Id().Id(), simonCounter);
    }
    return 0;
}

TInt ThreadCrash(TAny* aCall)
{
    User::After(gDelay*1000000);
    Tfunc call = (Tfunc)(aCall);
    RThread thread;
    RDebug::Printf("crashapp.exe - time to die[%Lu]!\n", thread.Id().Id());
    call();
    return 0;
}

void MainL()
{
	//Setup a property to monitor main thread    
	static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy);
	TInt err = RProperty::Define(KCrashAppUid, EMainThread, RProperty::EInt, KAllowAllPolicy, KAllowAllPolicy);
	if (err != KErrAlreadyExists)
	    {
	    User::LeaveIfError(err);
	    }

    Tfunc call = NULL;
	recurseCount = 0;

    _LIT(KConsole, "crash console");
    console = Console::NewL(KConsole, TSize(KConsFullScreen, KConsFullScreen));
    CleanupStack::PushL(console);          

    ParseCommandlineArgsL();
    
    if(gTrace)
    	{
    	WriteTraceData();
    	}

    CleanupStack::PopAndDestroy(console);
    
    //Load crash driver
    RKernelCrashDrv crashDriver;
    if(gKernelSide)
    	{
    	err = User::LoadLogicalDevice(KCrashDriverLddFileName);
    	if(KErrNone != err && KErrAlreadyExists != err )
    		{
    		RDebug::Printf("Unable to load kernel crash driver (%d)", err);
    		User::Leave(err);
    		}
    	CleanupClosePushL(crashDriver);
    	RDebug::Printf("Crash Driver loaded");
    	
    	err = crashDriver.Open();
    	if(KErrNone != err)
    		{
    		RDebug::Printf("Unable to open kernel crash driver (%d)", err);
    		User::Leave(err);
    		}
    	RDebug::Printf("Crash Driver opened");
    	
    	}        

    TInt32 array[3];
	array[0] = 0xDEADDEAD;
	array[1] = 0xF000BEEF;
	array[2] = 0xBEEBEE00;
    
    RDebug::Printf("crashapp.exe will crash after %d secs, crash type: ", gDelay);
    TRequestStatus status;
    switch(gFault)
    {
        default:
        case ENone:
        	{
            RDebug::Printf("null pointer\n");        	             
            break;
        	}
        case EPrefetchAbort:
        	{
            RDebug::Printf("prefetch abort\n");
        	if(gKernelSide)
	    		{
	    		User::After(gDelay*1000000);
#ifdef __MARM__	    		
	    		SetRegs();
#endif	    		
	    		crashDriver.SendPrefetchAbortFault(status);
	    		User::WaitForRequest(status);
	    		err = status.Int();
	    		if(KErrNone != err)
	    			{
	    			RDebug::Printf("Send request failed (%d)", err);
	    			User::Leave(err);
	    			}
	    		break;
	    		}              
            call = PrefetchAbort;
            break;
        	}
        case EDataRead:
        	{
            RDebug::Printf("data read");
        	if(gKernelSide)
	    		{
	    		User::After(gDelay*1000000);
#ifdef __MARM__	    		
	    		SetRegs();
#endif	    	
	    		crashDriver.SendDataReadFault(status);
	    		User::WaitForRequest(status);
	    		err = status.Int();
	    		if(KErrNone != err)
	    			{
	    			RDebug::Printf("Send request failed (%d)", err);
	    			User::Leave(err);
	    			}
	    		break;
	    		}            
            call = DataRead;
            break;
        	}            
        case EDataWrite:
        	{
            RDebug::Printf("data write");
        	if(gKernelSide)
	    		{
	    		User::After(gDelay*1000000);
#ifdef __MARM__	    		
	    		SetRegs();
#endif	    		
	    		crashDriver.SendDataWriteFault(status);
	    		User::WaitForRequest(status);
	    		err = status.Int();
	    		if(KErrNone != err)
	    			{
	    			RDebug::Printf("Send request failed (%d)", err);
	    			User::Leave(err);
	    			}
	    		break;
	    		}            
            call = DataWrite;
            break;
        	}
        case EUndefInstr:
        	{        	
        	RDebug::Printf("Undefined instruction");
        	if(gKernelSide)
	    		{
	    		User::After(gDelay*1000000);
#ifdef __MARM__	    		
	    		SetRegs();
#endif	    		
	    		crashDriver.SendUndefInstructionFault(status);
	    		User::WaitForRequest(status);
	    		err = status.Int();
	    		if(KErrNone != err)
	    			{
	    			RDebug::Printf("Send request failed (%d)", err);
	    			User::Leave(err);
	    			}
	    		break;
	    		}
            RDebug::Printf("undefined instruction");
            call = UndefInstr;
            break;
        	}        
        case EDivByZero:
        	{
        	RDebug::Printf("Division by zero");
        	if(gKernelSide)
        		{
        		User::After(gDelay*1000000);
#ifdef	__MARM__        		
        		SetRegs();
#endif        		
        		crashDriver.SendDivByZeroFault(status);
        		User::WaitForRequest(status);
        		err = status.Int();
        		if(KErrNone != err)
        			{
        			RDebug::Printf("Send request failed (%d)", err);
        			User::Leave(err);
        			}
        		break;
        		}
        	
            RDebug::Printf("division by zero");
            call = DivByZero;
            break;
        	}        
        case EUserPanic:
        	{        	
            RDebug::Printf("user panic");
        	if(gKernelSide)
        		{
        		RDebug::Printf("This crash is not supported kernel side");
        		User::Leave(KErrNotSupported);
        		}            
            call = UserPanic;
            break;
        	}
        case EUserException:
        	{
            RDebug::Printf("user exception\n");
        	if(gKernelSide)
        		{
        		RDebug::Printf("This crash is not supported kernel side");
        		User::Leave(KErrNotSupported);
        		}            
            call = UserException;
            break;
        	}
        case EUserExit:
        	{
            RDebug::Printf("user exit\n");
        	if(gKernelSide)
        		{
        		RDebug::Printf("This crash is not supported kernel side");
        		User::Leave(KErrNotSupported);
        		}            
            call = UserExit;
            break;
        	}
        case EUserLeave:
        	{
            RDebug::Printf("user leave\n");
        	if(gKernelSide)
        		{
        		RDebug::Printf("This crash is not supported kernel side");
        		User::Leave(KErrNotSupported);
        		}            
            call = UserLeaveL;
            break;
        	}
        case EThreadKill:
        	{
            RDebug::Printf("thread kill\n");
        	if(gKernelSide)
        		{
        		RDebug::Printf("This crash is not supported kernel side");
        		User::Leave(KErrNotSupported);
        		}            
            call = ThreadKill;
            break;
        	}
        case EThreadPanic:
        	{
            RDebug::Printf("thread panic\n");
        	if(gKernelSide)
        		{
        		RDebug::Printf("This crash is not supported kernel side");
        		User::Leave(KErrNotSupported);
        		}            
            call = ThreadPanic;
            break;
        	}
        case EThreadTerminate:
        	{
            RDebug::Printf("thread terminate\n");
        	if(gKernelSide)
        		{
        		RDebug::Printf("This crash is not supported kernel side");
        		User::Leave(KErrNotSupported);
        		}            
            call = ThreadTerminate;
            break;
        	}
        case EStackOverflow:
        	{
            RDebug::Printf("stack overflow");
        	if(gKernelSide)
        		{
#ifdef __MARM__        		
        		SetRegs();
#endif        		
        		crashDriver.SendStackOverFlowFault(status);
        		User::WaitForRequest(status);
        		err = status.Int();
        		if(KErrNone != err)
        			{
        			RDebug::Printf("Send request failed (%d)", err);
        			User::Leave(err);
        			}
        		break;
        		}            
            call = StackOverflow;
            break;
        	}
    }
    
    if(gKernelSide) 
    	{    	
    	CleanupStack::PopAndDestroy(&crashDriver);
    	err = User::FreeLogicalDevice(RKernelCrashDrv::Name());
    	if(KErrNone != err)
    		{
    		RDebug::Printf("Unable to free kernel crash driver");
    		User::Leave(err);
    		}
    	}

    if(gLibrary)
    {
        _LIT(KDllName, "esock.dll");
        RLibrary dll;
        TInt err = dll.Load(KDllName);
    }

    for(TInt i = 0; i < gChunks; i++)
    {
        RChunk chunk;

        if(!i)
        {
            _LIT(KCrashChunk, "crashchunk");
            RDebug::Printf("crashapp.exe - creating global chunk\n");
            TInt err = chunk.CreateGlobal(KCrashChunk, 1024, 4096);
            TUint8 data = 0xCA;
            TUint8 *ptr = chunk.Base();
            for(TInt i = 0; i < 1024; i++)
                ptr[i] = data; 
        }
        else
        {
            RDebug::Printf("crashapp.exe - creating global chunk:%d\n", i);
            TInt err = chunk.CreateLocal(512, 1024);
            TUint8 data = 0xBA;
            TUint8 *ptr = chunk.Base();
            for(TInt i = 0; i < 512; i++)
                ptr[i] = data; 
        }
    }

    RDebug::Printf("crashapp.exe - allocating %d kbytes of heap space", gAlloc); 
    if(gAlloc)
    {
        TAny *memory = User::Alloc(gAlloc*1024);
        if(!memory)
        {
            RDebug::Printf("crashapp.exe - unable to allocate memory on the heap!!!\n");
        }
        else
        {
            TUint8 data = 0xDA;
            TUint8 *ptr = (TUint8*)memory;
            for(TInt i = 0; i < gAlloc*1024; i++)
                ptr[i] = data;
        }
    }

    if(gScreamer)
    {
        //screaming thread just
        _LIT(KThreadSimon, "SimonTheFirst");
        RThread threadSimon;
        threadSimon.Create(KThreadSimon(), ThreadSimon, KDefaultStackSize, NULL, (TAny*)NULL);
        threadSimon.Resume();
        threadSimon.Close();
    }


    if(gMultikill == 0) //main thread crashes, no child threads
    {
        User::After(gDelay*1000000); //crash timeout
        RDebug::Printf("crashapp.exe - main thread says...\n");
        call();
    }
    else //child threads crashing
    {
        RDebug::Printf("crashapp.exe - child threads crashing every second...\n");
        _LIT(KCrashThread, "crashthread%d");
        TBuf<36> threadName;
        for(TInt i = 0; i < gMultikill; i++)
        {
            RThread crashThread;
            threadName.Format(KCrashThread, i);
            crashThread.Create(threadName, ThreadCrash, KDefaultStackSize, NULL, (TAny*)call);
            crashThread.Resume();
            crashThread.Close();
            User::After(1000000);
        }
    }
    
    RThread thread;
    TUint mainCounter = 1;
    while(mainCounter++) //main thread keep alive signals
    {
        User::After(300000);
        RDebug::Printf("main thread[%Lu] loop:%d\n", thread.Id().Id(), mainCounter);
        RDebug::Printf("setting property to %d: ", mainCounter);
        err = RProperty::Set(KCrashAppUid, EMainThread, mainCounter);
        if(err != KErrNone)
            {
            RDebug::Printf("Failed to set RProperty in main thread! err:%d\n", err);
            }              
    }
    delete console;
    
    
    if(array[0] > 1) //remove warning
    	return;
}

TInt E32Main()
{
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();
	_LIT(KPanicCleanup, "CRASH-NO-CLEANUP");
	__ASSERT_ALWAYS(cleanup, User::Panic(KPanicCleanup, KErrNoMemory));

	TRAPD(err, MainL());
	_LIT(KPanicLeave, "CRASH-LEAVE");
 	__ASSERT_ALWAYS(err == KErrNone, User::Panic(KPanicLeave, err));

	delete cleanup;
	__UHEAP_MARKEND;

	return err;
}
