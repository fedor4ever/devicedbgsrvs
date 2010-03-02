// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// $Change: $
//

#include <e32cons.h>
#include <e32debug.h>
#include <e32test.h>

#include <plugindata.h>
#include <threaddata.h>
#include <processdata.h>
#include <executabledata.h>

#include <coredumpinterface.h>

#include <optionconfig.h>
#include <debuglogging.h>

LOCAL_C void doTestL();   // the test code

GLDEF_C TInt E32Main() // main function called by E32
    {
	__UHEAP_MARK;
	// Because this is not an EIKON program we have to create our
	// own clean-up stack.
	CTrapCleanup* cleanup=CTrapCleanup::New();

	__UHEAP_MARK;

	// Trap any leaves that occur during test code
	TRAPD(error,doTestL());
	RDebug::Printf( "TRAPD(error,doTestL())=%d;\n", error);
	// Handle any leaves that occured during the test code.
	// Since this is a test program, we don't need any special error handling to
	// handle leaves - just panic the test program.
	
	_LIT(KTxtTrainingTest,"Crash Configuration Tester");	

	RDebug::Printf( "__ASSERT_ALWAYS(!error,User::Panic(KTxtTrainingTest,error));\n");
	__ASSERT_ALWAYS(!error,User::Panic(KTxtTrainingTest,error));


	RDebug::Printf( "__UHEAP_MARKEND; before deleting cleanup stack\n");
	__UHEAP_MARKEND; // Check the test program doesn't leak memory.

	RDebug::Printf( "delete cleanup;\n");
	delete cleanup;

	RDebug::Printf( "__UHEAP_MARKEND; before returning\n");
	__UHEAP_MARKEND; // Check the test program doesn't leak memory.

	RDebug::Printf( "__UHEAP_CHECK(0);\n");
    __UHEAP_CHECK(0);

	RDebug::Printf( "E32Main returning KErrNone; \n");
	return KErrNone;
	}



_LIT(KCrashAppFileName,"z:\\Sys\\Bin\\crashapP.exe");

#define KListGranularity (20)

LOCAL_C void doTestL()
	{

	// Create an RTest object for performing the tests.
	_LIT(KTestTitle, "Crash Config - Text UI");
	RTest test(KTestTitle);
	CleanupClosePushL(test);
		
	_LIT(KFormatString1,"Connecting to Server (correct result = KErrNone)");
	test.Start( KFormatString1 );

	RDebug::Printf( "__UHEAP_MARK; //before CCrashConfig::NewL( );\n");
	__UHEAP_MARK; // crashConfig

    RCoreDumpSession iSess;

	RDebug::Printf( "__UHEAP_MARK; //after CCrashConfig::NewL( );\n");
	__UHEAP_MARK; // crashConfig

	// Now connect to the server.
	test( iSess.Connect() == KErrNone );
    RDebug::Printf("connected\n");


	_LIT(KMainMenu, "\n Menu: \n  0: Save/Load Config\t 1:List Plugins\t2:Load Plugin\n a: List Plugin Infos\tb: Unload Plugin\tc:Bind Plugins\n 3:List Threads\t4:List Procs\t5:List Exes\n 6:Observe Proc by Name\t 7 : Leave Proc by Name\t 8 : Configure System\t 9 : Create Crashing Proc\n");
	_LIT(KConfigMenu,     "\n            0: Save Config\t 1: Load Config\n");
	_LIT(KProcObserveMenu, "\n            Type Process ID to observe:\n");
	
	TChar c;
	TBool finish = EFalse;
	TInt ret = KErrNone;

	_LIT( KSaveConfigMsg,		"Save Current Configuration to File" );
	_LIT( KLoadConfigMsg,		"Load Configuration From File" );


	_LIT( KGetThreadMsg,		"Request Thread List" );
	_LIT( KGetProcMsg,			"Request Proc List" );
	_LIT( KGetExeMsg,			"Request Executable List" );
	
	_LIT( KObserveProcName,	    "Observe a Process by Name" );
	_LIT( KLeaveProcName,	    "Leave a Process by Name" );


	RPluginList pluginsList;
	pluginsList.Reserve(KListGranularity);

	RThreadPointerList threadPtrList;
	threadPtrList.Reserve(KListGranularity);

	RProcessPointerList procPtrList;
	procPtrList.Reserve(KListGranularity);	

	RExecutablePointerList executablePtrList;
	executablePtrList.Reserve(KListGranularity);	

    RPluginPointerList formatterPtrList;
    formatterPtrList.Reserve(KListGranularity);

    RPluginPointerList writerPtrList;
    writerPtrList.Reserve(KListGranularity);

	do
		{
		test.Printf(KMainMenu);
		c=test.Getch();

		switch(c)
			{

			case('0'):
				{
				test.Printf(KConfigMenu);
				c=test.Getch();
				if( c == '0' )
					{
					test.Next(KSaveConfigMsg);
					RDebug::Printf( "doTestL() -> SaveConfig(KNullDesc==Default )\n");
					TRAP(ret, iSess.SaveConfigL( KNullDesC ));
					test( KErrNone == ret );
					RDebug::Printf( "doTestL : <- SaveConfig() returned 0x%X\n", ret );
					}
				else
					{
					test.Next(KLoadConfigMsg);
					RDebug::Printf( "doTestL() -> LoadConfig(KNullDesc==Dafault)\n");
					TRAP(ret, iSess.LoadConfigL( KNullDesC ));
					test( KErrNone == ret );
					RDebug::Printf( "doTestL : <- LoadConfig() returned 0x%X\n", ret );
					}
				break;
				}
                
			case('1'):
				{
	            _LIT( KGetPluginsMsg,		"Request Plugin List" );
				test.Next(KGetPluginsMsg);
				pluginsList.Reset();

				RDebug::Printf( "doTestL() -> GetPlugins()\n");

				TRAP(ret, iSess.GetPluginListL( pluginsList ));
				test( KErrNone == ret );

				RDebug::Printf( "doTestL : <- GetPlugins() returned 0x%X, pluginList.Count()=%d, with following names:\n", 
					ret, pluginsList.Count() );
				
				for(TUint i = 0; i < pluginsList.Count(); i++ )
					{

					test.Printf(_L("   Plugin List[%d].iName=%S, UID=0x%X, type=%d, loaded=%d\n"), 
						i, 
						&(pluginsList[i].iName),
						pluginsList[i].iUid,
						pluginsList[i].iType, 
						pluginsList[i].iLoaded );
					}

				break;

				}

			case('a'):
				{
	            _LIT( KGetPluginInfosMsg,		"Request Plugin Info List" );
				test.Next(KGetPluginInfosMsg);

				formatterPtrList.ResetAndDestroy();
				RDebug::Print( _L("doTestL() -> GetFormattersL()\n"));
				TRAP( ret, iSess.GetFormattersL( formatterPtrList ));
				test( KErrNone == ret );
				RDebug::Print( _L(" doTestL() : <- GetFormatters() returned 0x%X, formatterPtrList.Count()=%d, with following names:\n"), 
					ret, formatterPtrList.Count() );

				writerPtrList.ResetAndDestroy();
				RDebug::Print( _L("doTestL() -> GetWritersL()\n"));
				TRAP( ret, iSess.GetWritersL( writerPtrList ));
				test( KErrNone == ret );
				RDebug::Print( _L(" doTestL() : <- GetWriters() returned 0x%X, writerPtrList.Count()=%d, with following names:\n"), 
					ret, writerPtrList.Count() );

				for(TUint i = 0; i < formatterPtrList.Count(); i++ )
					{

					test.Printf(_L("   Formatter List[%d].iName=%S, UID=0x%X, version=%d, type=%d\n"), 
						i, 
						&(formatterPtrList[i]->Name()),
						formatterPtrList[i]->Uid(),
						formatterPtrList[i]->Version(), 
						formatterPtrList[i]->Type() );
					}

				for(TUint i = 0; i < writerPtrList.Count(); i++ )
					{

					test.Printf(_L("   Writter List[%d].iName=%S, UID=0x%X, version=%d, type=%d\n"), 
						i, 
						&(writerPtrList[i]->Name()),
						writerPtrList[i]->Uid(),
						writerPtrList[i]->Version(), 
						writerPtrList[i]->Type() );
					}

				break;

				}

			case('2'):
				{
	            _LIT( KLoadPluginMsg, "Load Plugin" );
				test.Next(KLoadPluginMsg);

	            _LIT(KPluginLoadMenu, "\n\t\t\tType Index of the plugin to load:\n");
				test.Printf(KPluginLoadMenu);
				c=test.Getch();
				TInt pluginIndex = c.GetNumericValue();

				RDebug::Print(_L("doTestL() ->  LoadPlugin[%d].iName=%S, UID=0x%X\n"),
							  pluginIndex, &(pluginsList[pluginIndex].iName), pluginsList[pluginIndex].iUid ) ;

                TPluginRequest req;
                req.iUid = pluginsList[pluginIndex].iUid;
                req.iPluginType = pluginsList[pluginIndex].iType;
                req.iLoad = ETrue;

                TRAP(ret, iSess.PluginRequestL( req ));
                test(ret == KErrNone);
                RDebug::Printf( " Plugin with UID = 0x%X has been loaded\n", req.iUid );
                test.Printf( _L("   Plugin with UID = 0x%X has been loaded\n"), req.iUid ); 

				break;

				}
            case('b'):
                {
	            _LIT( KLoadPluginMsg, "Unload Plugin" );
				test.Next(KLoadPluginMsg);

                TPluginRequest req;
                req.iLoad = EFalse;

	            _LIT(KPluginUnloadoadMenu, "\n\t\t\tType Index of the plugin to unload:\n");
				test.Printf(KPluginUnloadoadMenu);
				c=test.Getch();
				TInt pluginIndex = c.GetNumericValue();

	            _LIT(KPluginTypeUnloadMenu, "\n\t\t\tType type of the plugin to unload:[f/w]\n");
				test.Printf(KPluginTypeUnloadMenu);
				c=test.Getch();

                if(c == 'f')
                {
				    RDebug::Print(_L("doTestL() ->  UnloadPlugin[%d].iName=%S, UID=0x%X\n"),
							      pluginIndex, &(formatterPtrList[pluginIndex]->Name()), formatterPtrList[pluginIndex]->Uid() ) ;

                    req.iPluginType = TPluginRequest::EFormatter;
                    req.iUid = TUid::Uid(formatterPtrList[pluginIndex]->Uid());
                }
                else if(c == 'w')
                {
				    RDebug::Print(_L("doTestL() ->  UnloadPlugin[%d].iName=%S, UID=0x%X\n"),
							      pluginIndex, &(writerPtrList[pluginIndex]->Name()), writerPtrList[pluginIndex]->Uid() ) ;
                    req.iPluginType = TPluginRequest::EWriter;
                    req.iUid = TUid::Uid(writerPtrList[pluginIndex]->Uid());
                }
                else
                {
                    test.Printf(_L("Unknown plugin type!\n"));
                    break;
                }

                TRAP(ret, iSess.PluginRequestL( req ));
                test(ret == KErrNone);
                RDebug::Printf( " Plugin with UID = 0x%X has been unloaded\n", req.iUid );
                test.Printf( _L("   Plugin with UID = 0x%X has been unloaded\n"), req.iUid ); 
                break;
                }

            case('c'):
                {
	            _LIT( KBindPluginMsg, "Bind Plugins" );
				test.Next(KBindPluginMsg);

	            _LIT(KFormatterBindMenu, "\n\t\t\tType Index of the formatter plugin to bind:\n");
				test.Printf(KFormatterBindMenu);
				c=test.Getch();
				TInt formatterIndex = c.GetNumericValue();

	            _LIT(KWriterBindMenu, "\n\t\t\tType Index of the writer plugin to bind:\n");
				test.Printf(KWriterBindMenu);
				c=test.Getch();
				TInt writerIndex = c.GetNumericValue();

                TPluginRequest req;
                req.iIndex = formatterIndex;
                req.iPair = writerIndex;
                req.iUid = TUid::Uid(0); 

                TRAP(ret, iSess.PluginRequestL( req ));
                test(ret == KErrNone);
                RDebug::Printf( " Plugin %d and %d have been bound\n", req.iIndex, req.iPair );
                test.Printf( _L(" Plugin %d and %d have been bound\n"), req.iIndex, req.iPair );
                break;
                }

			case('3'):
				{

				test.Next(KGetThreadMsg);
				threadPtrList.ResetAndDestroy();

				RDebug::Print( _L("doTestL() -> GetThreads()\n"));

				TRAP( ret, iSess.GetThreadsL( threadPtrList ));
				test( KErrNone == ret );

				RDebug::Print( _L(" doTestL() : <- GetThreads() returned 0x%X, threadPtrList.Count()=%d, with following names:\n"), 
					ret, threadPtrList.Count() );

				RBuf rPrintBuf;

				TUint64 pId;
				TUint32 pIdLow;
				TUint32 pIdHigh;

				for(TUint i = 0; i < threadPtrList.Count(); i++ )
					{
					pId = threadPtrList[i]->ProcessId();
					pIdLow = I64LOW( pId );
					pIdHigh = I64HIGH( pId );

					test.Printf(_L("   threadList[%d].iName=%S, id=0x%X"), 
						i, &(threadPtrList[i]->Name()), threadPtrList[i]->Id() );
					test.Printf(_L("  owner=0x%X%X\n"), pIdHigh, pIdLow );
                    }
				}

				break;

			case('4'):
				{

				test.Next( KGetProcMsg );
				procPtrList.ResetAndDestroy();

				RDebug::Print( _L("doTestL() -> GetProcess()\n"));

                TRAP( ret, iSess.GetProcessesL( procPtrList ));
				test( KErrNone == ret );

				RDebug::Printf( "doTestL() <- GetProcesses() returned 0x%X, procPtrList.Count()=%d, with following names:\n", 
					ret, procPtrList.Count() );
				
				for(TUint i = 0; i < procPtrList.Count(); i++ )
					{
					test.Printf(_L("   procPtrList[%d].iName=%S, id=%d \n"), 
						i, 
						&(procPtrList[i]->Name()),
						procPtrList[i]->Id() );
					}

				}

				break;

			case('5'):
				{

				test.Next( KGetExeMsg );

				executablePtrList.ResetAndDestroy();

				RDebug::Print( _L("doTestL() -> GetExecutable()\n"));

                TRAP( ret, iSess.GetExecutablesL( executablePtrList ));
				test( KErrNone == ret );

				RDebug::Printf( "doTestL() <- GetExecutable() returned 0x%X, executablePtrList.Count()=%d, with following names:\n", 
					ret, executablePtrList.Count() );
				
				for(TUint i = 0; i < executablePtrList.Count(); i++ )
					{
					test.Printf(_L("   executablePtrList[%d].iName=%S, active=%d, passive=%d, observed=%d\n"), 
						i, 
						&(executablePtrList[i]->Name()),
						executablePtrList[i]->ActivelyDebugged(),
						executablePtrList[i]->PassivelyDebugged(),
						executablePtrList[i]->Observed() );
					}		
				}

				break;


			case('6'):
				{

				test.Next( KObserveProcName );
				c=test.Getch();
				TInt procId1 = c.GetNumericValue();
				c=test.Getch();
				TInt procId2 = c.GetNumericValue();
				c=test.Getch();
				TInt procId3 = c.GetNumericValue();

				TInt procId = procId1 * 100 + 10 * procId2 + procId3;

				TBool found = EFalse;
				TUint i;
				for( i = 0; i < procPtrList.Count(); i++ )
					{

					if( procId == procPtrList[i]->Id() )
						{
							found = ETrue;
							break;
						}
					}

				if( found )
					{
					RDebug::Print( _L("doTestL() : -> Observe( proc name: %S )\n"), &procPtrList[i]->Name() );

                    TRAP(ret, iSess.ObservationRequestL( procPtrList[i]->Name(), procPtrList[i]->Name(), ETrue) );

					RDebug::Printf( "doTestL() <- Observe() returned %d\n", ret );

					test( KErrNone == ret );

					}
			
				}

				break;

			case('7'):
				{

				test.Next( KLeaveProcName );

				test.Printf( KProcObserveMenu );
				c=test.Getch();
				TInt procId1 = c.GetNumericValue();
				c=test.Getch();
				TInt procId2 = c.GetNumericValue();
				c=test.Getch();
				TInt procId3 = c.GetNumericValue();

				TInt procId = procId1 * 100 + 10 * procId2 + procId3;

				TBool found = EFalse;
				TUint i;
				for( i = 0; i < procPtrList.Count(); i++ )
					{

					if( procId == procPtrList[i]->Id() )
						{
							found = ETrue;
							break;
						}
					}

				if( found )
					{
					RDebug::Print( _L("doTestL() : -> Leave( proc name: %S )\n"), &procPtrList[i]->Name() );

                    TRAP(ret, iSess.ObservationRequestL( procPtrList[i]->Name(), procPtrList[i]->Name(), EFalse) );

					RDebug::Printf( "doTestL() <- Leave() returned %d\n", ret );

					test( KErrNone == ret );

					}
			
				}

				break;
                

			case('8'):
				{
	            _LIT( KConfigureSystem,     "Configure the System Parameters" );
				test.Next( KConfigureSystem );

				TInt numConfigParams = -1;
			
				RPointerArray<COptionConfig> configParams;
				COptionConfig * config = NULL;

				numConfigParams = iSess.GetNumberConfigParametersL();
				RDebug::Printf( "There are %d system params\n", numConfigParams );

                for(TInt i = 0 ; i < numConfigParams; i++)
                {
				    config = iSess.GetConfigParameterL(i);
                    configParams.AppendL(config);
                }

                for(TInt i = 0 ; i < numConfigParams; i++)
                {
                    config = configParams[i];
                    RDebug::Printf("param[%d] index=%d, instance=%d, source=%d, type=%d, value=%d values=%S",
                                   i, config->Index(), config->Instance(), config->Source(), config->Type(),
                                   config->Value(), &config->ValueAsDesc() );
                }

                configParams.ResetAndDestroy();

	            _LIT(KFormatterConfigMenu, "\n\t\t\tType Index of the parameter to change:\n");
				test.Printf(KFormatterConfigMenu);
				c=test.Getch();
				TInt paramIndex = c.GetNumericValue();

				config = iSess.GetConfigParameterL( paramIndex );
				if(config)
                {
                    const TDesC & lastPrompt = config->Prompt();
                    const TDesC & lastOpts = config->Options();

                    RBuf printPromptBuf;
                    printPromptBuf.Create( lastPrompt , lastPrompt.Length()+1 );
                    //printPromptBuf.SetMax();
                    char* clPrompt = (char*) printPromptBuf.Collapse().PtrZ();
                    RDebug::Printf("  lastPrompt=%s, length=%d\n", clPrompt, printPromptBuf.Length() );
                    printPromptBuf.Close();

                    RBuf printOptsBuf;
                    printOptsBuf.Create( lastOpts , lastOpts.Length()+1 );
                    //printPromptBuf.SetMax();
                    char* clOpts = (char*) printOptsBuf.Collapse().PtrZ();
                    RDebug::Printf("  lastOpts=%s, length=%d\n", clOpts , printOptsBuf.Length() );
                    printOptsBuf.Close();

                    RDebug::Printf("  Enter param value:\n" );
                    c=test.Getch();
                    TInt paramValue = c.GetNumericValue();
                    LOG_MSG3( "  ->SetConfigParameterL( paramIndex=%d, value=%d )\n", paramIndex, paramValue );
                    config->ValueL( KNullDesC );//To let us change the value as an int
                    config->Value( paramValue );
                    TRAP(ret, iSess.SetConfigParameterL(*config) );
                    delete config;
				    config = iSess.GetConfigParameterL( paramIndex );
                    if(config)
                    {
                        RDebug::Printf( "param[%d] index=%d, instance=%d, type=%d, source=%d, value=%d values=%S",
                                config->Index(), config->Instance(), config->Type(), config->Source(),
                                config->Value(), &config->ValueAsDesc() );
                    }
                    test(KErrNone == ret);
                }
				break;
				}


			case('9'):
				{

				// Create crashing app
				RProcess iCrashProcess;
				RDebug::Printf( "Creating crashing application\n" );
				TInt err = iCrashProcess.Create( KCrashAppFileName, KNullDesC );
				test( err == KErrNone );

				User::After( 1000000 );
				iCrashProcess.Resume();
				//
				
				break;
				}

			case('q'):
				{
				iSess.Disconnect();
				finish = ETrue;
				break;
				}

			default:
				{
				break;
				}

			}// switch

		}
	while( finish == EFalse );

	RDebug::Printf( "pluginsList.Close();\n");
	pluginsList.Close();

	RDebug::Printf( "threadPtrList.ResetAndDestroy();\n");
	threadPtrList.ResetAndDestroy();

	RDebug::Printf( "procPtrList.ResetAndDestroy();\n");
	procPtrList.ResetAndDestroy();

	RDebug::Printf( "executablePtrList.ResetAndDestroy();\n");
	executablePtrList.ResetAndDestroy();


	RDebug::Printf( "formatterPtrList.ResetAndDestroy();\n");
	formatterPtrList.ResetAndDestroy();

	RDebug::Printf( "writerPtrList.ResetAndDestroy();\n");
	writerPtrList.ResetAndDestroy();

	__UHEAP_MARKEND; 
	__UHEAP_MARKEND; 

	RDebug::Printf( "CleanupStack::Pop(); // test\n");
	CleanupStack::Pop(); // test
	RDebug::Printf( "test.End();\n");
	test.End();
	_LIT(KPressAnyKey, "\nPress any key\n");
	test.Printf(KPressAnyKey);
	RDebug::Printf( "test.Getch();\n");
	test.Getch();
	test.Close();
	}
