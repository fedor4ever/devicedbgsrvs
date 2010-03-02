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



/**
 @file
 @internalTechnology
 @released
*/

#include <e32cons.h>
#include <e32debug.h>
#include <bacline.h>
#include <e32property.h>

#include <coredumpinterface.h>
#include <debuglogging.h>

LOCAL_C void processConfigL()
	{

	// Create a CCrashConfig object, the configuration client-side 
	// object of the CoreDump Server .
    RCoreDumpSession coredumpinterface;

	TInt ret = coredumpinterface.Connect();

	if( KErrNone != ret )
		{
		LOG_MSG2( "requestConfigLoadL():: Could not create a Core Dump configuration object, error=%d", ret );
		User::Leave( ret );
		}

	TUint crashes = 1;

    TInt argc = User::CommandLineLength();
    TPtrC configFile(KNullDesC);	
	HBufC* args = NULL;
    if(argc > 0)
    {
        args = HBufC::NewLC(User::CommandLineLength());
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
                case 'c':
                    lex.Val(crashes);
                    break;
                case 'f':
                    configFile.Set(lex.NextToken());
                    break;
                default:
                    User::Leave(KErrArgument);
                }
            }
            lex.SkipSpace();
        }
    }

    TRAPD(err, coredumpinterface.LoadConfigL( configFile ));

    if(err != KErrNone)
    {
	LOG_MSG2("unable to load config file! err:%d\n", err );
    coredumpinterface.Disconnect();
    User::Leave(err);
    }

	LOG_MSG2( "Will wait for %u crashes\n", crashes );

	RProperty crashCountProperty;
	User::LeaveIfError( crashCountProperty.Attach( KCoreDumpServUid, ECrashCount ) );

	TInt crashCount = 0;
	do
		{
		
		User::After(5000000);
		ret = crashCountProperty.Get( crashCount );
		LOG_MSG2( "  crashCountProperty.Get( crashCount )=%d\n", crashCount );
		if ( KErrNone != ret )
			{
			break;
			}
		}
	while( crashes > crashCount );

    crashCountProperty.Close();
	if(args)
        {
        CleanupStack::PopAndDestroy(args);  
        }
    coredumpinterface.Disconnect();
	LOG_MSG( "  returned from CleanupStack::PopAndDestroy( cmd );" );

	}


GLDEF_C TInt E32Main() // main function called by E32
    {
	__UHEAP_MARK;

	CTrapCleanup* cleanup=CTrapCleanup::New();
    if(!cleanup)
        return KErrNoMemory;

	TRAPD(err,processConfigL());
   

	delete cleanup;
	__UHEAP_MARKEND; // Check memory leaks

	return err;
	}

